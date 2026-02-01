#pragma once

#include "Classes/Resource/Resource.hpp"

#include <vulkan/vulkan.hpp>

namespace nihil
{
    template <>
    class Resource<vk::DescriptorSetLayout> : public ResourceAbstract<vk::DescriptorSetLayout>
    {
    protected:
        vk::Device device;
    public:
        Resource(vk::DescriptorSetLayout _res, vk::Device _device)
        {
            res = _res;
            device = _device;
            assigned = true;

            Carbo::Logger::Log("The specialized constructor for vk::DescriptorSetLayout called.");
        }
        Resource() { Carbo::Logger::Log("The specialized constructor for vk::DescriptorSetLayout called."); };
        void destroy() override
        {
            if (destroyed || !assigned) return;
            device.destroyDescriptorSetLayout(res);
            destroyed = true;

            Carbo::Logger::Log("Destroying vk::DescriptorSetLayout");
        }
        ~Resource() override
        {
            if (!destroyed) Carbo::Logger::Warn("A vk::DescriptorSetLayout has gone out of scope without being destroyed, destroying automatically.");
            destroy();
        };

        virtual void assignRes(vk::DescriptorSetLayout _res, vk::Device _device)
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