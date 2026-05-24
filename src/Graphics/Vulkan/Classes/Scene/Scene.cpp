#include "Scene.hpp"

#include "Classes/Camera/Camera.hpp"
#include "Classes/Model/Model.hpp"
#include "Classes/Engine/Engine.hpp"
#include "Classes/Pipeline/Pipeline.hpp"
#include "Functions/BVH/BVH.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_handles.hpp"

#include <chrono>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Functions/Enumerate/Enumerate.hpp"

#include "Enums/VisibilityQueryResult.hpp"

#include <format>
#include <limits>

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

void Scene::recordCommands(vk::CommandBuffer& commandBuffer, Camera* camera, Pipeline* debugPipeline, DescriptorAllocator* descriptorAllocator)
{
    auto start = std::chrono::high_resolution_clock::now();

    instancedDraws.clear();
    normalDraws.clear();

    //Cull
    BVHNodeAllocator.reset();
    toRender.clear();
    BVHIndices.resize(objects.size());
    toRender.reserve(objects.size());

    if (objects.empty()) [[unlikely]]
    {
        Carbo::Logger::Log("Culled: 0% percent of objects.");
        return;
    }

    for(int i = 0; i < BVHIndices.size(); ++i)
    {
        BVHIndices[i] = i;
    }

    auto end = std::chrono::high_resolution_clock::now();

    std::cout<<std::format("Setup took: {}\n", std::chrono::duration_cast<std::chrono::microseconds>(end - start));

    start = std::chrono::high_resolution_clock::now();

    //Build and cull BVH
    size_t BVHRoot = buildBVH(objects, BVHIndices, 0, objects.size(), BVHNodeAllocator);
    cullBVH(BVHRoot, camera->_planes(), BVHNodeAllocator, toRender);

    float culledPercent = (1.0f - (static_cast<float>(toRender.size()) / static_cast<float>(objects.size()))) * 100.0f;

    std::cout<<std::format("Culled: {}% percent of objects.\n", culledPercent);

    end = std::chrono::high_resolution_clock::now();

    std::cout<<std::format("Culling took: {}\n", std::chrono::duration_cast<std::chrono::microseconds>(end - start));

    start = std::chrono::high_resolution_clock::now();

    for(size_t i : toRender)
    {
        Object* o = objects[i];
        
        //if (AABB::isAABBVisible(o->_transformedAABB(), camera->_planes()) == VisibilityQueryResult::Outside) continue;

        if(o->_material()->_instancedPipeline())
        {
            instancedDraws.emplace_back( o->_modelMaterialEncoded(), o );
        }
        else
        {
            normalDraws.push_back(o);
        }
    }

    end = std::chrono::high_resolution_clock::now();

    std::cout<<std::format("Draw splitting took: {}\n", std::chrono::duration_cast<std::chrono::microseconds>(end - start));

    if(instancedDraws.size() > 0)
    {
        start = std::chrono::high_resolution_clock::now();

        //sort so that all objects with the same key are contigous in memory
        std::sort(instancedDraws.begin(), instancedDraws.end(), [](const ObjectInstance& a, const ObjectInstance& b) { return a.key < b.key; });

        uint64_t lastKey = instancedDraws.size() > 0 ? instancedDraws[0].key : 0;
        size_t lastKeyIndex = 0;

        std::vector<std::pair<uint64_t, uint64_t>> drawCounts = { { instancedDraws[0].key, 0 } };

        for(size_t i = 0; i < instancedDraws.size(); i++)
        {
            uint64_t currentKey = instancedDraws[i].key;
            uint64_t currentKeyIndex = 0;
            if(currentKey == lastKey) [[likely]]
            {
                drawCounts[lastKeyIndex].second++;
                currentKeyIndex = lastKeyIndex;
            }
            else
            {
                drawCounts.emplace_back(instancedDraws[i].key, 1);
                currentKeyIndex = drawCounts.size() - 1;
            }

            lastKey = currentKey;
            lastKeyIndex = currentKeyIndex;
        }

        end = std::chrono::high_resolution_clock::now();

        std::cout<<std::format("Instanced draw preparation took: {}\n", std::chrono::duration_cast<std::chrono::microseconds>(end - start));

        std::chrono::microseconds allBufferConstructions(0), allSlotConstructions(0);

        start = std::chrono::high_resolution_clock::now();

        size_t prevOffset = 0, prevCount = 0;
        for(auto [key, count] : drawCounts)
        {
            size_t offset = prevOffset + prevCount;
            prevOffset = offset;
            prevCount = count;

            if(count == 1) [[unlikely]]
            {
                normalDraws.push_back(instancedDraws[offset].object);
                continue;
            }

            Object* firstObject = instancedDraws[offset].object;

            if (descriptorAllocator) [[likely]]
            {
                commandBuffer.bindDescriptorSets(
                    vk::PipelineBindPoint::eGraphics,
                    firstObject->_material()->_instancedPipeline()->_layout(),
                    0,
                    descriptorAllocator->globalDescriptorSet->set.getRes(),
                    {}
                );
            }

            //Push Constants
            //Model component of the push constants is unused
            commandBuffer.pushConstants(firstObject->_material()->_instancedPipeline()->_layout(), vk::ShaderStageFlagBits::eVertex, 0, sizeof(PushConstants), firstObject->_pushConstants(camera));

            //Draw
            commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, firstObject->_material()->_instancedPipeline()->_pipeline());

            //Create or reuse the instance buffer
            const size_t instanceDataChunkSize = firstObject->_instanceData().second;
            const size_t instanceDataSize = count * instanceDataChunkSize;

            Buffer<std::vector<std::byte>, vk::BufferUsageFlagBits::eVertexBuffer>* instanceBuffer = nullptr;

            auto bufferStart = std::chrono::high_resolution_clock::now();

            auto bufferIT = std::find_if(instanceBuffers.begin(), instanceBuffers.end(), 
            [key](const std::pair<uint64_t, Buffer<std::vector<std::byte>, vk::BufferUsageFlagBits::eVertexBuffer>*>& a){ return a.first == key; }
            );

            if(bufferIT != instanceBuffers.end())
            {
                instanceBuffer = bufferIT->second;

                if(instanceBuffer->_size() < instanceDataSize)
                {
                    std::vector<std::byte> newInstanceData;
                    newInstanceData.resize(instanceDataSize - instanceBuffer->_size());
                    decltype(instanceBuffer) newInstanceBuffer = instanceBufferAllocator.allocate(instanceBuffer, std::move(newInstanceData), engine);

                    instanceBufferAllocator.free(instanceBuffer);

                    instanceBuffer = newInstanceBuffer;

                    bufferIT->second = instanceBuffer;
                }
                //Shrink impl later
            }
            else
            {
                std::vector<std::byte> instanceData;
                instanceData.resize(instanceDataSize);
                instanceBuffer = instanceBufferAllocator.allocate(std::move(instanceData), engine);
                instanceBuffers.emplace_back( key, instanceBuffer );
            }

            auto slotsIT = std::find_if(instanceSlots.begin(), instanceSlots.end(), [key](const std::pair<uint64_t, std::vector<InstanceDataSlot>>& a){ return a.first == key; });
            std::vector<InstanceDataSlot>* instanceDataSlots = nullptr;
            if(slotsIT == instanceSlots.end())
            {
                instanceSlots.emplace_back( key, std::vector<InstanceDataSlot>() );
                instanceDataSlots = &instanceSlots.back().second;
            }
            else
            {
                instanceDataSlots = &slotsIT->second;
            }
            
            auto slotsStart = std::chrono::high_resolution_clock::now();
            constructSlots(instanceDataSize, instanceDataChunkSize, instancedDraws.data() + offset, count, *instanceDataSlots, engine->_currentFrame(), *instanceBuffer);
            auto slotsEnd = std::chrono::high_resolution_clock::now();

            allSlotConstructions += std::chrono::duration_cast<std::chrono::microseconds>(slotsEnd - slotsStart);

            instanceBuffer->moveToGPU();

            auto bufferEnd = std::chrono::high_resolution_clock::now();

            allBufferConstructions += std::chrono::duration_cast<std::chrono::microseconds>(bufferEnd - bufferStart);

            std::array<vk::Buffer, 2> vertexBuffers = {
                firstObject->_model()->_vertexBuffer()._buffer(),
                instanceBuffer->_buffer()

            };
            std::array<vk::DeviceSize, 2> vertexBufferOffsets = {
                0,
                0
            };
            commandBuffer.bindVertexBuffers(0, vertexBuffers, vertexBufferOffsets);

            commandBuffer.bindIndexBuffer(firstObject->_model()->_indexBuffer()._buffer(), 0, vk::IndexType::eUint32);

            commandBuffer.drawIndexed(static_cast<uint32_t>(firstObject->_model()->_indexBuffer()._typedSize()), count, 0, 0, 0);
        }

        std::cout<<std::format("Buffer construction took: {}, on average. Of which: {}, was slots construction\n", allBufferConstructions / (float)drawCounts.size(), allSlotConstructions / (float)drawCounts.size());
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
}
