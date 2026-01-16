#pragma once

/*
    ! All of the Resources are immutable objects, except for the SwapchainKHR because Vulkan requires you to recreate the swapchain manually
*/

#include "Instance.hpp"
#include "Device.hpp"
#include "SurfaceKHR.hpp"
#include "Buffer.hpp"
#include "Pipeline.hpp"
#include "PipelineLayout.hpp"
#include "RenderPass.hpp"
#include "Shader.hpp"
#include "CommandBuffer.hpp"
#include "CommandPool.hpp"
#include "Fence.hpp"
#include "Semaphore.hpp"
#include "DeviceMemory.hpp"
#include "Image.hpp"
#include "ImageView.hpp"
#include "FrameBuffer.hpp"
#include "Sampler.hpp"
#include "DescriptorPool.hpp"
#include "DescriptorSet.hpp"
#include "DescriptorSetLayout.hpp"
#include "PipelineCache.hpp"

#include "SwapchainKHR.hpp"