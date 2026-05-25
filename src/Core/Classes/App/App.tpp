#include "Classes/Listeners/Listeners.hpp"
#include "Macros/break_assert.hpp"
namespace nihil
{
    template<typename T>
    void App::addEventListener(T* listener, Listeners type)
    {
        if(!(type == Listeners::onResize || type == Listeners::onHandle)) [[unlikely]] Carbo::Logger::Exception("The App can only provide onHandle, onResize events. Provided type: {}", type);

        switch(type)
        {
            case Listeners::onResize:
            {
                onResizeListener* typedPointer = dynamic_cast<onResizeListener*>(listener);
                assert(typedPointer != nullptr);
                onResizeListeners.push_back(typedPointer);
            }
            break;
            case Listeners::onHandle:
            {
                onHandleListener* typedPointer = dynamic_cast<onHandleListener*>(listener);
                assert(typedPointer != nullptr);
                onHandleListeners.push_back(typedPointer);
            }
            break;
            default:
            break;
        }
    }
}