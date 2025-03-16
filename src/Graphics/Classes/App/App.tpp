namespace nihil
{
    template<typename T>
    void App::addEventListener(T* listener, Listeners type)
    {
        if(type == Listeners::onResize)
        {
            onResizeListener* typedPointer = dynamic_cast<onResizeListener*>(listener);
            assert(typedPointer != nullptr);
            onResizeListeners.push_back(typedPointer);
        }
    }
}