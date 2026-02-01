#pragma once

#include "Classes/Resource/Resource.hpp"

namespace nihil
{
    template <>
    class Resource<vk::Pipeline> : public ResourceAbstract<vk::Pipeline>
    {
    protected:
        vk::Device device;
    public:
        Resource(vk::Pipeline _res, vk::Device _device)
        {
            res = _res;
            device = _device;
            assigned = true;

            Carbo::Logger::Log("The specialized constructor for vk::Pipeline called.");
        }
        Resource() {Carbo::Logger::Log("The specialized constructor for vk::Pipeline called.");};
        void destroy() override
        {
            if (destroyed || !assigned) return;
            device.destroyPipeline(res);
            destroyed = true;

            Carbo::Logger::Log("Destroying vk::Pipeline");
        }
        ~Resource() override 
        {
            if(!destroyed) Carbo::Logger::Warn("A vk::Pipeline has gone out of scope without being destroyed, destroying automatically.");
            destroy();
        };

        virtual void assignRes(vk::Pipeline _res, vk::Device _device)
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
    };
}