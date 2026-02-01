#pragma once

#include "Classes/Resource/Resource.hpp"

#include <vulkan/vulkan.hpp>

namespace nihil
{
    template <>
    class Resource<vk::DescriptorSet> : public ResourceAbstract<vk::DescriptorSet>
    {
    protected:
        vk::Device device;
    public:
        Resource(vk::DescriptorSet _res, vk::Device _device)
        {
            res = _res;
            device = _device;
            assigned = true;

            Carbo::Logger::Log("The specialized constructor for vk::DescriptorSet called.");
        }
        Resource() { Carbo::Logger::Log("The specialized constructor for vk::DescriptorSet called."); };
        void destroy() override
        {
            if (destroyed || !assigned) return;
            //destroyed by reseting the pool
            destroyed = true;

            Carbo::Logger::Log("Destroying vk::DescriptorSet");
        }
        ~Resource() override
        {
            if (!destroyed) Carbo::Logger::Warn("A vk::DescriptorSet has gone out of scope without being destroyed, destroying automatically.");
            destroy();
        };

        virtual void assignRes(vk::DescriptorSet _res, vk::Device _device)
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