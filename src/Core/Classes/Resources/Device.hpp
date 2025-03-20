#pragma once

#include "Classes/Resource/Resource.hpp"

#include <vulkan/vulkan.hpp>

namespace nihil
{
    template <>
    class Resource<vk::Device> : public ResourceAbstract<vk::Device>
    {
    public:

        Resource(vk::Device _res)
        {
            res = _res;
            assigned = true;

            Logger::Log("The specialized constructor for vk::Device called.");
        }
        Resource() {Logger::Log("The specialized constructor for vk::Device called.");};
        void destroy() override
        {
            if(destroyed) return;
            res.destroy();
            destroyed = true;

            Logger::Log("Destroying vk::Device");
        }
        ~Resource() override 
        {
            if(!destroyed) Logger::Warn("A vk::Device has gone out of scope without being destroyed, destroying automatically.");
            destroy();
        };

        //void assignRes

        //T getRes
        //T* getResP
    };
}