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

        std::unordered_set<Model*> models;

        std::vector<Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>*> instanceBuffers;

        size_t instanceBufferSlabSize = 0;
        Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>* instanceBufferSlab = nullptr;
        
        inline void addObject(Object* object) { objects.push_back(object); models.insert(object->model); };

        inline void use() { for(Object* o : objects){ o->use(); } };
        inline void unuse() { for(Object* o : objects){ o->unuse(); } };

        void recordCommands(vk::CommandBuffer& commandBuffer, Camera* camera);

        Scene(Engine* _engine)
        {
            assert(_engine != nullptr);

            engine = _engine;
        }

        ~Scene()
        {
            // for (Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>* b : instanceBuffers)
            // {
            //     delete b;
            // }

            for(int i = 0; i < instanceBufferSlabSize / sizeof(Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>); i++)
            {
                if((instanceBufferSlab + i)->_engine() == engine) (instanceBufferSlab + i)->~Buffer();
            }

            free(instanceBufferSlab);
        }
    };
}