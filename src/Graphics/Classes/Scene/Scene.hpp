#pragma once

#include <unordered_set>

#include <vulkan/vulkan.hpp>

#include "Classes/Object/Object.hpp"

namespace nihil::graphics
{
    class Engine;

    class Scene
    {
        Engine* engine = nullptr;
    public:
        std::vector<Object*> objects;

        std::vector<Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>*> instanceBuffers;

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
            for (Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>* b : instanceBuffers)
            {
                delete b;
            }
        }
    };
}