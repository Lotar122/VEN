#pragma once

#include <unordered_set>

#include <vulkan/vulkan.hpp>

#include "Classes/Object/Object.hpp"

#include "ShortHeap.hpp"

namespace nihil::graphics
{
    class Engine;

    class Scene
    {
        Engine* engine = nullptr;
    public:
        std::vector<Object*> objects;

        ShortHeap bufferHeap = ShortHeap(1024, 1024);
        std::unordered_map<size_t, std::pair<Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>*, bool>> instanceBuffers;

        inline void addObject(Object* object) { objects.push_back(object); };

        inline void use() { for (Object* o : objects) { o->use(); } };
        inline void unuse() { for (Object* o : objects) { o->unuse(); } };

        void recordCommands(vk::CommandBuffer& commandBuffer, Camera* camera);

        Scene(Engine* _engine)
        {
            assert(_engine != nullptr);

            engine = _engine;
        }

        ~Scene()
        {
            for (auto& b : instanceBuffers)
            {
                delete b.second.first;
            }
        }
    };
}