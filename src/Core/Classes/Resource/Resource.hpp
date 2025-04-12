#pragma once

namespace nihil
{
    template <typename T>
    class ResourceAbstract
    {
    protected:
        T res;
    public:
        virtual void destroy() { Logger::Error("Called an empty destroy function for a Resource."); };
        ResourceAbstract() {};
        virtual ~ResourceAbstract() {};
        // {
        //     destroy();
        //     destroyed = true;
        // }

        virtual T getRes()
        {
            if(!assigned) return (T)nullptr;
            return res;
        }

        virtual T* getResP()
        {
            return &res;
        }
        virtual void assignRes(T _res)
        {
            if(!assigned)
            {
                res = _res;
                assigned = true;
            }
            else
            {
                Logger::Exception(std::string("Cannot assign an already assigned resource"));
            }
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