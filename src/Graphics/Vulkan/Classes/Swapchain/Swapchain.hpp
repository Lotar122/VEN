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
            Resource<vk::ImageView> resolvedView;
            Resource<vk::Image> multisampled;
            Resource<vk::ImageView> multisampledView;
            Resource<vk::DeviceMemory> multisampledImageMemory;

            Resource<vk::Framebuffer> frameBuffer;

            Resource<vk::CommandPool> commandPool;
            Resource<vk::CommandBuffer> commandBuffer;

            Resource<vk::Image> depthBuffer;
            Resource<vk::ImageView> depthBufferView;
            Resource<vk::DeviceMemory> depthBufferMemory;

            Resource<vk::Semaphore> imageAvailable, renderFinished;
            Resource<vk::Fence> inFlightFence;
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

        template<typename ReturnT = vk::SampleCountFlagBits>
        requires(Integer<ReturnT> || (std::is_enum_v<ReturnT> && Integer<std::underlying_type_t<ReturnT>>))
        inline ReturnT _sampleCount() { return static_cast<ReturnT>(sampleCount); };
    private:
        void onResize() final override;
    };
}