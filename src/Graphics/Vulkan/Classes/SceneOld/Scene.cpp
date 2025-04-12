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

    std::unordered_map<Model*, std::vector<Object*>> instancedDraws;
    std::vector<Object*> normalDraws;

    for (auto& b : instanceBuffers)
    {
        delete b.second.first;
    }

    size_t toReserve = instanceBuffers.size();
    instanceBuffers.clear();
    instanceBuffers.reserve(toReserve);

    for (Object* o : objects)
    {
        if (o->model->_instancedPipeline())
        {
            auto it = instancedDraws.find(o->model);
            if (it != instancedDraws.end())
            {
                it->second.push_back(o);
            }
            else
            {
                instancedDraws.insert(std::make_pair(o->model, std::vector<Object*>()));
                instancedDraws[o->model].reserve(50);
                instancedDraws[o->model].push_back(o);
            }
        }
        else
        {
            normalDraws.push_back(o);
        }
    }

    for (auto& it : instancedDraws)
    {
        //checks if there's more than one object to be instanced.
        if (it.second.size() == 1)
        {
            normalDraws.push_back(it.second[0]);
        }
        else
        {
            //Push Constants
            //Model component of the push constants is unused
            commandBuffer.pushConstants(it.first->_instancedPipeline()->_layout(), vk::ShaderStageFlagBits::eVertex, 0, sizeof(PushConstants), it.second[0]->_pushConstants(camera));

            //Draw
            commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, it.first->_instancedPipeline()->_pipeline());

            constexpr size_t instanceDataChunkSize = 16 * sizeof(float);

            std::vector<float> instanceData;
            instanceData.resize(it.second.size() * instanceDataChunkSize);

            for (int i = 0; i < it.second.size(); i++)
            {
                const float* matrixData = glm::value_ptr(it.second[i]->_modelMatrix());
                memcpy(reinterpret_cast<char*>(instanceData.data()) + (instanceDataChunkSize * i), matrixData, instanceDataChunkSize);
            }

            //implement buffer reuse
            // Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>* instanceBuffer = new Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>(instanceData, engine);
            // instanceBuffers.push_back(instanceBuffer);
            // instanceBuffer->moveToGPU();

            /*
            TODO: 
            iterate over all buffers, mark them as false (not used). only mark reused buffers as true (used). delete all buffers marked as false.
            */

            Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>* instanceBuffer = nullptr;
            auto it = instanceBuffers.find(instanceData.size());
            if(it == instanceBuffers.end())
            {
                instanceBuffer = bufferHeap.alloc<Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>>();
                instanceBuffer = new (instanceBuffer) Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>(instanceData, engine);
            }
            else
            {
                //reuse the buffer
            }

            std::array<vk::Buffer, 2> vertexBuffers = {
                it.first->_vertexBuffer()._buffer(),
                instanceBuffer->_buffer()

            };
            std::array<vk::DeviceSize, 2> vertexBufferOffsets = {
                0,
                0
            };
            commandBuffer.bindVertexBuffers(0, vertexBuffers, vertexBufferOffsets);

            commandBuffer.bindIndexBuffer(it.first->_indexBuffer()._buffer(), 0, vk::IndexType::eUint32);

            //std::cout << it.first->_indexBuffer()._typedSize() << " " << it.second.size() << '\n';
            commandBuffer.drawIndexed(static_cast<uint32_t>(it.first->_indexBuffer()._typedSize()), it.second.size(), 0, 0, 0);
        }
    }

    for (Object* o : normalDraws)
    {
        //Push Constants
        commandBuffer.pushConstants(o->model->_pipeline()->_layout(), vk::ShaderStageFlagBits::eVertex, 0, sizeof(PushConstants), o->_pushConstants(camera));

        //Draw
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, o->model->_pipeline()->_pipeline());
        commandBuffer.bindVertexBuffers(0, o->_vertexBuffer()._buffer(), { 0 });

        commandBuffer.bindIndexBuffer(o->_indexBuffer()._buffer(), 0, vk::IndexType::eUint32);

        commandBuffer.drawIndexed(static_cast<uint32_t>(o->_indexBuffer()._typedSize()), 1, 0, 0, 0);
    }
}