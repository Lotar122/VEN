#include "Engine.hpp"

using namespace nihil::graphics;

void Engine::CreateVulkanInstance(VulkanInstanceCreateInfo createInfo) {
	uint32_t version{ 0 };
	version &= ~(0xFFFU);
	version = VK_MAKE_API_VERSION(0, 1, 0, 0);
	vkEnumerateInstanceVersion(&version);
	vk::ApplicationInfo appInfo = vk::ApplicationInfo(
		createInfo.appName.c_str(),
		VK_MAKE_API_VERSION(createInfo.appVersion.variant, createInfo.appVersion.major, createInfo.appVersion.minor, createInfo.appVersion.patch),
		"Nihil",
		version,
		VK_MAKE_API_VERSION(createInfo.vulkanVersion.variant, createInfo.vulkanVersion.major, createInfo.vulkanVersion.minor, createInfo.vulkanVersion.patch)
	);
	uint32_t extensionCount;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionCount);

	// Create a vector of const char* from glfwExtensions
	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + extensionCount);

	std::vector<const char*> validationLayers;

	if (createInfo.validationLayers) {
		validationLayers = {
			"VK_LAYER_KHRONOS_validation"
		};
	}
	else {
		validationLayers = {};
	}

	vk::InstanceCreateInfo VKCreateInfo = vk::InstanceCreateInfo(
		vk::InstanceCreateFlags(),
		&appInfo,
		static_cast<uint32_t>(validationLayers.size()), validationLayers.data(),
		static_cast<uint32_t>(extensions.size()), extensions.data()
	);

	try {
		instance = vk::createInstance(VKCreateInfo);
	}
	catch (vk::SystemError err) {
		std::abort();
	}
}

void Engine::PickPhysicalDevice()
{
	bool foundDevice = false;
	std::vector<vk::PhysicalDevice> availableDevices = instance.enumeratePhysicalDevices();
	for (const vk::PhysicalDevice& devices : availableDevices)
	{
		if (isSuitable(devices))
		{
			device = devices;
			foundDevice = true;
		}
	}
	if (!foundDevice) {
		std::abort();
	}
}

void Engine::CreateVulkanSurface(GLFWwindow* window)
{
	VkSurfaceKHR c_surface;
	glfwCreateWindowSurface(instance, window, NULL, &c_surface);
	surface = c_surface;
}

void Engine::CreateVulkanQueues()
{
	std::vector<vk::QueueFamilyProperties> queueFamilies = device.getQueueFamilyProperties();
	int iterator1 = 0;
	for (const vk::QueueFamilyProperties& queueFamily : queueFamilies)
	{
		/*
		* Graphics_BIT : 0x00000001;
		* Compute_BIT: 0x00000002;
		* Transfer_BIT: 0x00000004;
		* Sparse_Binding_BIT: 0x00000008;
		*/
		if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
		{
			famind.graphics = iterator1;
			famind.present = iterator1;
		}
		if (device.getSurfaceSupportKHR(iterator1, surface))
			if (famind.isComplete())
			{
				break;
			}
		iterator1++;
	}
	uniqueFAMIND.push_back(famind.graphics.value());
	if (famind.graphics.value() != famind.present.value())
	{
		uniqueFAMIND.push_back(famind.present.value());
	}

	//finalize the queue creation in CreateVulkanLogicalDevice
}

void Engine::CreateVulkanLogicalDevice()
{
	float queuepriority = 1.0f;
	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfo;
	for (uint32_t index : uniqueFAMIND)
	{
		queueCreateInfo.push_back(vk::DeviceQueueCreateInfo(
			vk::DeviceQueueCreateFlags(), famind.graphics.value(), 1,
			&queuepriority
		));
	}

	std::vector<const char*> deviceExtension = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	vk::PhysicalDeviceFeatures deviceFeatures = vk::PhysicalDeviceFeatures();
	deviceFeatures.samplerAnisotropy = VK_TRUE;
	vk::DeviceCreateInfo deviceInfo = vk::DeviceCreateInfo(
		vk::DeviceCreateFlags(), queueCreateInfo.size(), queueCreateInfo.data(), 0, NULL, deviceExtension.size(), deviceExtension.data(), &deviceFeatures
	);

	try {
		logicalDevice = device.createDevice(deviceInfo);
	}
	catch (vk::SystemError err) {
		std::abort();
	}

	graphicsQueue = logicalDevice.getQueue(famind.graphics.value(), 0);
	presentQueue = logicalDevice.getQueue(famind.present.value(), 0);
}