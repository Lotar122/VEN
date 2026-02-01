#pragma once

#include "Classes/Resource/Resource.hpp"

namespace nihil
{
    template <>
    class Resource<vk::Buffer> : public ResourceAbstract<vk::Buffer>
    {
    protected:
        vk::Device device;
    public:
        Resource(vk::Buffer _res, vk::Device _device)
        {
            res = _res;
            device = _device;
            assigned = true;

            Carbo::Logger::Log("The specialized constructor for vk::Buffer called.");
        }
        Resource() {Carbo::Logger::Log("The specialized constructor for vk::Buffer called.");};
        void destroy() override
        {
            if(destroyed || !assigned) return;
            device.destroyBuffer(res);
            destroyed = true;

            Carbo::Logger::Log("Destroying vk::Buffer");
        }
        ~Resource() override 
        {
            if(!destroyed) Carbo::Logger::Warn("A vk::Buffer has gone out of scope without being destroyed, destroying automatically.");
            destroy();
        };

        virtual void assignRes(vk::Buffer _res, vk::Device _device)
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