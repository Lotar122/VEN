#pragma once

namespace nihil
{
    ///The virtual abstract class for the onSwapchainRecreation event listener
    class onSwapchainRecreationListener
    {
    public:
        ///The event handler
        virtual void onSwapchainRecreation() = 0;
    };
}