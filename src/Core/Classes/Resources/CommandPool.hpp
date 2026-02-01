#pragma once

#include "Classes/Resource/Resource.hpp"

#include <vulkan/vulkan.hpp>

namespace nihil
{
    template <>
    class Resource<vk::CommandPool> : public ResourceAbstract<vk::CommandPool>
    {
    protected:
        vk::Device device;
    public:
        Resource(vk::CommandPool _res, vk::Device _device)
        {
            res = _res;
            device = _device;
            assigned = true;

            Carbo::Logger::Log("The specialized constructor for vk::CommandPool called.");
        }
        Resource() {Carbo::Logger::Log("The specialized constructor for vk::CommandPool called.");};
        void destroy() override
        {
            if (destroyed || !assigned) return;
            device.destroyCommandPool(res);
            destroyed = true;

            Carbo::Logger::Log("Destroying vk::CommandPool");
        }
        ~Resource() override 
        {
            if(!destroyed) Carbo::Logger::Warn("A vk::CommandPool has gone out of scope without being destroyed, destroying automatically.");
            destroy();
        };

        virtual void assignRes(vk::CommandPool _res, vk::Device _device)
        {
            if(!assigned)
            {
                device = _device;
                res = _res;
                
                assigned = true;
            }
            else
            {
                Carbo::Logger::Exception(std::string("Cannot assign an already assigned resource"));
            }
        }

        //T getRes
        //T* getResP
    };
}