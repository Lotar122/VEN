#include "Scene.hpp"

#include "Classes/Camera/Camera.hpp"
#include "Classes/Model/Model.hpp"
#include "Classes/Engine/Engine.hpp"
#include "Classes/Pipeline/Pipeline.hpp"
#include "Functions/BVH/BVH.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_handles.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Functions/Enumerate/Enumerate.hpp"

#include "Enums/VisibilityQueryResult.hpp"

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

template<bool _freeList = false, bool _homeless = false>
void constructSlots(
    size_t instanceDataSize,
    size_t instanceDataChunkSize,
    const std::vector<Object*>& toRender, 
    std::vector<InstanceDataSlot>& slots, 
    size_t thisFrame, 
    Buffer<std::vector<std::byte>, vk::BufferUsageFlagBits::eVertexBuffer>& instanceBuffer, 
    std::vector<size_t>* __freeList = nullptr, std::vector<size_t>* __homeless = nullptr
)
{
    using FreeListType = std::conditional_t<_freeList, std::vector<size_t>&, std::vector<size_t>>;
    using HomelessType = std::conditional_t<_homeless, std::vector<size_t>&, std::vector<size_t>>;

    struct InitVec
    {
        static inline FreeListType initFreeList(std::vector<size_t>* p)
        {
            if constexpr (_freeList) {
                p->clear();
                return *p;
            }
            else
            {
                return FreeListType();
            }
        }
        static inline HomelessType initHomeless(std::vector<size_t>* p)
        {
            if constexpr (_homeless) {
                p->clear();
                return *p;
            }
            else
            {
                return HomelessType();
            }
        }
    };

    HomelessType homeless = InitVec::initHomeless(__homeless);
    FreeListType freeList = InitVec::initFreeList(__freeList);

    homeless.reserve(toRender.size());
    freeList.reserve(slots.size());

    if(toRender.size() < slots.size())
    {
        slots.erase(slots.end() - (slots.size() - toRender.size()), slots.end());
    }

    for(size_t i = 0; i < toRender.size(); i++)
    {
        Object* o = toRender[i];
        size_t prevDataSlot = o->prevDataSlot;
        if(prevDataSlot != std::numeric_limits<size_t>::max() && prevDataSlot < slots.size()) [[likely]]
        {
            InstanceDataSlot& slot = slots[prevDataSlot];
            if(slot.prevAssignedFrame == thisFrame) [[unlikely]]
            {
                homeless.push_back(i);
                continue;
            }

            slot.prevAssignedFrame = thisFrame;
            slot.currentResident = o->_getAssetId();
            slot.currentResidentRenderIndex = i;
        }
        else
        {
            homeless.push_back(i);
        }
    }

    for(size_t i = 0; i < slots.size(); i++)
    {
        if(slots[i].prevAssignedFrame != thisFrame) freeList.push_back(i);
    }

    slots.reserve(slots.size() + homeless.size());

    while(!homeless.empty())
    {
        size_t homelessBack = homeless.back();
        size_t objectId = toRender[homelessBack]->_getAssetId();
        if(!freeList.empty()) 
        {
            InstanceDataSlot& slot = slots[freeList.back()];
            slot.prevAssignedFrame = thisFrame;
            slot.currentResident = objectId;
            slot.currentResidentRenderIndex = homelessBack;

            toRender[homelessBack]->prevDataSlot = freeList.back();

            freeList.pop_back();
        }
        else
        {
            slots.emplace_back( objectId, homelessBack );
            toRender[homelessBack]->prevDataSlot = slots.size() - 1;
        }

        homeless.pop_back();
    }

    if(!freeList.empty()) Carbo::Logger::Exception("Non-integral slots");

    instanceBuffer.beginUpdateRecording();

    for(size_t i = 0; i < slots.size(); i++)
    {
        InstanceDataSlot& slot = slots[i];
        Object* o = toRender[slot.currentResidentRenderIndex];

        bool dirty = o->lastModifiedFrame == thisFrame || (slot.prevOffset != i && slot.prevOffset != std::numeric_limits<size_t>::max()) || slot.lastResident != slot.currentResident;
        if(dirty)
            //std::memcpy(reinterpret_cast<char*>(*mem + i), reinterpret_cast<char*>(&o->data), sizeof(InstanceData));
            instanceBuffer.update(reinterpret_cast<const std::byte*>(o->_instanceData().first), vk::BufferCopy{ 0, i * o->_instanceData().second, o->_instanceData().second });

        slot.prevOffset = i;
        slot.lastResident = slots[i].currentResident;
    }

    instanceBuffer.executeRecordedUpdates();
}

void Scene::recordCommands(vk::CommandBuffer& commandBuffer, Camera* camera, Pipeline* debugPipeline, DescriptorAllocator* descriptorAllocator)
{
    instancedDraws.clear();
    normalDraws.clear();
    BVHNodeAllocator.reset();
    toRender.clear();
    BVHIndices.resize(objects.size());
    toRender.reserve(objects.size());

    for(int i = 0; i < BVHIndices.size(); ++i)
    {
        BVHIndices[i] = i;
    }

    //Build and cull BVH
    size_t BVHRoot = buildBVH(objects, BVHIndices, 0, objects.size(), BVHNodeAllocator);
    cullBVH(BVHRoot, camera->_planes(), BVHNodeAllocator, toRender);

    float culledPercent = (1.0f - (static_cast<float>(toRender.size()) / static_cast<float>(objects.size()))) * 100.0f;

    Carbo::Logger::Log("Culled: {}% percent of objects.", culledPercent);

    //To reduce cache misses during rendering
    //std::sort(toRender.begin(), toRender.end());
    
    for(size_t objectIndex : toRender)
    {
        Object* o = objects[objectIndex];
        //if (AABB::isAABBVisible(o->_transformedAABB(), camera->_planes()) == VisibilityQueryResult::Outside) continue;

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

    std::vector<uint32_t> debugIndices =
    {
        // front
        0,1,2, 0,2,3,

        // back
        4,6,5, 4,7,6,

        // front
        0,5,1, 0,4,5,

        // back
        3,2,6, 3,6,7,

        // left
        0,3,7, 0,7,4,

        // right
        1,5,6, 1,6,2
    };

    /*Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer> debugVertexBuffer(debugVertices, engine);
    Buffer<uint32_t, vk::BufferUsageFlagBits::eIndexBuffer> debugIndexBuffer(debugIndices, engine);*/

    /*if(!debugVertexBuffer) debugVertexBuffer = new Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>(debugVertices, engine);*/
    if(!debugIndexBuffer) debugIndexBuffer = new Buffer<std::vector<uint32_t>, vk::BufferUsageFlagBits::eIndexBuffer>(debugIndices, engine);
    debugIndexBuffer->moveToGPU();

    for (auto p : debugVertexBuffers)
    {
        delete p;
    }

    debugVertexBuffers.clear();

    //Display AABBs for debug
    for (size_t i : toRender)
    {
        Object* o = objects[i];
        glm::vec3 min = o->_transformedAABB().min;
        glm::vec3 max = o->_transformedAABB().max;

        std::vector<float> vertices = {
            min.x, min.y, max.z, // 0
            max.x, min.y, max.z, // 1
            max.x, max.y, max.z, // 2
            min.x, max.y, max.z, // 3
            min.x, min.y, min.z, // 4
            max.x, min.y, min.z, // 5
            max.x, max.y, min.z, // 6
            min.x, max.y, min.z  // 7
        };

        debugVertexBuffers.push_back(new Buffer<std::vector<float>, vk::BufferUsageFlagBits::eVertexBuffer>(vertices, engine));
        debugVertexBuffers.back()->moveToGPU();

        //Push Constants
        commandBuffer.pushConstants(debugPipeline->_layout(), vk::ShaderStageFlagBits::eVertex, 0, sizeof(PushConstants), o->_pushConstants(camera));

        //Draw
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, debugPipeline->_pipeline());
        commandBuffer.bindVertexBuffers(0, debugVertexBuffers.back()->_buffer(), {0});

        commandBuffer.bindIndexBuffer(debugIndexBuffer->_buffer(), 0, vk::IndexType::eUint32);

        commandBuffer.drawIndexed(static_cast<uint32_t>(debugIndexBuffer->_typedSize()), 1, 0, 0, 0);
    }

    //Multithread the recording by doing instanced on t1 and normal on t2. This will only decrease performance in our use case.
    //Make it split the work into 4 threads if the CPU has 6 or more cores

    for(auto& it : instancedDraws)
    {
        if(it.second.size() > 1) [[likely]]
        {
            if (descriptorAllocator) [[likely]]
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

            Buffer<std::vector<std::byte>, vk::BufferUsageFlagBits::eVertexBuffer>* instanceBuffer = nullptr;

            auto bufferIT = instanceBuffers.find(it.first);
            if(bufferIT != instanceBuffers.end()) 
            {
                instanceBuffer = bufferIT->second.first; 
            }
            else
            {
                std::vector<std::byte> instanceData;
                std::vector<InstanceDataSlot> instanceDataSlots;

                instanceData.resize(instanceDataSize);

                instanceBuffer = instanceBufferAllocator.allocate(std::move(instanceData), engine);

                //constructSlots(instanceDataSize, instanceDataChunkSize, it.second, instanceDataSlots, engine->_currentFrame(), *instanceBuffer);

                bufferIT = instanceBuffers.emplace(it.first, std::make_pair(instanceBuffer, std::move(instanceDataSlots))).first;
            }

            //if((int64_t)instanceDataSize != (int64_t)instanceBuffer->_size()) instanceDataSlots.clear();

            if(instanceDataSize > instanceBuffer->_size() || (int64_t)instanceDataSize - 1'000 >= (int64_t)instanceBuffer->_size())
            {
                std::vector<std::byte> instanceData;
                std::vector<InstanceDataSlot> instanceDataSlots;

                size_t newObjectCount = (instanceDataSize - instanceBuffer->_size()) / instanceDataChunkSize;

                instanceData.resize(instanceDataSize - instanceBuffer->_size());

                Buffer<std::vector<std::byte>, vk::BufferUsageFlagBits::eVertexBuffer>* newInstanceBuffer = instanceBufferAllocator.allocate(instanceBuffer, std::move(instanceData), engine);

                instanceBufferAllocator.free(instanceBuffer);

                instanceBuffer = newInstanceBuffer;

                

                bufferIT = instanceBuffers.find(it.first);
                bufferIT->second = std::make_pair(instanceBuffer, std::move(instanceDataSlots));
                // bufferIT->second.first = instanceBuffer;
                // bufferIT->second.second = std::move(instanceDataSlots);
            }

            constructSlots(instanceDataSize, instanceDataChunkSize, it.second, bufferIT->second.second, engine->_currentFrame(), *bufferIT->second.first);

            // instanceBuffer->beginUpdateRecording();

            // size_t lastWriteEnd;
            // for(auto [i, o] : Carbo::enumerate(it.second))
            // {
            //         instanceBuffer->update(
            //             reinterpret_cast<const std::byte*>(o->_instanceData().first), 
            //             { 0, i * o->_instanceData().second, o->_instanceData().second }
            //         );
            // }

            // instanceBuffer->executeRecordedUpdates();

            // //TODO: Multithread this

            instanceBuffer->moveToGPU();

            // Buffer<std::vector<std::byte>, vk::BufferUsageFlagBits::eVertexBuffer>* instanceBuffer = nullptr;

            // auto bufferIT = instanceBuffers.find(it.first);
            // if(bufferIT != instanceBuffers.end()) 
            // {
            //     instanceBuffer = bufferIT->second; 
            // }
            // else
            // {
            //     std::vector<std::byte> instanceData;

            //     instanceData.resize(instanceDataSize);

            //     for (int i = 0; i < it.second.size(); i++)
            //     {
            //         memcpy(reinterpret_cast<char*>(instanceData.data()) + (instanceDataChunkSize * i), it.second[i]->_instanceData().first, instanceDataChunkSize);
            //     }

            //     instanceBuffer = instanceBufferAllocator.allocate(instanceData, engine);

            //     instanceBuffers.insert(std::make_pair(it.first, instanceBuffer));
            // }

            // bool reconstructedInstanceBuffer = false;

            // if(instanceDataSize > instanceBuffer->_size() || (int64_t)instanceDataSize - 1'000 >= (int64_t)instanceBuffer->_size())
            // {
            //     std::vector<std::byte> instanceData;

            //     size_t newObjectCount = (instanceDataSize - instanceBuffer->_size()) / instanceDataChunkSize;

            //     instanceData.resize(instanceDataSize - instanceBuffer->_size());

            //     for (int i = it.second.size() - newObjectCount, c = 0; i < it.second.size(); i++, c++)
            //     {
            //         memcpy(reinterpret_cast<char*>(instanceData.data()) + (instanceDataChunkSize * c), it.second[i]->_instanceData().first, instanceDataChunkSize);
            //     }

            //     Buffer<std::vector<std::byte>, vk::BufferUsageFlagBits::eVertexBuffer>* newInstanceBuffer = instanceBufferAllocator.allocate(instanceBuffer, std::move(instanceData), engine);

            //     instanceBufferAllocator.free(instanceBuffer);

            //     instanceBuffer = newInstanceBuffer;

            //     reconstructedInstanceBuffer = true;

            //     bufferIT = instanceBuffers.find(it.first);
            //     bufferIT->second = instanceBuffer;
            // }

            // instanceBuffer->beginUpdateRecording();

            // for(auto [i, o] : Carbo::enumerate(it.second))
            // {
            //     if(o->modifiedThisFrame) 
            //     { 
            //         instanceBuffer->update(
            //             reinterpret_cast<const std::byte*>(o->_instanceData().first), 
            //             { 0, i * o->_instanceData().second, o->_instanceData().second }
            //         );
            //     }

            // }

            // instanceBuffer->executeRecordedUpdates();

            // // //TODO: Multithread this

            // instanceBuffer->moveToGPU();

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
}