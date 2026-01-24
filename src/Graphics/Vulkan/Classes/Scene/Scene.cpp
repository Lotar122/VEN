#include "Scene.hpp"

#include "Classes/Camera/Camera.hpp"
#include "Classes/Model/Model.hpp"
#include "Classes/Engine/Engine.hpp"
#include "Classes/Pipeline/Pipeline.hpp"
#include "vulkan/vulkan_handles.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace nihil::graphics;

void Scene::addObjects(const Object** newObjects, size_t size)
{
    size_t originalObjectsSize = objects.size();
    objects.resize(objects.size() + size);
    std::memcpy(reinterpret_cast<char*>(objects.data() + originalObjectsSize), reinterpret_cast<char*>(newObjects), size * sizeof(Object**));
}

void Scene::addObjects(Object* newObjects, size_t size)
{
    size_t originalObjectsSize = objects.size();
    objects.resize(objects.size() + size);
    for (size_t i = 0; i < size; i++)
    {
        objects[originalObjectsSize + i] = &newObjects[i];
    }
}

void Scene::addObjects(const std::vector<Object*>& newObjects)
{
    size_t originalObjectsSize = objects.size();
    objects.resize(objects.size() + newObjects.size());
    std::memcpy(reinterpret_cast<char*>(objects.data() + originalObjectsSize), reinterpret_cast<const char*>(newObjects.data()), newObjects.size() * sizeof(Object**));
}

void Scene::addObjects(std::vector<Object>& newObjects)
{
    size_t originalObjectsSize = objects.size();
    objects.resize(objects.size() + newObjects.size());
    for (size_t i = 0; i < newObjects.size(); i++)
    {
        objects[originalObjectsSize + i] = &newObjects[i];
    }
}

//Threading
struct ThreadContext
{
    vk::CommandBuffer commandBuffer;
    vk::CommandPool commandPool;
};

ThreadContext createThreadContext(vk::Device device, uint32_t graphicsQueueFamily)
{
    vk::CommandPoolCreateInfo poolCI{
        vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        graphicsQueueFamily
    };

    vk::CommandPool pool = device.createCommandPool(poolCI);

    vk::CommandBufferAllocateInfo alloc{
        pool,
        vk::CommandBufferLevel::eSecondary,
        1
    };

    vk::CommandBuffer secondary =
        device.allocateCommandBuffers(alloc)[0];

    return { secondary, pool };
}

//TODO
//remember to remove pipelines and the renderpass from the Model class

void Scene::recordCommands(vk::CommandBuffer& commandBuffer, Camera* camera, DescriptorAllocator* descriptorAllocator)
{
    instancedDraws.clear();
    normalDraws.clear();
    
    for(Object* o : objects)
    {
        if(o->_material()->_instancedPipeline())
        {
            auto it = instancedDraws.find(o->_modelMaterialEncoded());
            if(it != instancedDraws.end())
            {
                it->second.push_back(o);
            }
            else
            {
                //put the vectors in a pool and reuse across frames
                instancedDraws.insert(std::make_pair(o->_modelMaterialEncoded(), std::vector<Object*>()));
                instancedDraws[o->_modelMaterialEncoded()].reserve(64);
                instancedDraws[o->_modelMaterialEncoded()].push_back(o);
            }
        }
        else
        {
            normalDraws.push_back(o);
        }
    }

    //Multithread the recording by doing instanced on t1 and normal on t2. This will only decrease performance in our use case.
    //Make it split the work into 4 threads if the CPU has 6 or more cores

    for(auto& it : instancedDraws)
    {
        if(it.second.size() > 1) [[likely]]
        {
            if (descriptorAllocator)
            {
                commandBuffer.bindDescriptorSets(
                    vk::PipelineBindPoint::eGraphics,
                    it.second[0]->_material()->_instancedPipeline()->_layout(),
                    0,
                    descriptorAllocator->globalDescriptorSet->set.getRes(),
                    {}
                );
            }

            //Push Constants
            //Model component of the push constants is unused
            commandBuffer.pushConstants(it.second[0]->_material()->_instancedPipeline()->_layout(), vk::ShaderStageFlagBits::eVertex, 0, sizeof(PushConstants), it.second[0]->_pushConstants(camera));

            //Draw
            commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, it.second[0]->_material()->_instancedPipeline()->_pipeline());

            //Create or reuse the instance buffer
            size_t instanceDataChunkSize = it.second[0]->_instanceData().second;
            size_t instanceDataSize = it.second.size() * instanceDataChunkSize;

            Buffer<std::byte, vk::BufferUsageFlagBits::eVertexBuffer>* instanceBuffer = nullptr;

            auto bufferIT = instanceBuffers.find(it.first);
            if(bufferIT != instanceBuffers.end()) 
            {
                instanceBuffer = bufferIT->second; 
            }
            else
            {
                instanceBuffer = instanceBufferAllocator.allocate();

                std::vector<std::byte> instanceData;

                instanceData.resize(instanceDataSize);

                for (int i = 0; i < it.second.size(); i++)
                {
                    memcpy(reinterpret_cast<char*>(instanceData.data()) + (instanceDataChunkSize * i), it.second[i]->_instanceData().first, instanceDataChunkSize);
                }

                instanceBuffer = new (instanceBuffer) Buffer<std::byte, vk::BufferUsageFlagBits::eVertexBuffer>(instanceData, engine);

                instanceBuffers.insert(std::make_pair(it.first, instanceBuffer));
            }

            bool reconstructedInstanceBuffer = false;

            if(instanceDataSize > instanceBuffer->_size() || (int64_t)instanceDataSize - 1'000 >= (int64_t)instanceBuffer->_size())
            {
                std::vector<std::byte> instanceData;

                instanceData.resize(instanceDataSize);

                for (int i = 0; i < it.second.size(); i++)
                {
                    memcpy(reinterpret_cast<char*>(instanceData.data()) + (instanceDataChunkSize * i), it.second[i]->_instanceData().first, instanceDataChunkSize);
                }

                instanceBuffer->~Buffer();

                instanceBuffer = std::launder(new (instanceBuffer) Buffer<std::byte, vk::BufferUsageFlagBits::eVertexBuffer>(std::move(instanceData), engine));

                instanceBuffer->moveToGPU();

                reconstructedInstanceBuffer = true;

                bufferIT = instanceBuffers.find(it.first);
                bufferIT->second = instanceBuffer;
            }

            if(!reconstructedInstanceBuffer)
            {
                size_t counter = 0;
                for(Object* o : it.second)
                {
                    if(o->modifiedThisFrame) 
                    { 
                        instanceBuffer->update(
                            reinterpret_cast<const std::byte*>(o->_instanceData().first), 
                            { 0, counter * o->_instanceData().second, o->_instanceData().second }
                        );
                    }

                    counter++;
                }
            }

            // //TODO: Mutithread this

            assert(instanceBuffer != nullptr);

            instanceBuffer->moveToGPU();

            std::array<vk::Buffer, 2> vertexBuffers = {
                it.second[0]->_model()->_vertexBuffer()._buffer(),
                instanceBuffer->_buffer()

            };
            std::array<vk::DeviceSize, 2> vertexBufferOffsets = {
                0,
                0
            };
            commandBuffer.bindVertexBuffers(0, vertexBuffers, vertexBufferOffsets);

            commandBuffer.bindIndexBuffer(it.second[0]->_model()->_indexBuffer()._buffer(), 0, vk::IndexType::eUint32);

            commandBuffer.drawIndexed(static_cast<uint32_t>(it.second[0]->_model()->_indexBuffer()._typedSize()), it.second.size(), 0, 0, 0);
        }
        else if(it.second.size() == 1)
        {
            normalDraws.push_back(it.second[0]);
        }
    }

    for(Object* o : normalDraws)
    {
        if (descriptorAllocator) [[likely]]
        {
            commandBuffer.bindDescriptorSets(
                vk::PipelineBindPoint::eGraphics,
                o->_material()->_pipeline()->_layout(),
                0,
                descriptorAllocator->globalDescriptorSet->set.getRes(),
                {}
            );
        }

        //Push Constants
        commandBuffer.pushConstants(o->_material()->_pipeline()->_layout(), vk::ShaderStageFlagBits::eVertex, 0, sizeof(PushConstants), o->_pushConstants(camera));

        //Draw
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, o->model->_pipeline()->_pipeline());
        commandBuffer.bindVertexBuffers(0, o->_vertexBuffer()._buffer(), { 0 });

        commandBuffer.bindIndexBuffer(o->_indexBuffer()._buffer(), 0, vk::IndexType::eUint32);

        commandBuffer.drawIndexed(static_cast<uint32_t>(o->_indexBuffer()._typedSize()), 1, 0, 0, 0);
    }

    for(Object* o : objects) { o->afterRender(); };
}