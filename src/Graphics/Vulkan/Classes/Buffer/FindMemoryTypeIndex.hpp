#pragma once

namespace nihil::graphics
{
    static uint32_t findMemoryTypeIndex(vk::PhysicalDeviceMemoryProperties memProperties, vk::MemoryRequirements memRequirements, vk::MemoryPropertyFlags memFlags)
    {
        uint32_t memoryTypeIndex = uint32_t(-1);
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
        {
            if (
                (memRequirements.memoryTypeBits & (1 << i)) &&
                (memProperties.memoryTypes[i].propertyFlags & memFlags)
                )
            {
                memoryTypeIndex = i;
                break;
            }
        }

        return memoryTypeIndex;
    }
}