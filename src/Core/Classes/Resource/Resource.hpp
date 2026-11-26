#pragma once

#include "Classes/Logger/Logger.hpp"

namespace nihil
{
    ///The abstract implementation of the owning RAII helper.
    //
    ///@tparam T The type of the object
    template <typename T>
    class ResourceAbstract
    {
    protected:
        ///The resource held by the helper
        T res;
    public:
        ///The virtual function to destroy the object held by the helper.
        virtual void destroy() { Carbo::Logger::Error("Called an empty destroy function for a Resource."); };
        ResourceAbstract() : res{} {};
        ResourceAbstract(T&& _res) : res{_res} { assigned = true; };
        virtual ~ResourceAbstract() {};
        // {
        //     destroy();
        //     destroyed = true;
        // }

        ///Make this implicitly castable to T
        inline operator T&() { return res; };

        ///Gets a copy of the resource in the handle
        virtual T getRes()
        {
            if(!assigned) return (T)nullptr;
            return res;
        }

        ///Gets the pointer to the resource in the handle
        virtual T* getResP()
        {
            return &res;
        }

        ///Gets the reference to the resource in the handle
        virtual T& getResR()
        {
            return res;
        }

        //TODO: Pack these into a single uint8_t
        ///A flag to check whether the resource was destroyed
        bool destroyed = false;
        ///A flag to check whether a resource was assigned
        bool assigned = false;
    };

    ///An owning RAII helper for VEN objects.
    template<typename T>
    class Resource : public ResourceAbstract<T>
    {
    public:
        ///Moves a resource into a new handle
        Resource(T&& _res) : ResourceAbstract<T>(_res) {};
        ///Constructs a new empty handle
        Resource() : ResourceAbstract<T>() {};
    };
}