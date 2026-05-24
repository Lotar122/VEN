#pragma once

#include <limits>
#include <vector>

#include "Classes/BlockAllocator/BlockAllocator.hpp"

#include <vulkan/vulkan.hpp>

#include "Classes/ECSAllocator/ECSAllocator.hpp"
#include "Classes/Object/Object.hpp"
#include "Classes/DescriptorAllocator/DescriptorAllocator.hpp"
#include "Structs/BVHNode.hpp"

namespace nihil::graphics
{
    class Camera;
    class Engine;
    class Model;

    class Scene
    {
        struct ObjectInstance
        {
            uint64_t key;
            Object* object;
        };

        struct InstanceDataSlot
        {
            size_t prevOffset = std::numeric_limits<size_t>::max();
            size_t lastResident = std::numeric_limits<size_t>::max();
            size_t currentResident = std::numeric_limits<size_t>::max();
            size_t currentResidentRenderIndex = std::numeric_limits<size_t>::max();
            
            uint64_t prevAssignedFrame = std::numeric_limits<uint64_t>::max();
            uint64_t prevWriteFrame = std::numeric_limits<size_t>::max();

            InstanceDataSlot(size_t _currentResident, size_t _currentResidentRenderIndex, uint64_t _prevAssignedFrame) : 
                currentResident(_currentResident), currentResidentRenderIndex(_currentResidentRenderIndex), prevAssignedFrame(_prevAssignedFrame) {};

            InstanceDataSlot() {};
        };

        template<bool _freeList = false, bool _homeless = false>
        static void constructSlots(
            size_t instanceDataSize,
            size_t instanceDataChunkSize,
            ObjectInstance* toRender,
            size_t toRenderSize,
            std::vector<InstanceDataSlot>& slots, 
            size_t thisFrame, 
            Buffer<std::vector<std::byte>, vk::BufferUsageFlagBits::eVertexBuffer>& instanceBuffer, 
            std::vector<size_t>* __freeList = nullptr, std::vector<size_t>* __homeless = nullptr
        );

        Engine* engine = nullptr;
        Carbo::BlockAllocator<Buffer<std::vector<std::byte>, vk::BufferUsageFlagBits::eVertexBuffer>> instanceBufferAllocator;
        Carbo::ECSAllocator<BVHNode> BVHNodeAllocator;
        std::vector<size_t> BVHIndices;
        std::vector<size_t> toRender;

        //instead of Model* use two asset ids (model, material) packed into a uint64_t
        std::vector<std::pair<uint64_t, Buffer<std::vector<std::byte>, vk::BufferUsageFlagBits::eVertexBuffer>*>> instanceBuffers;
        std::vector<std::pair<uint64_t, std::vector<InstanceDataSlot>>> instanceSlots;

        std::vector<ObjectInstance> instancedDraws;
        std::vector<Object*> normalDraws;

        std::vector<Object*> objects;

        //Debug buffers
        std::vector<Buffer<std::vector<float>, vk::BufferUsageFlagBits::eVertexBuffer>*> debugVertexBuffers;
        Buffer<std::vector<uint32_t>, vk::BufferUsageFlagBits::eIndexBuffer>* debugIndexBuffer = nullptr;

        //New instancing system resources
        std::vector<size_t> instanceDataSlotFreeList;
        std::vector<size_t> homelessData;

        // std::vector<InstanceDataSlot> instanceDataSlots;
    public:

        inline void addObject(Object* object) { objects.push_back(object); };
        void addObjects(const Object** newObjects, size_t size);
        void addObjects(Object* newObjects, size_t size);
        void addObjects(const std::vector<Object*>& newObjects);
        void addObjects(std::vector<Object>& newObjects);

        inline void use() { for (Object* o : objects) { o->use(); } };
        inline void unuse() { for (Object* o : objects) { o->unuse(); } };

        void recordCommands(vk::CommandBuffer& commandBuffer, Camera* camera, Pipeline* debugPipeline, DescriptorAllocator* descriptorAllocator = nullptr);

        Scene(Engine* _engine)
        {
            assert(_engine != nullptr);

            engine = _engine;
        }

        ~Scene()
        {
            for(auto& b : instanceBuffers)
            {
                b.second->~Buffer();
            }

            for (auto p : debugVertexBuffers)
            {
                delete p;
            }

            if (debugIndexBuffer) delete debugIndexBuffer;
        }
    };
}

#include "Scene.tpp"