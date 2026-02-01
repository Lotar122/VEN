#pragma once

#include <vulkan/vulkan.hpp>
#include "Classes/Logger/Logger.hpp"

namespace nihil
{
    uint32_t findMemoryType(const vk::PhysicalDevice& physicalDevice, uint32_t typeFilter, vk::MemoryPropertyFlags properties) 
    {
        vk::PhysicalDeviceMemoryProperties memoryProperties = physicalDevice.getMemoryProperties();

        for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i) 
        {
            if ((typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) return i;
        }

        Carbo::Logger::Exception("Failed to find suitable memory type!");

        return 0;
    }
}