#pragma once

#include "Classes/Resource/Resource.hpp"

#include <vulkan/vulkan.hpp>

namespace nihil
{
    template <>
    class Resource<vk::Semaphore> : public ResourceAbstract<vk::Semaphore>
    {
    protected:
        vk::Device device;
    public:
        Resource(vk::Semaphore _res, vk::Device _device)
        {
            res = _res;
            device = _device;
            assigned = true;

            Carbo::Logger::Log("The specialized constructor for vk::Semaphore called.");
        }
        Resource() { Carbo::Logger::Log("The specialized constructor for vk::Semaphore called.");};
        void destroy() override
        {
            if (destroyed || !assigned) return;
            device.destroySemaphore(res);
            destroyed = true;

            Carbo::Logger::Log("Destroying vk::Semaphore");
        }
        ~Resource() override 
        {
            if(!destroyed) Carbo::Logger::Warn("A vk::Semaphore has gone out of scope without being destroyed, destroying automatically.");
            destroy();
        };

        virtual void assignRes(vk::Semaphore _res, vk::Device _device)
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