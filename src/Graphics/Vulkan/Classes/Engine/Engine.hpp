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
        uint32_t presentQueueIndex;
        uint32_t renderQueueIndex;

        Resource<vk::SurfaceKHR> surface;

        Resource<vk::Device> device;

        App* app = nullptr;
        Swapchain* swapchain = nullptr;
        Renderer* renderer = nullptr;

        uint64_t lastAssetId = 0;

    public:
        Engine(App* _app, EngineArgs& args);
        ~Engine();

        inline uint64_t requestAssetId() { return requestAssetId++; };

        inline vk::Instance _instance() { return instance.getRes(); };
        inline vk::PhysicalDevice _physicalDevice() const { return physicalDevice; };
        inline vk::SurfaceKHR _surfaceKHR() { return surface.getRes(); };
        inline vk::Device _device() { return device.getRes(); };

        inline uint32_t _renderQueueIndex() const { return renderQueueIndex; };
        inline uint32_t _presentQueueIndex() const { return presentQueueIndex; };
        inline vk::Queue _renderQueue() const { return renderQueue; };
        inline vk::Queue _presentQueue() const { return presentQueue; };

        inline Renderer* _renderer() { return renderer; };

        inline Swapchain* _swapchain() { return swapchain; };

        inline App* _app() { return app; };

        inline void setSwapchain(Swapchain* _swapchain) { assert(_swapchain != nullptr); swapchain = _swapchain; };

        void createRenderer();
    private:
        vk::Instance CreateVKInstance(EngineArgs& args, Platform platform);
        vk::PhysicalDevice PickPhysicalDevice(vk::Instance _instance, PhysicalDevicePreferences& prefs);
        vk::SurfaceKHR GetSurfaceKHR(App* _app);

        std::pair<vk::DeviceQueueCreateInfo, vk::DeviceQueueCreateInfo> CreateVKQueueInfo(
            vk::Instance instance, vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface
        ); 
        vk::Device CreateVKLogicalDevice(
            vk::PhysicalDevice physicalDevice,
            const std::pair<vk::DeviceQueueCreateInfo, vk::DeviceQueueCreateInfo>& queueInfos,
            const std::vector<const char*>& requiredExtensions
        );
        std::pair<std::pair<vk::Queue, vk::Queue>, std::pair<uint32_t, uint32_t>> GetVKQueues(
            vk::Device _device, 
            const std::pair<vk::DeviceQueueCreateInfo, vk::DeviceQueueCreateInfo>& queueInfos
        ); 
    
        //? Empty for now
        void onResize() final override {};
    };
}