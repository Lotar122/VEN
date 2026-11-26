/**
 * @file DescriptorSet.hpp
 * @brief RAII specialization of Resource for vk::DescriptorSet.
 */
#pragma once

#include "Classes/Resource/Resource.hpp"

#include <vulkan/vulkan.hpp>

namespace nihil
{
    ///@brief Owning RAII handle for a vk::DescriptorSet.
    ///
    ///Holds the descriptor set together with the logical device it was allocated on.
    ///The set is not freed individually; it is released together with its descriptor pool.
    template <>
    class Resource<vk::DescriptorSet> : public ResourceAbstract<vk::DescriptorSet>
    {
    protected:
        ///The vulkan logical device the descriptor set was allocated on.
        vk::Device device;
    public:
        ///@brief Moves a resource into a new handle.
        ///@param _res The descriptor set handle to take ownership of.
        ///@param _device The logical device the descriptor set was allocated on.
        Resource(vk::DescriptorSet _res, vk::Device _device)
        {
            res = _res;
            device = _device;
            assigned = true;

            Carbo::Logger::Log("The specialized constructor for vk::DescriptorSet called.");
        }
        ///@brief Constructs a new empty handle.
        ///@note The handle holds no resource until assignRes() is called.
        Resource() { Carbo::Logger::Log("The specialized constructor for vk::DescriptorSet called."); };
        ///@brief Marks the descriptor set as destroyed.
        ///@note No Vulkan call is made here: the set is freed when its pool is reset or destroyed.
        ///@note Does nothing if the resource was already destroyed or was never assigned.
        void destroy() override
        {
            if (destroyed || !assigned) return;
            //destroyed by reseting the pool
            destroyed = true;

            Carbo::Logger::Log("Destroying vk::DescriptorSet");
        }
        ///@brief The destructor for the resource (RAII).
        ///@note Logs a warning if the resource was not destroyed explicitly, then calls destroy().
        ~Resource() override
        {
            if (!destroyed) Carbo::Logger::Warn("A vk::DescriptorSet has gone out of scope without being destroyed, destroying automatically.");
            destroy();
        };

        ///@brief Assigns the resource if the handle is empty.
        ///@param _res The descriptor set handle to take ownership of.
        ///@param _device The logical device the descriptor set was allocated on.
        ///@note If the handle is already assigned, an exception is reported through Carbo::Logger::Exception and nothing changes.
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
