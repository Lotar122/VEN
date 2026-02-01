#pragma once

#include "Classes/Resource/Resource.hpp"

namespace nihil
{
    template <>
    class Resource<vk::ShaderModule> : public ResourceAbstract<vk::ShaderModule>
    {
    protected:
        vk::Device device;
    public:
        Resource(vk::ShaderModule _res, vk::Device _device)
        {
            res = _res;
            device = _device;
            assigned = true;

            Carbo::Logger::Log("The specialized constructor for vk::ShaderModule called.");
        }
        Resource() {Carbo::Logger::Log("The specialized constructor for vk::ShaderModule called.");};
        void destroy() override
        {
            if (destroyed || !assigned) return;
            device.destroyShaderModule(res);
            destroyed = true;

            Carbo::Logger::Log("Destroying vk::ShaderModule");
        }
        ~Resource() override 
        {
            if(!destroyed) Carbo::Logger::Warn("A vk::ShaderModule has gone out of scope without being destroyed, destroying automatically.");
            destroy();
        };

        virtual void assignRes(vk::ShaderModule _res, vk::Device _device)
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