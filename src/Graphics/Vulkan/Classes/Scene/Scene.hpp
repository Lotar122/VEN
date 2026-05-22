#pragma once

#include <unordered_map>
#include <vector>

#include "Classes/BlockAllocator/BlockAllocator.hpp"

#include <vulkan/vulkan.hpp>

#include "Classes/ECSAllocator/ECSAllocator.hpp"
#include "Classes/Object/Object.hpp"
#include "Classes/DescriptorAllocator/DescriptorAllocator.hpp"
#include "Structs/BVHNode.hpp"

struct InstanceDataSlot
{
    size_t prevOffset = std::numeric_limits<size_t>::max();
    size_t lastResident = std::numeric_limits<size_t>::max();
    size_t currentResident = std::numeric_limits<size_t>::max();
    size_t currentResidentRenderIndex = std::numeric_limits<size_t>::max();
    
    uint64_t prevAssignedFrame = std::numeric_limits<uint64_t>::max();

    InstanceDataSlot(size_t _currentResident, size_t _currentResidentRenderIndex) : 
        currentResident(_currentResident), currentResidentRenderIndex(_currentResidentRenderIndex) {};

    InstanceDataSlot() {};
};

namespace nihil::graphics
{
    class Camera;
    class Engine;
    class Model;

    class Scene
    {
        Engine* engine = nullptr;
        Carbo::BlockAllocator<Buffer<std::vector<std::byte>, vk::BufferUsageFlagBits::eVertexBuffer>> instanceBufferAllocator;
        Carbo::ECSAllocator<BVHNode> BVHNodeAllocator;
        std::vector<size_t> BVHIndices;
        std::vector<size_t> toRender;

        //instead of Model* use two asset ids (model, material) packed into a uint64_t
        std::unordered_map<uint64_t, std::pair<Buffer<std::vector<std::byte>, vk::BufferUsageFlagBits::eVertexBuffer>*, std::vector<InstanceDataSlot>>> instanceBuffers;

        std::unordered_map<uint64_t, std::vector<Object*>> instancedDraws;
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
            for (auto& it : instanceBuffers)
            {
                it.second.first->~Buffer();
            }

            for (auto p : debugVertexBuffers)
            {
                delete p;
            }

            if (debugIndexBuffer) delete debugIndexBuffer;
        }
    };
}