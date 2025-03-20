namespace nihil
{
    template<typename T>
    void App::addEventListener(T* listener, Listeners type)
    {
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
        }
    }
}