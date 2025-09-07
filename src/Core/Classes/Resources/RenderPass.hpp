#pragma once

#include "Classes/Resource/Resource.hpp"

namespace nihil
{
    template <>
    class Resource<vk::RenderPass> : public ResourceAbstract<vk::RenderPass>
    {
    protected:
        vk::Device device;
    public:
        Resource(vk::RenderPass _res, vk::Device _device)
        {
            res = _res;
            device = _device;
            assigned = true;

            Logger::Log("The specialized constructor for vk::RenderPass called.");
        }
        Resource() {Logger::Log("The specialized constructor for vk::RenderPass called.");};
        void destroy() override
        {
            if (destroyed || !assigned) return;
            device.destroyRenderPass(res);
            destroyed = true;

            Logger::Log("Destroying vk::RenderPass");
        }
        ~Resource() override 
        {
            if(!destroyed) Logger::Warn("A vk::RenderPass has gone out of scope without being destroyed, destroying automatically.");
            destroy();
        };

        virtual void assignRes(vk::RenderPass _res, vk::Device _device)
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
    };
}