#pragma once

namespace nihil
{
    ///The virtual abstract class for the onResize event listener
    class onResizeListener
    {
    public:
        ///The event handler
        virtual void onResize() = 0;
    };
}