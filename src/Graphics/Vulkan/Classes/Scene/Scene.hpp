#pragma once

#include <unordered_map>
#include <vector>

#include "ShortHeap.hpp"

#include <vulkan/vulkan.hpp>

#include "Classes/Object/Object.hpp"
#include "Classes/DescriptorAllocator/DescriptorAllocator.hpp"

namespace nihil::graphics
{
    class Camera;
    class Engine;
    class Model;

    class Scene
    {
        Engine* engine = nullptr;
        ShortHeap bufferHeap = ShortHeap(1024, 1024);

        //instead of Model* use two asset ids (model, material) packed into a uint64_t
        std::unordered_map<uint64_t, Buffer<std::byte, vk::BufferUsageFlagBits::eVertexBuffer>*> instanceBuffers;

        std::unordered_map<uint64_t, std::vector<Object*>> instancedDraws;
        std::vector<Object*> normalDraws;

        std::vector<Object*> objects;
    public:

        inline void addObject(Object* object) { objects.push_back(object); };
        void addObjects(const Object** newObjects, size_t size);
        void addObjects(Object* newObjects, size_t size);
        void addObjects(const std::vector<Object*>& newObjects);
        void addObjects(std::vector<Object>& newObjects);

        inline void use() { for (Object* o : objects) { o->use(); } };
        inline void unuse() { for (Object* o : objects) { o->unuse(); } };

        void recordCommands(vk::CommandBuffer& commandBuffer, Camera* camera, DescriptorAllocator* descriptorAllocator = nullptr);

        Scene(Engine* _engine)
        {
            assert(_engine != nullptr);

            engine = _engine;
        }

        ~Scene()
        {
            for (auto& it : instanceBuffers)
            {
                it.second->~Buffer();
            }
        }
    };
}