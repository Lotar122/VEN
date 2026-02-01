#pragma once

#include "Classes/Resource/Resource.hpp"

#include <vulkan/vulkan.hpp>

namespace nihil
{
    template <>
    class Resource<vk::Sampler> : public ResourceAbstract<vk::Sampler>
    {
    protected:
        vk::Device device;
    public:
        Resource(vk::Sampler _res, vk::Device _device)
        {
            res = _res;
            device = _device;
            assigned = true;

            Carbo::Logger::Log("The specialized constructor for vk::Sampler called.");
        }
        Resource() { Carbo::Logger::Log("The specialized constructor for vk::Sampler called."); };
        void destroy() override
        {
            if (destroyed || !assigned) return;
            device.destroySampler(res);
            destroyed = true;

            Carbo::Logger::Log("Destroying vk::Sampler");
        }
        ~Resource() override
        {
            if (!destroyed) Carbo::Logger::Warn("A vk::Sampler has gone out of scope without being destroyed, destroying automatically.");
            destroy();
        };

        virtual void assignRes(vk::Sampler _res, vk::Device _device)
        {
            if (!assigned)
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