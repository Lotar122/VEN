#include "Engine.hpp"

#include "Constants.hpp"

#include "Classes/Swapchain/Swapchain.hpp"

using namespace nihil::graphics;
using namespace nihil;

vk::Instance Engine::CreateVKInstance(EngineArgs& args, Platform platform)
{
    vk::ApplicationInfo appInfo(
        args.appName.c_str(), // App name
        VK_MAKE_VERSION(args.appVersion.major, args.appVersion.minor, args.appVersion.patch), // App version
        "VEN", // Engine name
        VK_MAKE_VERSION(ENGINE_MAJOR, ENGINE_MINOR, ENGINE_PATCH), // Engine version
        VK_API_VERSION_1_2 // Vulkan version
    );

	std::vector<const char*> layers;

	if(args.validationLayers)
	{
		layers.push_back("VK_LAYER_KHRONOS_validation");
	}

    std::vector<const char*> extensions;

    extensions.push_back("VK_KHR_surface");

    switch(platform)
    {
        case Platform::LinuxWayland:
            extensions.push_back("VK_KHR_wayland_surface");
            break;
        case Platform::LinuxX11:
            extensions.push_back("VK_KHR_xcb_surface");
            break;
        case Platform::Windows:
            extensions.push_back("VK_KHR_win32_surface");
            break;
    }

	vk::InstanceCreateInfo instanceCreateInfo(
        {},
        &appInfo,
        layers.size(), layers.data(),
        extensions.size(), extensions.data()  //Extensions
    );

	vk::Instance instance;

	try {
		instance = vk::createInstance(instanceCreateInfo);
	}
	catch (const vk::SystemError& err) {
		Logger::Exception(std::string("An error has occured during instance creation: \n") + std::string(err.what()));
	}

	return instance;
}

vk::PhysicalDevice Engine::PickPhysicalDevice(vk::Instance _instance, PhysicalDevicePreferences& prefs)
{
    assert(_instance != nullptr);

    std::vector<vk::PhysicalDevice> availableDevices = _instance.enumeratePhysicalDevices();

    if(availableDevices.empty())
    {
        Logger::Exception("Couldnt find any vulkan capable device");
    }

    vk::PhysicalDevice pickedDevice;
    bool foundPrefferedDevice = false;

    if(prefs.prefferDedicated)
    {
        //in the future check which cards has best performance, for now it picks the one with the most VRAM
        uint64_t maxRam = 0;
        vk::PhysicalDevice tempMax;

        for(vk::PhysicalDevice& device : availableDevices)
        {
            vk::PhysicalDeviceProperties properties = device.getProperties();
            vk::PhysicalDeviceMemoryProperties memoryProperties = device.getMemoryProperties();

            if (properties.limits.maxSamplerAnisotropy >= 1.0f) {
                //std::cout << "GPU supports anisotropy: max "
                //    << properties.limits.maxSamplerAnisotropy << "x" << std::endl;
            }
            else {
                //std::cout << "GPU does NOT support anisotropic filtering!" << std::endl;
            }

            uint64_t deviceLocalMemory = 0;
            for (uint32_t i = 0; i < memoryProperties.memoryHeapCount; ++i) {
                if (memoryProperties.memoryHeaps[i].flags & vk::MemoryHeapFlagBits::eDeviceLocal) {
                    deviceLocalMemory += memoryProperties.memoryHeaps[i].size;
               }
            }

            if(deviceLocalMemory > maxRam && properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
            {
                maxRam = deviceLocalMemory;
                tempMax = device;

                foundPrefferedDevice = true;
            }
        }

        if(!foundPrefferedDevice)
        {
            for(vk::PhysicalDevice& device : availableDevices)
            {
                vk::PhysicalDeviceProperties properties = device.getProperties();
                vk::PhysicalDeviceMemoryProperties memoryProperties = device.getMemoryProperties();

                uint64_t deviceLocalMemory = 0;
                for (uint32_t i = 0; i < memoryProperties.memoryHeapCount; ++i) {
                    if (memoryProperties.memoryHeaps[i].flags & vk::MemoryHeapFlagBits::eDeviceLocal) {
                        deviceLocalMemory += memoryProperties.memoryHeaps[i].size;
                }
                }

                if(deviceLocalMemory > maxRam)
                {
                    maxRam = deviceLocalMemory;
                    tempMax = device;

                    foundPrefferedDevice = true;
                }
            }
        }

        pickedDevice = tempMax;
    }
    else
    {
        uint64_t maxRam = 0;
        vk::PhysicalDevice tempMax;

        for(vk::PhysicalDevice& device : availableDevices)
        {
            vk::PhysicalDeviceProperties properties = device.getProperties();
            vk::PhysicalDeviceMemoryProperties memoryProperties = device.getMemoryProperties();

            uint64_t deviceLocalMemory = 0;
            for (uint32_t i = 0; i < memoryProperties.memoryHeapCount; ++i) {
                if (memoryProperties.memoryHeaps[i].flags & vk::MemoryHeapFlagBits::eDeviceLocal) {
                    deviceLocalMemory += memoryProperties.memoryHeaps[i].size;
                }
            }

            if(deviceLocalMemory > maxRam)
            {
                maxRam = deviceLocalMemory;
                tempMax = device;

                foundPrefferedDevice = true;
            }
        }

        pickedDevice = tempMax;
    }

    if(!foundPrefferedDevice) Logger::Exception("Couldn't find a suitable device");

    return pickedDevice;
}

std::pair<vk::DeviceQueueCreateInfo, vk::DeviceQueueCreateInfo> Engine::CreateVKQueueInfo(
    vk::Instance instance, vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface
) 
{
    assert(instance != nullptr);
    assert(physicalDevice != nullptr);
    assert(surface != nullptr);

    auto queueFamilies = physicalDevice.getQueueFamilyProperties();
    uint32_t graphicsQueueFamilyIndex = -1;
    uint32_t presentQueueFamilyIndex = -1;

    for (uint32_t i = 0; i < queueFamilies.size(); ++i) {
        if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics) {
            graphicsQueueFamilyIndex = i;
            Logger::Log("Graphics Queue found");
        }
        vk::Bool32 presentSupport = physicalDevice.getSurfaceSupportKHR(i, surface);
        if (presentSupport) {
            presentQueueFamilyIndex = i;
            Logger::Log("Present Queue found");
        }

        if (graphicsQueueFamilyIndex != -1 && presentQueueFamilyIndex != -1) {
            break;
        }
    }

    if (graphicsQueueFamilyIndex == -1 || presentQueueFamilyIndex == -1) {
        throw std::runtime_error("Failed to find suitable queue families.");
    }

    float queuePriority = 1.0f;
    vk::DeviceQueueCreateInfo graphicsQueueCreateInfo({}, graphicsQueueFamilyIndex, 1, &queuePriority);
    vk::DeviceQueueCreateInfo presentQueueCreateInfo({}, presentQueueFamilyIndex, 1, &queuePriority);

    return std::make_pair(graphicsQueueCreateInfo, presentQueueCreateInfo);
}

vk::Device Engine::CreateVKLogicalDevice(
    vk::PhysicalDevice physicalDevice,
    const std::pair<vk::DeviceQueueCreateInfo, vk::DeviceQueueCreateInfo>& queueInfos,
    const std::vector<const char*>& requiredExtensions
) 
{
    assert(physicalDevice != nullptr);

    std::vector<vk::DeviceQueueCreateInfo> deviceQueueInfos = {queueInfos.first};
    if (queueInfos.first.queueFamilyIndex != queueInfos.second.queueFamilyIndex) {
        deviceQueueInfos.push_back(queueInfos.second);
    }

    vk::PhysicalDeviceFeatures deviceFeatures{};

    vk::DeviceCreateInfo deviceCreateInfo({}, deviceQueueInfos, nullptr, requiredExtensions, &deviceFeatures);
    return physicalDevice.createDevice(deviceCreateInfo);
}

// first is queues, second is queueIndices, where first is present and second is render
std::pair<std::pair<vk::Queue, vk::Queue>, std::pair<uint32_t, uint32_t>> Engine::GetVKQueues(
    vk::Device _device, 
    const std::pair<vk::DeviceQueueCreateInfo, vk::DeviceQueueCreateInfo>& queueInfos
) 
{
    assert(_device != nullptr);

    uint32_t graphicsQueueFamilyIndex = queueInfos.first.queueFamilyIndex;
    uint32_t presentQueueFamilyIndex = queueInfos.second.queueFamilyIndex;

    vk::Queue graphicsQueue = _device.getQueue(graphicsQueueFamilyIndex, 0);
    vk::Queue presentQueue = _device.getQueue(presentQueueFamilyIndex, 0);

    return std::make_pair(
        std::make_pair(presentQueue, graphicsQueue), 
        std::make_pair<uint32_t, uint32_t>(static_cast<uint32_t>(presentQueueFamilyIndex), static_cast<uint32_t>(graphicsQueueFamilyIndex))
    );
}

vk::SurfaceKHR Engine::GetSurfaceKHR(App* _app)
{
    assert(_app != nullptr);

    VkSurfaceKHR cSurface;
    if(glfwCreateWindowSurface(static_cast<VkInstance>(instance.getRes()), _app->window, nullptr, &cSurface) != 0)
    {
        Logger::Exception("Failed to create a vk::SurfaceKHR. Make sure that you picked the right platform in the \"Platform\" file");
    }

    vk::SurfaceKHR surface(cSurface);

    return surface;
}