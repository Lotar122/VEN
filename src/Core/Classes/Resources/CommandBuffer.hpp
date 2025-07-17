#pragma once

#include "Classes/Resource/Resource.hpp"

#include <vulkan/vulkan.hpp>

namespace nihil
{
    template <>
    class Resource<vk::CommandBuffer> : public ResourceAbstract<vk::CommandBuffer>
    {
    protected:
        vk::Device device;
        vk::CommandPool commandPool;
    public:
        Resource(vk::CommandBuffer _res, vk::Device _device, vk::CommandPool _commandPool)
        {
            res = _res;
            device = _device;
            commandPool = _commandPool;
            assigned = true;

            Logger::Log("The specialized constructor for vk::CommandBuffer called.");
        }
        Resource() {Logger::Log("The specialized constructor for vk::CommandBuffer called.");};
        void destroy() override
        {
            if(destroyed) return;
            device.freeCommandBuffers(commandPool, 1, &res);
            destroyed = true;

            Logger::Log("Destroying vk::CommandBuffer");
        }
        ~Resource() override 
        {
            if(!destroyed) Logger::Warn("A vk::CommandBuffer has gone out of scope without being destroyed, destroying automatically.");
            destroy();
        };

        virtual void assignRes(vk::CommandBuffer _res, vk::Device _device, vk::CommandPool _commandPool)
        {
            if(!assigned)
            {
                device = _device;
                commandPool = _commandPool;
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