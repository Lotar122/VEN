/**
 * @file Device.hpp
 * @brief RAII specialization of Resource for vk::Device.
 */
#pragma once

#include "Classes/Resource/Resource.hpp"

#include <vulkan/vulkan.hpp>

namespace nihil
{
    ///@brief Owning RAII handle for a vk::Device (logical device).
    ///
    ///Destroys the logical device automatically when it goes out of scope.
    template <>
    class Resource<vk::Device> : public ResourceAbstract<vk::Device>
    {
    public:

        ///@brief Moves a resource into a new handle.
        ///@param _res The logical device handle to take ownership of.
        Resource(vk::Device _res)
        {
            res = _res;
            assigned = true;

            Carbo::Logger::Log("The specialized constructor for vk::Device called.");
        }
        ///@brief Constructs a new empty handle.
        ///@note The handle holds no resource until assignRes() is called.
        Resource() {Carbo::Logger::Log("The specialized constructor for vk::Device called.");};
        ///@brief Destroys the logical device in the handle.
        ///@note Does nothing if the resource was already destroyed or was never assigned.
        void destroy() override
        {
            if (destroyed || !assigned) return;
            res.destroy();
            destroyed = true;

            Carbo::Logger::Log("Destroying vk::Device");
        }
        ///@brief The destructor for the resource (RAII).
        ///@note Logs a warning if the resource was not destroyed explicitly, then calls destroy().
        ~Resource() override
        {
            if(!destroyed) Carbo::Logger::Warn("A vk::Device has gone out of scope without being destroyed, destroying automatically.");
            destroy();
        };

        ///@brief Assigns the resource if the handle is empty.
        ///@param _res The logical device handle to take ownership of.
        ///@note If the handle is already assigned, an exception is reported through Carbo::Logger::Exception and nothing changes.
        virtual void assignRes(vk::Device _res)
        {
            if(!assigned)
            {
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
