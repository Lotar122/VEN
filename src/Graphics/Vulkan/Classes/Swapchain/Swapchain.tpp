#include "Classes/Listeners/Listeners.hpp"
#include "Classes/Listeners/onSwapchainRecreation.hpp"

namespace nihil::graphics
{
    template<typename T>
    void Swapchain::addEventListener(T* listener, Listeners type)
    {
        switch(type)
        {
            case Listeners::onSwapchainRecreation:
            {
                onSwapchainRecreationListener* typedPointer = dynamic_cast<onSwapchainRecreationListener*>(listener);
                assert(typedPointer != nullptr);
                onSwapchainRecreationListeners.push_back(typedPointer);
            }
            break;
        }
    }
}