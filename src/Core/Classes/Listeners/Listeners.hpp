#pragma once

#include "onSwapchainRecreation.hpp"
#include "onResize.hpp"
#include "onHandle.hpp"

namespace nihil
{
    enum class Listeners
    {
        onResize,
        onHandle,
        onSwapchainRecreation
    };
}