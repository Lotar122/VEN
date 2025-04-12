#pragma once

#include "Classes/Resource/Resource.hpp"

namespace nihil
{
    template <>
    class Resource<vk::PipelineLayout> : public ResourceAbstract<vk::PipelineLayout>
    {
    protected:
        vk::Device device;
    public:
        Resource(vk::PipelineLayout _res, vk::Device _device)
        {
            res = _res;
            device = _device;
            assigned = true;

            Logger::Log("The specialized constructor for vk::PipelineLayout called.");
        }
        Resource() {Logger::Log("The specialized constructor for vk::PipelineLayout called.");};
        void destroy() override
        {
            if(destroyed) return;
            device.destroyPipelineLayout(res);
            destroyed = true;

            Logger::Log("Destroying vk::PipelineLayout");
        }
        ~Resource() override 
        {
            if(!destroyed) Logger::Warn("A vk::PipelineLayout has gone out of scope without being destroyed, destroying automatically.");
            destroy();
        };

        virtual void assignRes(vk::PipelineLayout _res, vk::Device _device)
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