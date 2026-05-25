#include "Classes/Listeners/Listeners.hpp"
#include "Classes/Listeners/onSwapchainRecreation.hpp"
#include "Macros/break_assert.hpp"

namespace nihil::graphics
{
    template<typename T>
    void Swapchain::addEventListener(T* listener, Listeners type)
    {
        if(type != Listeners::onSwapchainRecreation) [[unlikely]] Carbo::Logger::Exception("The Swapchain can only provide onSwapchainRecreation event. Provided type: {}", type);

        switch(type)
        {
            case Listeners::onSwapchainRecreation:
            {
                onSwapchainRecreationListener* typedPointer = dynamic_cast<onSwapchainRecreationListener*>(listener);
                break_assert(typedPointer != nullptr);
                onSwapchainRecreationListeners.push_back(typedPointer);
            }
            break;
            default:
            break;
        }
    }
}