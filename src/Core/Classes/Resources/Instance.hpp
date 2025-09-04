#pragma once

#include "Classes/Resource/Resource.hpp"

#include <vulkan/vulkan.hpp>

namespace nihil
{
    template <>
    class Resource<vk::Instance> : public ResourceAbstract<vk::Instance>
    {
    public:

        Resource(vk::Instance _res)
        {
            res = _res;
            assigned = true;

            Logger::Log("The specialized constructor for vk::Instance called.");
        }
        Resource() {Logger::Log("The specialized constructor for vk::Instance called.");};
        void destroy() override
        {
            if (destroyed || !assigned) return;
            res.destroy();
            destroyed = true;

            Logger::Log("Destroying vk::Instance");
        }
        ~Resource() override 
        {
            if(!destroyed) Logger::Warn("A vk::Instance has gone out of scope without being destroyed, destroying automatically.");
            destroy();
        };

        //void assignRes

        //T getRes
        //T* getResP
    };
}