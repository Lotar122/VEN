/**
 * @file CommandBuffer.hpp
 * @brief RAII specialization of Resource for vk::CommandBuffer.
 */
#pragma once

#include "Classes/Resource/Resource.hpp"

#include <vulkan/vulkan.hpp>

namespace nihil
{
    ///@brief Owning RAII handle for a vk::CommandBuffer.
    ///
    ///Holds the command buffer together with its device and command pool, and frees it automatically when it goes out of scope.
    template <>
    class Resource<vk::CommandBuffer> : public ResourceAbstract<vk::CommandBuffer>
    {
    protected:
        ///The vulkan logical device, needed for freeing the resource.
        vk::Device device;
        ///The command pool the command buffer was allocated from.
        vk::CommandPool commandPool;
    public:
        ///@brief Moves a resource into a new handle.
        ///@param _res The command buffer handle to take ownership of.
        ///@param _device The logical device the command buffer was allocated on.
        ///@param _commandPool The command pool the command buffer was allocated from.
        Resource(vk::CommandBuffer _res, vk::Device _device, vk::CommandPool _commandPool)
        {
            res = _res;
            device = _device;
            commandPool = _commandPool;
            assigned = true;

            Carbo::Logger::Log("The specialized constructor for vk::CommandBuffer called.");
        }
        ///@brief Constructs a new empty handle.
        ///@note The handle holds no resource until assignRes() is called.
        Resource() {Carbo::Logger::Log("The specialized constructor for vk::CommandBuffer called.");};
        ///@brief Frees the command buffer back to its command pool.
        ///@note Does nothing if the resource was already destroyed or was never assigned.
        void destroy() override
        {
            if (destroyed || !assigned) return;
            device.freeCommandBuffers(commandPool, 1, &res);
            destroyed = true;

            Carbo::Logger::Log("Destroying vk::CommandBuffer");
        }
        ///@brief The destructor for the resource (RAII).
        ///@note Logs a warning if the resource was not destroyed explicitly, then calls destroy().
        ~Resource() override
        {
            if(!destroyed) Carbo::Logger::Warn("A vk::CommandBuffer has gone out of scope without being destroyed, destroying automatically.");
            destroy();
        };

        ///@brief Assigns the resource if the handle is empty.
        ///@param _res The command buffer handle to take ownership of.
        ///@param _device The logical device the command buffer was allocated on.
        ///@param _commandPool The command pool the command buffer was allocated from.
        ///@note If the handle is already assigned, an exception is reported through Carbo::Logger::Exception and nothing changes.
        virtual void assignRes(vk::CommandBuffer _res, vk::Device _device, vk::CommandPool _commandPool)
        {
            if(!assigned)
            {
                device = _device;
                commandPool = _commandPool;
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
