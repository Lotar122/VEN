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

#include "SwapchainKHR.hpp"