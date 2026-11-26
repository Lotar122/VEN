/**
 * @file Semaphore.hpp
 * @brief RAII specialization of Resource for vk::Semaphore.
 */
#pragma once

#include "Classes/Resource/Resource.hpp"

#include <vulkan/vulkan.hpp>

namespace nihil
{
    ///@brief Owning RAII handle for a vk::Semaphore.
    ///
    ///Holds the semaphore together with the logical device it was created on, and destroys it automatically when it goes out of scope.
    template <>
    class Resource<vk::Semaphore> : public ResourceAbstract<vk::Semaphore>
    {
    protected:
        ///The vulkan logical device, needed for destroying the resource.
        vk::Device device;
    public:
        ///@brief Moves a resource into a new handle.
        ///@param _res The semaphore handle to take ownership of.
        ///@param _device The logical device the semaphore was created on.
        Resource(vk::Semaphore _res, vk::Device _device)
        {
            res = _res;
            device = _device;
            assigned = true;

            Carbo::Logger::Log("The specialized constructor for vk::Semaphore called.");
        }
        ///@brief Constructs a new empty handle.
        ///@note The handle holds no resource until assignRes() is called.
        Resource() { Carbo::Logger::Log("The specialized constructor for vk::Semaphore called.");};
        ///@brief Destroys the resource in the handle.
        ///@note Does nothing if the resource was already destroyed or was never assigned.
        void destroy() override
        {
            if (destroyed || !assigned) return;
            device.destroySemaphore(res);
            destroyed = true;

            Carbo::Logger::Log("Destroying vk::Semaphore");
        }
        ///@brief The destructor for the resource (RAII).
        ///@note Logs a warning if the resource was not destroyed explicitly, then calls destroy().
        ~Resource() override
        {
            if(!destroyed) Carbo::Logger::Warn("A vk::Semaphore has gone out of scope without being destroyed, destroying automatically.");
            destroy();
        };

        ///@brief Assigns the resource if the handle is empty.
        ///@param _res The semaphore handle to take ownership of.
        ///@param _device The logical device the semaphore was created on.
        ///@note If the handle is already assigned, an exception is reported through Carbo::Logger::Exception and nothing changes.
        virtual void assignRes(vk::Semaphore _res, vk::Device _device)
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
