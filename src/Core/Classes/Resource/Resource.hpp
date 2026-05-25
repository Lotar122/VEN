#pragma once

#include "Classes/Logger/Logger.hpp"

namespace nihil
{
    template <typename T>
    class ResourceAbstract
    {
    protected:
        T res;
    public:
        virtual void destroy() { Carbo::Logger::Error("Called an empty destroy function for a Resource."); };
        ResourceAbstract() {};
        virtual ~ResourceAbstract() {};
        // {
        //     destroy();
        //     destroyed = true;
        // }

        //make this implicitly castable to T
        inline operator T&() { return res; };

        virtual T getRes()
        {
            if(!assigned) return (T)nullptr;
            return res;
        }

        virtual T* getResP()
        {
            return &res;
        }

        virtual T& getResR()
        {
            return res;
        }

        bool destroyed = false;
        bool assigned = false;
    };

    template<typename T>
    class Resource : public ResourceAbstract<T>
    {
        Resource(T _res) : ResourceAbstract<T>(_res) {};
    };
}