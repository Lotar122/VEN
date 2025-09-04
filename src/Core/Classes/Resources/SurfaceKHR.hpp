#pragma once

namespace nihil
{
    template<>
    class Resource<vk::SurfaceKHR> : public ResourceAbstract<vk::SurfaceKHR>
    {
    protected:
        vk::Instance instance;
    public:
        Resource(vk::SurfaceKHR _res, vk::Instance _instance)
        {
            res = _res;
            instance = _instance;
            assigned = true;

            Logger::Log("The specialized constructor for vk::SurfaceKHR called.");
        }
        Resource() {Logger::Log("The specialized constructor for vk::SurfaceKHR called.");};
        void destroy() override
        {
            if (destroyed || !assigned) return;
            instance.destroySurfaceKHR(res);
            destroyed = true;

            Logger::Log("Destroying vk::SurfaceKHR");
        }
        ~Resource() override 
        {
            if(!destroyed) Logger::Warn("A vk::SurfaceKHR has gone out of scope without being destroyed, destroying automatically.");
            destroy();
        };

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
                Logger::Exception(std::string("Cannot assign an already assigned resource"));
            }
        }
    };
}