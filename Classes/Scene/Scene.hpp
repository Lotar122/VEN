#pragma once

#include <unordered_set>

#include <vulkan/vulkan.hpp>

#include "Classes/Object/Object.hpp"

namespace nihil::graphics
{
    class Scene
    {
    public:
        std::vector<Object*> objects;

        std::unordered_set<Model*> models;
        
        inline void addObject(Object* object) { objects.push_back(object); models.insert(object->model); };

        inline void use() { for(Object* o : objects){ o->use(); } };
        inline void unuse() { for(Object* o : objects){ o->unuse(); } };

        void recordCommands(vk::CommandBuffer& commandBuffer, Camera* camera);
    };
}