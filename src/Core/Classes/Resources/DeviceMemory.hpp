/**
 * @file DeviceMemory.hpp
 * @brief RAII specialization of Resource for vk::DeviceMemory.
 */
#pragma once

#include "Classes/Resource/Resource.hpp"

#include <vulkan/vulkan.hpp>

namespace nihil
{
    ///@brief Owning RAII handle for a vk::DeviceMemory allocation.
    ///
    ///Holds the memory allocation together with the logical device it was made on, and frees it automatically when it goes out of scope.
    template <>
    class Resource<vk::DeviceMemory> : public ResourceAbstract<vk::DeviceMemory>
    {
    protected:
        ///The vulkan logical device, needed for freeing the resource.
        vk::Device device;
    public:
        ///@brief Moves a resource into a new handle.
        ///@param _res The device memory handle to take ownership of.
        ///@param _device The logical device the memory was allocated on.
        Resource(vk::DeviceMemory _res, vk::Device _device)
        {
            res = _res;
            device = _device;
            assigned = true;

            Carbo::Logger::Log("The specialized constructor for vk::DeviceMemory called.");
        }
        ///@brief Constructs a new empty handle.
        ///@note The handle holds no resource until assignRes() is called.
        Resource() { Carbo::Logger::Log("The specialized constructor for vk::DeviceMemory called.");};
        ///@brief Frees the device memory in the handle.
        ///@note Does nothing if the resource was already destroyed or was never assigned.
        void destroy() override
        {
            if (destroyed || !assigned) return;
            device.freeMemory(res);
            destroyed = true;

            Carbo::Logger::Log("Destroying vk::DeviceMemory");
        }
        ///@brief The destructor for the resource (RAII).
        ///@note Logs a warning if the resource was not destroyed explicitly, then calls destroy().
        ~Resource() override
        {
            if(!destroyed) Carbo::Logger::Warn("A vk::DeviceMemory has gone out of scope without being destroyed, destroying automatically.");
            destroy();
        };

        ///@brief Assigns the resource if the handle is empty.
        ///@param _res The device memory handle to take ownership of.
        ///@param _device The logical device the memory was allocated on.
        ///@note If the handle is already assigned, an exception is reported through Carbo::Logger::Exception and nothing changes.
        virtual void assignRes(vk::DeviceMemory _res, vk::Device _device)
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

        //T getRes
        //T* getResP
    };
}
