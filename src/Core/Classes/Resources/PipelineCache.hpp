#pragma once

#include "Classes/Resource/Resource.hpp"

#include <vulkan/vulkan.hpp>

namespace nihil
{
    template <>
    class Resource<vk::PipelineCache> : public ResourceAbstract<vk::PipelineCache>
    {
    protected:
        vk::Device device;
    public:
        Resource(vk::PipelineCache _res, vk::Device _device)
        {
            res = _res;
            device = _device;
            assigned = true;

            Logger::Log("The specialized constructor for vk::PipelineCache called.");
        }
        Resource() {Logger::Log("The specialized constructor for vk::PipelineCache called.");};
        void destroy() override
        {
            if (destroyed || !assigned) return;
            device.destroyPipelineCache(res);
            destroyed = true;

            Logger::Log("Destroying vk::PipelineCache");
        }
        ~Resource() override 
        {
            if(!destroyed) Logger::Warn("A vk::PipelineCache has gone out of scope without being destroyed, destroying automatically.");
            destroy();
        };

        virtual void assignRes(vk::PipelineCache _res, vk::Device _device)
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