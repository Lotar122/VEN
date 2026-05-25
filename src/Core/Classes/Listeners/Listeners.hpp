#pragma once

#include "onSwapchainRecreation.hpp"
#include "onResize.hpp"
#include "onHandle.hpp"

#include <format>

namespace nihil
{
    enum class Listeners
    {
        onResize,
        onHandle,
        onSwapchainRecreation
    };
}

template<>
struct std::formatter<nihil::Listeners> : std::formatter<std::string_view>
{
    auto format(nihil::Listeners format, format_context& ctx) const
    {
        std::string_view name = "Unknown";

        switch(format)
        {
            case nihil::Listeners::onResize:
                name = "onResize";
                break;

            case nihil::Listeners::onHandle:
                name = "onHandle";
                break;

            case nihil::Listeners::onSwapchainRecreation:
                name = "onSwapchainRecreation";
                break;
        }

        return std::formatter<std::string_view>::format(name, ctx);
    }
};