#include "Scene.hpp"

#include "Classes/Camera/Camera.hpp"
#include "Classes/Model/Model.hpp"
#include "Classes/Engine/Engine.hpp"
#include "Classes/Pipeline/Pipeline.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace nihil::graphics;

void Scene::recordCommands(vk::CommandBuffer& commandBuffer, Camera* camera)
{
    instancedDraws.clear();
    normalDraws.clear();
    
    for(Object* o : objects)
    {
        if(o->_model()->_instancedPipeline())
        {
            auto it = instancedDraws.find(o->_model());
            if(it != instancedDraws.end())
            {
                it->second.push_back(o);
            }
            else 
            {
                instancedDraws.insert(std::make_pair(o->_model(), std::vector<Object*>()));
                instancedDraws[o->_model()].reserve(64);
                instancedDraws[o->_model()].push_back(o);
            }
        }
        else
        {
            normalDraws.push_back(o);
        }
    }
    for(auto& it : instancedDraws)
    {
        if(it.second.size() > 1)
        {
            //Push Constants
            //Model component of the push constants is unused
            commandBuffer.pushConstants(it.first->_instancedPipeline()->_layout(), vk::ShaderStageFlagBits::eVertex, 0, sizeof(PushConstants), it.second[0]->_pushConstants(camera));

            //Draw
            commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, it.first->_instancedPipeline()->_pipeline());

            //Create or reuse the instance buffer
            constexpr size_t instanceDataChunkSize = 16 * sizeof(float);
            size_t instanceDataSize = it.second.size() * instanceDataChunkSize;

            Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>* instanceBuffer = nullptr;

            auto bufferIT = instanceBuffers.find(it.first);
            if(bufferIT != instanceBuffers.end()) 
            {
                instanceBuffer = bufferIT->second; 
                Logger::Log("Reusing buffer.");
            }
            else
            {
                Logger::Log("Creating a buffer.");

                instanceBuffer = bufferHeap.alloc<Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>>();

                std::vector<float> instanceData;

                instanceData.resize(instanceDataSize / sizeof(float));

                for (int i = 0; i < it.second.size(); i++)
                {
                    const float* matrixData = glm::value_ptr(it.second[i]->_modelMatrix());
                    memcpy(reinterpret_cast<char*>(instanceData.data()) + (instanceDataChunkSize * i), matrixData, instanceDataChunkSize);
                }

                instanceBuffer = new (instanceBuffer) Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>(instanceData, engine);

                instanceBuffer->moveToGPU();

                instanceBuffers.insert(std::make_pair(it.first, instanceBuffer));
            }

            bool objectModified = false;

            for(Object* o : it.second)
            {
                if(o->modifiedThisFrame) objectModified = true;
            }

            if(bufferIT != instanceBuffers.end() && objectModified)
            {
                Logger::Log("Modifing a buffer.");

                //In the future do this so that it only modifies the matrix of the modified object
                std::vector<float> instanceData;

                instanceData.resize(instanceDataSize / sizeof(float));

                for (int i = 0; i < it.second.size(); i++)
                {
                    const float* matrixData = glm::value_ptr(it.second[i]->_modelMatrix());
                    memcpy(reinterpret_cast<char*>(instanceData.data()) + (instanceDataChunkSize * i), matrixData, instanceDataChunkSize);
                }

                if(instanceDataSize == instanceBuffer->_size())
                {
                    //Just modify the data
                    instanceBuffer->update(std::move(instanceData));
                }
                else
                {
                    //the buffer is of the wrong size, delete it and make a new one. reuse the same memory location
                    instanceBuffer->~Buffer();

                    instanceBuffer = new (instanceBuffer) Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>(instanceData, engine);
                }
            }

            assert(instanceBuffer != nullptr);

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
        else if(it.second.size() == 1)
        {
            normalDraws.push_back(it.second[0]);
        }
    }

    for(Object* o : normalDraws)
    {
        //Push Constants
        commandBuffer.pushConstants(o->model->_pipeline()->_layout(), vk::ShaderStageFlagBits::eVertex, 0, sizeof(PushConstants), o->_pushConstants(camera));

        //Draw
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, o->model->_pipeline()->_pipeline());
        commandBuffer.bindVertexBuffers(0, o->_vertexBuffer()._buffer(), { 0 });

        commandBuffer.bindIndexBuffer(o->_indexBuffer()._buffer(), 0, vk::IndexType::eUint32);

        commandBuffer.drawIndexed(static_cast<uint32_t>(o->_indexBuffer()._typedSize()), 1, 0, 0, 0);
    }

    for(Object* o : objects) { o->afterRender(); };
}