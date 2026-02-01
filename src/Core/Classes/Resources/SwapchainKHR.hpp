#pragma once

#include "Classes/Resource/Resource.hpp"

namespace nihil
{
    template <>
    class Resource<vk::SwapchainKHR> : public ResourceAbstract<vk::SwapchainKHR>
    {
    protected:
        vk::Device device;
    public:
        Resource(vk::SwapchainKHR _res, vk::Device _device)
        {
            res = _res;
            device = _device;
            assigned = true;

            Carbo::Logger::Log("The specialized constructor for vk::SwapchainKHR called.");
        }
        Resource() { Carbo::Logger::Log("The specialized constructor for vk::SwapchainKHR called.");};
        void destroy() override
        {
            if (destroyed || !assigned) return;
            device.destroySwapchainKHR(res);
            destroyed = true;

            Carbo::Logger::Log("Destroying vk::SwapchainKHR");
        }
        ~Resource() override 
        {
            if(!destroyed) Carbo::Logger::Warn("A vk::SwapchainKHR has gone out of scope without being destroyed, destroying automatically.");
            destroy();
        };

        void reassign(vk::SwapchainKHR _res)
        {
            device.destroySwapchainKHR(res);
            res = _res;
        }

        virtual void assignRes(vk::SwapchainKHR _res, vk::Device _device)
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