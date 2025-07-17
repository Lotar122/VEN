#pragma once

#include <vulkan/vulkan.hpp>

#include <utility>

#include "Classes/App/App.hpp"
#include "Version.hpp"
#include "Logger.hpp"
#include "Classes/Resources/Resources.hpp"
#include "Platform.hpp"
#include "Classes/Renderer/Renderer.hpp"
#include "Classes/Listeners/Listeners.hpp"

#include "Concepts/Integer.hpp"

//! TODO: MAKE A std::mutex FOR THE MAIN COMMAND BUFFER, THIS SHOULD ENSURE THAT IT ISN'T RECORDED TO FROM TWO THREADS AT THE SAME TIME

namespace nihil::graphics
{
    struct PhysicalDevicePreferences
    {
        bool prefferDedicated = true;
        bool prefferLargerVRAM = true;
    };

    struct EngineArgs
    {
        Version appVersion;
        std::string appName;

        bool validationLayers = false;

        PhysicalDevicePreferences dPrefs;
    };

    class Swapchain;

    class Engine : public onResizeListener
    {
        friend class Renderer;

        Resource<vk::Instance> instance;
        vk::PhysicalDevice physicalDevice;

        vk::Queue presentQueue;
        vk::Queue renderQueue;
        vk::Queue transferQueue;
        uint32_t presentQueueIndex;
        uint32_t renderQueueIndex;
        uint32_t transferQueueIndex;

        vk::Fence transferFence;

        Resource<vk::SurfaceKHR> surface;

        Resource<vk::Device> device;

        vk::CommandPool mainCommandPool;
        vk::CommandBuffer mainCommandBuffer;

        App* app = nullptr;
        Swapchain* swapchain = nullptr;
        Renderer* renderer = nullptr;

        uint64_t lastAssetId = 0;

    public:
        Engine(App* _app, EngineArgs& args);
        ~Engine();

        inline uint64_t requestAssetId() { return lastAssetId++; };
        template<typename ReturnT>
        requires(Integer<ReturnT> || (std::is_enum_v<ReturnT> && Integer<std::underlying_type_t<ReturnT>>))
        ReturnT getMaxSampleCount()
        {
            auto props = physicalDevice.getProperties();
            auto colorSampleCounts = props.limits.framebufferColorSampleCounts;
            auto depthSampleCounts = props.limits.framebufferDepthSampleCounts;
            auto counts = colorSampleCounts & depthSampleCounts;

            if (counts & vk::SampleCountFlagBits::e64) return static_cast<ReturnT>(vk::SampleCountFlagBits::e64);
            if (counts & vk::SampleCountFlagBits::e32) return static_cast<ReturnT>(vk::SampleCountFlagBits::e32);
            if (counts & vk::SampleCountFlagBits::e16) return static_cast<ReturnT>(vk::SampleCountFlagBits::e16);
            if (counts & vk::SampleCountFlagBits::e8)  return static_cast<ReturnT>(vk::SampleCountFlagBits::e8);
            if (counts & vk::SampleCountFlagBits::e4)  return static_cast<ReturnT>(vk::SampleCountFlagBits::e4);
            if (counts & vk::SampleCountFlagBits::e2)  return static_cast<ReturnT>(vk::SampleCountFlagBits::e2);
            return static_cast<ReturnT>(vk::SampleCountFlagBits::e1);
        }

        inline vk::Instance _instance() { return instance.getRes(); };
        inline vk::PhysicalDevice _physicalDevice() const { return physicalDevice; };
        inline vk::SurfaceKHR _surfaceKHR() { return surface.getRes(); };
        inline vk::Device _device() { return device.getRes(); };

        inline vk::CommandPool& _mainCommandPool() { return mainCommandPool; };
        inline vk::CommandBuffer& _mainCommandBuffer() { return mainCommandBuffer; };

        inline uint32_t _renderQueueIndex() const { return renderQueueIndex; };
        inline uint32_t _presentQueueIndex() const { return presentQueueIndex; };
        inline uint32_t _transferQueueIndex() const { return transferQueueIndex; };
        inline vk::Queue _renderQueue() const { return renderQueue; };
        inline vk::Queue _presentQueue() const { return presentQueue; };
        inline vk::Queue _transferQueue() const { return transferQueue; };

        inline vk::Fence& _transferFence() { return transferFence; };

        inline Renderer* _renderer() { return renderer; };

        inline Swapchain* _swapchain() { return swapchain; };

        inline App* _app() { return app; };

        inline void setSwapchain(Swapchain* _swapchain) { assert(_swapchain != nullptr); swapchain = _swapchain; };

        void createRenderer();
    private:
        static vk::Instance CreateVKInstance(EngineArgs& args, Platform platform);
        static vk::PhysicalDevice PickPhysicalDevice(vk::Instance _instance, PhysicalDevicePreferences& prefs);
        static vk::SurfaceKHR GetSurfaceKHR(App* _app, vk::Instance _instance);

        static std::tuple<vk::DeviceQueueCreateInfo, vk::DeviceQueueCreateInfo, vk::DeviceQueueCreateInfo> CreateVKQueueInfo(
            vk::Instance instance, vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface
        );
        static vk::Device CreateVKLogicalDevice(
            vk::PhysicalDevice physicalDevice,
            std::tuple<vk::DeviceQueueCreateInfo, vk::DeviceQueueCreateInfo, vk::DeviceQueueCreateInfo>& queueInfos,
            const std::vector<const char*>& requiredExtensions
        );
        static std::pair<std::tuple<vk::Queue, vk::Queue, vk::Queue>, std::tuple<uint32_t, uint32_t, uint32_t>> GetVKQueues(
            vk::Device _device, 
            const std::tuple<vk::DeviceQueueCreateInfo, vk::DeviceQueueCreateInfo, vk::DeviceQueueCreateInfo>& queueInfos
        ); 
        static vk::CommandPool CreateVKCommandPool(vk::Device _device, uint32_t _renderQueueIndex);
        static vk::CommandBuffer CreateVKCommandBuffer(vk::Device _device, vk::CommandPool commandPool);
    
        //? Empty for now
        void onResize() final override {};
    };
}