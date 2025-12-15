#pragma once

namespace nihil
{
    class onSwapchainRecreationListener
    {
    public:
        virtual void onSwapchainRecreation() = 0;
    };
}