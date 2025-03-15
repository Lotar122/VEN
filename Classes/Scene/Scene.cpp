#include "Scene.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Classes/Pipeline/Pipeline.hpp"
#include "Classes/Buffer/Buffer.hpp"
#include "Classes/Engine/Engine.hpp"

using namespace nihil::graphics;

void Scene::recordCommands(vk::CommandBuffer& commandBuffer, Camera* camera)
{
    /*
    TODO:
    Group the objects by their model, then check if the model has an instanced pipeline and render the model using the instanced pipeline.
    */

    std::unordered_map<Model*, std::vector<Object*>> instancedDraws;
    std::vector<Object*> normalDraws;

    for(Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>* b : instanceBuffers)
    {
        delete b;
    }

    size_t toReserve = instanceBuffers.size();
    instanceBuffers.clear();
    instanceBuffers.reserve(toReserve);

    for(Object* o : objects)
    {
        if(o->model->_instancedPipeline())
        {
            auto it = instancedDraws.find(o->model);
            if(it != instancedDraws.end())
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

    for(Object* o : normalDraws)
    {
        //Push Constants
        commandBuffer.pushConstants(o->model->_pipeline()->_layout(), vk::ShaderStageFlagBits::eVertex, 0, sizeof(PushConstants), o->_pushConstants(camera));

        //Draw
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, o->model->_pipeline()->_pipeline());
        commandBuffer.bindVertexBuffers(0, o->_vertexBuffer()._buffer(), {0});

        commandBuffer.bindIndexBuffer(o->_indexBuffer()._buffer(), 0, vk::IndexType::eUint32);

        commandBuffer.drawIndexed(static_cast<uint32_t>(o->_indexBuffer()._typedSize()), 1, 0, 0, 0);
    }

    for(auto& it : instancedDraws)
    {
        //Push Constants
        commandBuffer.pushConstants(it.first->_instancedPipeline()->_layout(), vk::ShaderStageFlagBits::eVertex, 0, sizeof(PushConstants), it.second[0]->_pushConstants(camera));

        //Draw
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, it.first->_instancedPipeline()->_pipeline());

        constexpr size_t instanceDataChunkSize = 16 * sizeof(float);

        std::vector<float> instanceData;
        instanceData.resize(it.second.size() * instanceDataChunkSize);

        for(int i = 0; i < it.second.size(); i++)
        {
            const float* matrixData = glm::value_ptr(it.second[i]->_modelMatrix());
            memcpy(reinterpret_cast<char*>(instanceData.data()) + (instanceDataChunkSize * i), matrixData, instanceDataChunkSize);
        }

        Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>* instanceBuffer = new Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>(instanceData, engine);
        instanceBuffers.push_back(instanceBuffer);
        instanceBuffer->moveToGPU();

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

        std::cout << it.first->_indexBuffer()._typedSize() << " " << it.second.size() << '\n';
        commandBuffer.drawIndexed(static_cast<uint32_t>(it.first->_indexBuffer()._typedSize()), it.second.size(), 0, 0, 0);
    }
}