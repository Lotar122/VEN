/**
 * @file SwapchainKHR.hpp
 * @brief RAII specialization of Resource for vk::SwapchainKHR.
 */
#pragma once

#include "Classes/Resource/Resource.hpp"

namespace nihil
{
    ///@brief Owning RAII handle for a vk::SwapchainKHR.
    ///
    ///Holds the swapchain together with the logical device it was created on, and destroys it automatically when it goes out of scope.
    ///Unlike the other resources, the swapchain can be replaced with reassign() when it has to be recreated.
    template <>
    class Resource<vk::SwapchainKHR> : public ResourceAbstract<vk::SwapchainKHR>
    {
    protected:
        ///The vulkan logical device, needed for destroying the resource.
        vk::Device device;
    public:
        ///@brief Moves a resource into a new handle.
        ///@param _res The swapchain handle to take ownership of.
        ///@param _device The logical device the swapchain was created on.
        Resource(vk::SwapchainKHR _res, vk::Device _device)
        {
            res = _res;
            device = _device;
            assigned = true;

            Carbo::Logger::Log("The specialized constructor for vk::SwapchainKHR called.");
        }
        ///@brief Constructs a new empty handle.
        ///@note The handle holds no resource until assignRes() is called.
        Resource() { Carbo::Logger::Log("The specialized constructor for vk::SwapchainKHR called.");};
        ///@brief Destroys the resource in the handle.
        ///@note Does nothing if the resource was already destroyed or was never assigned.
        void destroy() override
        {
            if (destroyed || !assigned) return;
            device.destroySwapchainKHR(res);
            destroyed = true;

            Carbo::Logger::Log("Destroying vk::SwapchainKHR");
        }
        ///@brief The destructor for the resource (RAII).
        ///@note Logs a warning if the resource was not destroyed explicitly, then calls destroy().
        ~Resource() override
        {
            if(!destroyed) Carbo::Logger::Warn("A vk::SwapchainKHR has gone out of scope without being destroyed, destroying automatically.");
            destroy();
        };

        ///@brief Replaces the swapchain with a newly created one, destroying the old one.
        ///@param _res The new swapchain handle that takes the place of the current one.
        ///@note The old swapchain is destroyed immediately, so it must no longer be in use.
        ///@note Does not update the stored device; the new swapchain must belong to the same device.
        void reassign(vk::SwapchainKHR _res)
        {
            device.destroySwapchainKHR(res);
            res = _res;
        }

        ///@brief Assigns the resource if the handle is empty.
        ///@param _res The swapchain handle to take ownership of.
        ///@param _device The logical device the swapchain was created on.
        ///@note If the handle is already assigned, an exception is reported through Carbo::Logger::Exception and nothing changes.
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
