#include "Scene.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <unordered_set>

#include <cstdlib>

#include "Classes/Pipeline/Pipeline.hpp"
#include "Classes/Buffer/Buffer.hpp"
#include "Classes/Engine/Engine.hpp"

#include "Node.hpp"

using namespace nihil::graphics;

enum class DrawCommandType : uint8_t
{
    Instanced,
    Normal
};

template<typename T>
struct InstancingNodeData
{
    Node<T>* nextUniqueModelNode = nullptr;
    bool head = false;
    size_t instancedDrawSize = 0;
};

struct DrawCommandNodeData
{
    DrawCommandType type;
    Object* object = nullptr;

    InstancingNodeData<DrawCommandNodeData> instancingData = {};
};

using byte = unsigned char;

inline void drawNormal(Object* object, Camera* camera, vk::CommandBuffer& commandBuffer)
{
    //Push Constants
    commandBuffer.pushConstants(object->model->_pipeline()->_layout(), vk::ShaderStageFlagBits::eVertex, 0, sizeof(PushConstants), object->_pushConstants(camera));

    //Draw
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, object->model->_pipeline()->_pipeline());
    commandBuffer.bindVertexBuffers(0, object->_vertexBuffer()._buffer(), { 0 });

    commandBuffer.bindIndexBuffer(object->_indexBuffer()._buffer(), 0, vk::IndexType::eUint32);

    commandBuffer.drawIndexed(static_cast<uint32_t>(object->_indexBuffer()._typedSize()), 1, 0, 0, 0);
}

inline void drawInstanced(Node<DrawCommandNodeData>* head, vk::CommandBuffer& commandBuffer, Camera* camera, Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>* _instanceBuffer, Engine* engine)
{
    //? This ensures that the buffer is initialized by checking if the engine ptr is correct
    if(_instanceBuffer->_engine() == engine) _instanceBuffer->~Buffer();
    std::vector<float> instanceData;
    instanceData.resize(head->data->instancingData.instancedDrawSize * 16);
    Node<DrawCommandNodeData>* nodeIterator = head;

    constexpr size_t instanceDataChunkSize = 16 * sizeof(float);

    size_t i = 0;
    while(nodeIterator)
    {
        const float* matrixData = glm::value_ptr(nodeIterator->data->object->_modelMatrix());
        memcpy(reinterpret_cast<char*>(instanceData.data()) + (instanceDataChunkSize * i), matrixData, instanceDataChunkSize);

        nodeIterator = nodeIterator->data->instancingData.nextUniqueModelNode;

        i++;
    }

    Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>* instanceBuffer = new(_instanceBuffer) Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>(instanceData, engine);

    Object* object = head->data->object;

    //Push Constants
            //Model component of the push constants is unused
            commandBuffer.pushConstants(object->model->_instancedPipeline()->_layout(), vk::ShaderStageFlagBits::eVertex, 0, sizeof(PushConstants), object->_pushConstants(camera));

            //Draw
            commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, object->model->_instancedPipeline()->_pipeline());

            instanceBuffer->moveToGPU();

            std::array<vk::Buffer, 2> vertexBuffers = {
                object->model->_vertexBuffer()._buffer(),
                instanceBuffer->_buffer()

            };
            std::array<vk::DeviceSize, 2> vertexBufferOffsets = {
                0,
                0
            };
            commandBuffer.bindVertexBuffers(0, vertexBuffers, vertexBufferOffsets);

            commandBuffer.bindIndexBuffer(object->model->_indexBuffer()._buffer(), 0, vk::IndexType::eUint32);

            commandBuffer.drawIndexed(static_cast<uint32_t>(object->model->_indexBuffer()._typedSize()), head->data->instancingData.instancedDrawSize, 0, 0, 0);
}

inline bool headExists(Node<DrawCommandNodeData>* head, Model* model)
{
    Node<DrawCommandNodeData>* nodeIterator = head;
    while(nodeIterator)
    {
        if(nodeIterator->data->object->model == model && nodeIterator->data->instancingData.head)
        {
            return true;
        }

        nodeIterator = nodeIterator->next;
    }

    return false;
}

inline Node<DrawCommandNodeData>* findModelHead(Node<DrawCommandNodeData>* head, Model* model)
{
    Node<DrawCommandNodeData>* nodeIterator = head;
    while(nodeIterator)
    {
        if(nodeIterator->data->object->model == model && nodeIterator->data->instancingData.head)
        {
            return nodeIterator;
        }

        nodeIterator = nodeIterator->next;
    }

    return nullptr;
}

void Scene::recordCommands(vk::CommandBuffer& commandBuffer, Camera* camera)
{
    /*
    TODO:
    Reuse instance buffers. 
    */

    if (drawCommandSlabSize != (sizeof(Node<DrawCommandNodeData>) * objects.size()) + (sizeof(DrawCommandNodeData) * objects.size()) || !drawCommandSlab)
    {
        drawCommandSlab = (byte*)realloc(
            drawCommandSlab,
            (sizeof(Node<DrawCommandNodeData>) * objects.size()) +
            (sizeof(DrawCommandNodeData) * objects.size())
        );
    }
    drawCommandSlabSize = (sizeof(Node<DrawCommandNodeData>) * objects.size()) + (sizeof(DrawCommandNodeData) * objects.size());

    byte* nodeMemory = drawCommandSlab;
    byte* dataMemory = drawCommandSlab + (sizeof(Node<DrawCommandNodeData>) * objects.size());

    size_t nodeRelativePointer = 0, dataRelativePointer = 0;

    bool setBaseNode = false;
    Node<DrawCommandNodeData>* baseNode = nullptr;
    Node<DrawCommandNodeData>* previousNode = nullptr;

    for(Object* o : objects)
    {
        if(o->model->_instancedPipeline())
        {
            Node<DrawCommandNodeData>* node = reinterpret_cast<Node<DrawCommandNodeData>*>(nodeMemory) + nodeRelativePointer;
            DrawCommandNodeData* data = reinterpret_cast<DrawCommandNodeData*>(dataMemory) + dataRelativePointer;

            *node = {};
            *data = {};

            data->type = DrawCommandType::Instanced;
            data->object = o;

            node->prev = previousNode;
            node->data = data;
            if(previousNode) previousNode->next = node;

            previousNode = node;

            if(!setBaseNode) {baseNode = node; setBaseNode = true;};

            nodeRelativePointer++;
            dataRelativePointer++;
        }
        else
        {
            Node<DrawCommandNodeData>* node = reinterpret_cast<Node<DrawCommandNodeData>*>(nodeMemory + nodeRelativePointer);
            DrawCommandNodeData* data = reinterpret_cast<DrawCommandNodeData*>(dataMemory + dataRelativePointer);

            *node = {};
            *data = {};

            data->type = DrawCommandType::Normal;
            data->object = o;

            node->prev = previousNode;
            node->data = data;
            if(previousNode) previousNode->next = node;

            previousNode = node;

            if(!setBaseNode) {baseNode = node; setBaseNode = true;};

            nodeRelativePointer++;
            dataRelativePointer++;
        }
    }

    size_t uniqueModelsCount = 0;

    Node<DrawCommandNodeData>* nodeIterator = baseNode;
    while(nodeIterator)
    {
        if(!nodeIterator) break;

        if(nodeIterator->data->type == DrawCommandType::Normal)
        {
            drawNormal(nodeIterator->data->object, camera, commandBuffer);
        }
        else if(nodeIterator->data->type == DrawCommandType::Instanced)
        {
            if(!headExists(baseNode, nodeIterator->data->object->model))
            {
                nodeIterator->data->instancingData.head = true;
                uniqueModelsCount++;
                nodeIterator->data->instancingData.instancedDrawSize = 1;
            }
            else 
            {
                Node<DrawCommandNodeData>* modelHead = findModelHead(baseNode, nodeIterator->data->object->model);
                modelHead->data->instancingData.instancedDrawSize++;

                //? Seek the node without a nextUniqueModelNode
                Node<DrawCommandNodeData>* nodeIterator2 = modelHead;
                while(nodeIterator2)
                {
                    if(nodeIterator2->data->instancingData.nextUniqueModelNode == nullptr)
                    {
                        nodeIterator2->data->instancingData.nextUniqueModelNode = nodeIterator;
                        break;
                    }
                    nodeIterator2 = nodeIterator2->data->instancingData.nextUniqueModelNode;
                }
            }
        }

        nodeIterator = nodeIterator->next;
    }

    size_t instanceBufferRelativePointer = 0;
    if(!instanceBufferSlab) 
    {
        instanceBufferSlab = (Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>*)malloc(sizeof(Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>) * uniqueModelsCount);
        instanceBufferSlabSize = sizeof(Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>) * uniqueModelsCount;
    }
    else
    {
        if(!(instanceBufferSlabSize == sizeof(Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>) * uniqueModelsCount))
        {
            instanceBufferSlab = (Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>*)realloc(instanceBufferSlab, sizeof(Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>) * uniqueModelsCount);
            instanceBufferSlabSize = sizeof(Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>) * uniqueModelsCount;
        }
    }

    nodeIterator = baseNode;
    size_t i = 0;
    while(nodeIterator)
    {
        if(nodeIterator->data->type == DrawCommandType::Instanced && nodeIterator->data->instancingData.head)
        {
            //TODO: figure out the instance buffer allocation
            if(nodeIterator->data->instancingData.instancedDrawSize > 1)
            {
                drawInstanced(nodeIterator, commandBuffer, camera, instanceBufferSlab + i, engine);
                i++;
            }
            else
            {
                drawNormal(nodeIterator->data->object, camera, commandBuffer);
            }
        }

        nodeIterator = nodeIterator->next;
    }
}