#pragma once

#include <cstdint>
#include "vulkan/vulkan.hpp"

namespace nihil::graphics
{
    inline static uint32_t findMemoryTypeIndex(
        vk::PhysicalDeviceMemoryProperties memProperties,
        vk::MemoryRequirements memRequirements,
        vk::MemoryPropertyFlags memFlags)
    {
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
        {
            if ((memRequirements.memoryTypeBits & (1u << i)) &&
                ((memProperties.memoryTypes[i].propertyFlags & memFlags) == memFlags))
            {
                return i;
            }
        }

        return uint32_t(-1);
    }
    }