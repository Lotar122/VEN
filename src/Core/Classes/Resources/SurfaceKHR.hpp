/**
 * @file SurfaceKHR.hpp
 * @brief RAII specialization of Resource for vk::SurfaceKHR.
 */
#pragma once

namespace nihil
{
    ///@brief Owning RAII handle for a vk::SurfaceKHR.
    ///
    ///Holds the surface together with the instance it was created from, and destroys it automatically when it goes out of scope.
    template<>
    class Resource<vk::SurfaceKHR> : public ResourceAbstract<vk::SurfaceKHR>
    {
    protected:
        ///The vulkan instance the surface belongs to, needed for destroying the resource.
        vk::Instance instance;
    public:
        ///@brief Moves a resource into a new handle.
        ///@param _res The surface handle to take ownership of.
        ///@param _instance The instance the surface was created from.
        Resource(vk::SurfaceKHR _res, vk::Instance _instance)
        {
            res = _res;
            instance = _instance;
            assigned = true;

            Carbo::Logger::Log("The specialized constructor for vk::SurfaceKHR called.");
        }
        ///@brief Constructs a new empty handle.
        ///@note The handle holds no resource until assignRes() is called.
        Resource() {Carbo::Logger::Log("The specialized constructor for vk::SurfaceKHR called.");};
        ///@brief Destroys the resource in the handle.
        ///@note Does nothing if the resource was already destroyed or was never assigned.
        void destroy() override
        {
            if (destroyed || !assigned) return;
            instance.destroySurfaceKHR(res);
            destroyed = true;

            Carbo::Logger::Log("Destroying vk::SurfaceKHR");
        }
        ///@brief The destructor for the resource (RAII).
        ///@note Logs a warning if the resource was not destroyed explicitly, then calls destroy().
        ~Resource() override
        {
            if(!destroyed) Carbo::Logger::Warn("A vk::SurfaceKHR has gone out of scope without being destroyed, destroying automatically.");
            destroy();
        };

        ///@brief Assigns the resource if the handle is empty.
        ///@param _res The surface handle to take ownership of.
        ///@param _instance The instance the surface was created from.
        ///@note If the handle is already assigned, an exception is reported through Carbo::Logger::Exception and nothing changes.
        virtual void assignRes(vk::SurfaceKHR _res, vk::Instance _instance)
        {
            if(!assigned)
            {
                instance = _instance;
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
