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

            Logger::Log("The specialized constructor for vk::Buffer called.");
        }
        Resource() {Logger::Log("The specialized constructor for vk::Buffer called.");};
        void destroy() override
        {
            if(destroyed) return;
            device.destroyBuffer(res);
            destroyed = true;

            Logger::Log("Destroying vk::Buffer");
        }
        ~Resource() override 
        {
            if(!destroyed) Logger::Warn("A vk::Buffer has gone out of scope without being destroyed, destroying automatically.");
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
                Logger::Exception(std::string("Cannot assign an already assigned resource"));
            }
        }
    };
}