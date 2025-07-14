#pragma once

#include <unordered_map>
#include <vector>

#include "ShortHeap.hpp"

#include <vulkan/vulkan.hpp>

#include "Classes/Object/Object.hpp"

namespace nihil::graphics
{
    class Camera;
    class Engine;
    class Model;

    class Scene
    {
        Engine* engine = nullptr;
        ShortHeap bufferHeap = ShortHeap(1024, 1024);

        std::unordered_map<Model*, std::vector<Object*>> instancedDraws;
        std::vector<Object*> normalDraws;

        std::vector<Object*> objects;

        std::unordered_map<Model*, Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>*> instanceBuffers;
    public:

        inline void addObject(Object* object) { objects.push_back(object); };
        void addObjects(const Object** newObjects, size_t size);
        void addObjects(Object* newObjects, size_t size);
        void addObjects(const std::vector<Object*>& newObjects);
        void addObjects(std::vector<Object>& newObjects);

        inline void use() { for (Object* o : objects) { o->use(); } };
        inline void unuse() { for (Object* o : objects) { o->unuse(); } };

        //* CONTROL FLOW
        //When rendering find all the instanced draws, using the method in the SceneOld
        //find the corresponding models buffer for each instanced draw
        //if the object was not modified then do not modify the buffer.
        //if an object was modified modify just its data in the buffer.

        void recordCommands(vk::CommandBuffer& commandBuffer, Camera* camera);

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