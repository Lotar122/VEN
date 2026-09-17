#pragma once

#include <cstdint>
#include "vulkan/vulkan.hpp"

namespace nihil::graphics
{
    ///Finds the suitable memory type for the given requirements
    //
    ///@param memProperties The properties of the memory type
    ///@param memRequirements The requirements of the memory type
    ///@param memFlags The flags for the memory type
    ///@return The most suitable memory types index
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