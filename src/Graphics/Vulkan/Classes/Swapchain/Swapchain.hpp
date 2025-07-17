#pragma once

#include <vulkan/vulkan.hpp>

#include "Classes/Engine/Engine.hpp"

#include "Classes/Resources/Resources.hpp"
#include "Classes/Listeners/Listeners.hpp"

#include <set>

namespace nihil::graphics
{
    class Swapchain : public onResizeListener
    {
        vk::Extent2D extent;
        
        App* app = nullptr;
        Engine* engine = nullptr;

    public:
        struct Frame {
            vk::Image resolved;
            vk::ImageView resolvedView;
            vk::Image multisampled;
            vk::DeviceMemory multisampledImageMemory;
            vk::ImageView multisampledView;
            vk::Framebuffer frameBuffer;
            vk::CommandPool commandPool;
            vk::CommandBuffer commandBuffer;
            vk::Image depthBuffer;
            vk::DeviceMemory depthBufferMemory;
            vk::ImageView depthBufferView;

            vk::Semaphore imageAvailable, renderFinished;
            vk::Fence inFlightFence;
	    };
    private:
        std::vector<Frame> frames;

        RenderPass* basicRenderPass = nullptr;
        std::pair<uint32_t, uint32_t> queueFamilyIndices;

    public:
        const inline vk::Extent2D _extent() { return extent; };
        const inline uint16_t _width() { return extent.width; };
        const inline uint16_t _height() { return extent.height; };
        inline std::vector<Frame>& _frames() { return frames; };

        vk::SurfaceFormatKHR surfaceFormat;
        vk::Format depthFormat;
        vk::Format imageFormat;

        uint8_t imageCount = 0;
        uint8_t prefferedImageCount = 0;
        uint8_t frameIndex = 0;

        uint64_t sampleCount = 0;

        vk::PresentModeKHR presentMode;

        vk::SurfaceTransformFlagBitsKHR transform;
        vk::CompositeAlphaFlagBitsKHR alpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;

        Swapchain(App* _app, vk::PresentModeKHR _presentMode, uint8_t _prefferedImageCount, uint64_t _sampleCount, Engine* _engine);

        Resource<vk::SwapchainKHR> swapchain;

        void create(std::pair<uint32_t, uint32_t> queueFamilyIndices, RenderPass* _basicRenderPass, bool recreation = false);

        ~Swapchain();

        Frame* acquireNextFrame(uint32_t* _imageIndex = nullptr, uint32_t* _frameIndex = nullptr);
        void presentFrame(Frame& frame, uint32_t& imageIndex);

        void recreate();

    private:
        void onResize() final override;
    };
}