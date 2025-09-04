#pragma once

#include "Classes/Resource/Resource.hpp"

#include <vulkan/vulkan.hpp>

namespace nihil
{
    template <>
    class Resource<vk::Framebuffer> : public ResourceAbstract<vk::Framebuffer>
    {
    protected:
        vk::Device device;
    public:
        Resource(vk::Framebuffer _res, vk::Device _device)
        {
            res = _res;
            device = _device;
            assigned = true;

            Logger::Log("The specialized constructor for vk::Framebuffer called.");
        }
        Resource() {Logger::Log("The specialized constructor for vk::Framebuffer called.");};
        void destroy() override
        {
            if (destroyed || !assigned) return;
            device.destroyFramebuffer(res);
            destroyed = true;

            Logger::Log("Destroying vk::Framebuffer");
        }
        ~Resource() override 
        {
            if(!destroyed) Logger::Warn("A vk::Framebuffer has gone out of scope without being destroyed, destroying automatically.");
            destroy();
        };

        virtual void assignRes(vk::Framebuffer _res, vk::Device _device)
        {
            if(!assigned)
            {
                device = _device;
                res = _res;
                
                assigned = true;
            }
            else
            {
                Logger::Exception(std::string("Cannot assign an already assigned resource"));
            }
        }

        //T getRes
        //T* getResP
    };
}