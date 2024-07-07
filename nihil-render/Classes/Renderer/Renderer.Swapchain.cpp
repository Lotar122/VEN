#include "Renderer.hpp"
#include "Classes/Engine/Engine.hpp"

namespace nihil::graphics {
	SwapchainConfiguration Renderer::CreateSwapchainConfiguration(SwapchainConfigCreateInfo createInfo)
	{
		SwapchainSupportDetails support;
		vk::SurfaceFormatKHR surfaceFormat;
		vk::PresentModeKHR presentMode;
		vk::Extent2D extent;

		support.capabilities = engine->device.getSurfaceCapabilitiesKHR(engine->surface);
		support.formats = engine->device.getSurfaceFormatsKHR(engine->surface);
		support.presentModes = engine->device.getSurfacePresentModesKHR(engine->surface);

		bool set1 = false;
		for (const vk::SurfaceFormatKHR& format : support.formats)
		{
			if (format == vk::Format::eB8G8R8A8Unorm && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
				surfaceFormat = format;
				set1 = true;
				break;
			}
		}
		if (!set1) { if (support.formats.size() > 0) { surfaceFormat = support.formats[0]; } else { std::abort(); } }

		bool set2 = false;
		for (const vk::PresentModeKHR& pForm : support.presentModes)
		{
			if (pForm == vk::PresentModeKHR::eMailbox)
			{
				presentMode = pForm;
				set2 = true;
				break;
			}
		}
		if (!set2) presentMode = vk::PresentModeKHR::eFifo;

		//!!! make this customizable
		presentMode = vk::PresentModeKHR::eFifo;

		extent.setWidth(createInfo.windowWidth);
		extent.setHeight(createInfo.windowHeight);
		bool set3 = false;
		if (support.capabilities.currentExtent.width != UINT32_MAX) {
			extent = support.capabilities.currentExtent;
			set3 = true;
		}
		else {
			extent.width = std::min(
				(int)support.capabilities.maxImageExtent.width,
				std::max((int)support.capabilities.minImageExtent.width, (int)createInfo.windowWidth)
			);
			extent.height = std::min(
				(int)support.capabilities.maxImageExtent.height,
				std::max((int)support.capabilities.minImageExtent.height, (int)createInfo.windowHeight)
			);
		}

		uint8_t imageCount = std::min(
			support.capabilities.maxImageCount,
			support.capabilities.minImageCount + (int)createInfo.preferredBuffering
		);
		if (imageCount >= 3) {
			imageCount = 3;
		}

		vk::SwapchainCreateInfoKHR swapchainCreateInfo = vk::SwapchainCreateInfoKHR(
			vk::SwapchainCreateFlagsKHR(), engine->surface, imageCount, surfaceFormat.format, surfaceFormat.colorSpace,
			extent, 1, vk::ImageUsageFlagBits::eColorAttachment
		);

		if (famind.graphics.value() != famind.present.value())
		{
			swapchainCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
			swapchainCreateInfo.queueFamilyIndexCount = uniqueFAMIND.size();
			swapchainCreateInfo.pQueueFamilyIndices = uniqueFAMIND.data();
		}
		else
		{
			swapchainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
		}

		swapchainCreateInfo.preTransform = support.capabilities.currentTransform;
		swapchainCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
		swapchainCreateInfo.presentMode = presentMode;
		swapchainCreateInfo.clipped = VK_TRUE;
		swapchainCreateInfo.oldSwapchain = vk::SwapchainKHR(nullptr);

		return swapchainCreateInfo;
	}

	void Renderer::CreateSwapchain(SwapchainConfiguration createInfo)
	{
		swapchainConfiguration = createInfo;
		try {
			swapchainBundle.swapchain = engine->logicalDevice.createSwapchainKHR(createInfo);
		}
		catch (vk::SystemError err) {
			std::abort();
		}

		swapchainBundle.format = createInfo.imageFormat;
		swapchainBundle.extent = createInfo.imageExtent;
	}

	void Renderer::RecreateSwapchain()
	{
		int* pWidth = new int(0);
		int* pHeight = new int(0);
		glfwGetFramebufferSize(const_cast<GLFWwindow*>(engine->app->get->window), pWidth, pHeight);
		engine->app->SetWidth(*pWidth);
		engine->app->SetHeight(*pHeight);
		while (engine->app->get->width == 0 || engine->app->get->height == 0)
		{
			glfwGetFramebufferSize(const_cast<GLFWwindow*>(engine->app->get->window), pWidth, pHeight);
			glfwWaitEvents();
		}
		engine->app->SetWidth(static_cast<uint16_t>(*pWidth));
		engine->app->SetHeight(static_cast<uint16_t>(*pHeight));

		delete pWidth;
		delete pHeight;
		engine->logicalDevice.waitIdle();
		destroySwapchain();

		objobject->Load("./resources/models/car.obj", nstd::LoadBinObj::DontCare, engine->app->screenRatio);

		SwapchainConfigCreateInfo configInfo = {};
		configInfo.preferredBuffering = BufferingMode::eTriple;
		configInfo.windowHeight = *engine->app->get->height;
		configInfo.windowWidth = *engine->app->get->width;
		swapchainConfiguration = CreateSwapchainConfiguration(configInfo);
		CreateSwapchain(swapchainConfiguration);
		CreateImageViews();
		createDepthBuffers();
		createFrameBuffers();
		createSyncObjects();
		createFrameCommandBuffers();
	}

	void Renderer::createCommandPools()
	{
		//create a command pool
		vk::CommandPoolCreateInfo commandPoolInfo = {};
		commandPoolInfo.flags = vk::CommandPoolCreateFlags() | vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
		commandPoolInfo.queueFamilyIndex = famind.graphics.value();

		try {
			commandPool = engine->logicalDevice.createCommandPool(commandPoolInfo);
		}
		catch (vk::SystemError err) {
			std::cerr << "Error whilst creating the command pool" << std::endl;
			std::abort();
		}
	}

	void Renderer::createSyncObjects()
	{
		//create semaphores
		vk::SemaphoreCreateInfo semaphoreCreateInfo = {};
		semaphoreCreateInfo.flags = vk::SemaphoreCreateFlags();

		//do it for each frame
		for (SwapChainFrame& frame : swapchainBundle.frames)
		{
			try {
				frame.imageAvailable = engine->logicalDevice.createSemaphore(semaphoreCreateInfo);
			}
			catch (vk::SystemError err) {
				std::cerr << "Error whilst creating a semaphore" << std::endl;
				std::abort();
			}

			try {
				frame.renderFinished = engine->logicalDevice.createSemaphore(semaphoreCreateInfo);
			}
			catch (vk::SystemError err) {
				std::cerr << "Error whilst creating a semaphore" << std::endl;
				std::abort();
			}

			//create a fence
			vk::FenceCreateInfo fenceCreateInfo = {};
			fenceCreateInfo.flags = vk::FenceCreateFlags() | vk::FenceCreateFlagBits::eSignaled;

			try {
				frame.inFlightFence = engine->logicalDevice.createFence(fenceCreateInfo);
			}
			catch (vk::SystemError err) {
				std::cerr << "Error whilst creating a fence" << std::endl;
				std::abort();
			}
		}
	}

	void Renderer::createFrameCommandBuffers()
	{
		//make command buffers
		vk::CommandBufferAllocateInfo commandBufferAllocInfo = {};
		commandBufferAllocInfo.commandPool = commandPool;
		commandBufferAllocInfo.level = vk::CommandBufferLevel::ePrimary;
		commandBufferAllocInfo.commandBufferCount = 1;

		for (int i = 0; i < swapchainBundle.frames.size(); i++)
		{
			try {
				swapchainBundle.frames[i].commandBuffer = engine->logicalDevice.allocateCommandBuffers(commandBufferAllocInfo)[0];
			}
			catch (vk::SystemError err) {
				std::cerr << "Error whilst allocating command buffers" << std::endl;
				std::abort();
			}
		}
	}

	void Renderer::createMainCommandBuffer()
	{
		//make command buffers
		vk::CommandBufferAllocateInfo commandBufferAllocInfo = {};
		commandBufferAllocInfo.commandPool = commandPool;
		commandBufferAllocInfo.level = vk::CommandBufferLevel::ePrimary;
		commandBufferAllocInfo.commandBufferCount = 1;

		try {
			commandBuffer = engine->logicalDevice.allocateCommandBuffers(commandBufferAllocInfo)[0];
		}
		catch (vk::SystemError err) {
			std::cerr << "Error whilst allocating the main command buffer" << std::endl;
			std::abort();
		}
	}

	void Renderer::createFrameBuffers()
	{
		//create frame buffer
		for (int i = 0; i < swapchainBundle.frames.size(); i++)
		{
			std::vector<vk::ImageView> attachments = {
				swapchainBundle.frames[i].view,
				swapchainBundle.frames[i].depthBufferView
			};
			vk::FramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.flags = vk::FramebufferCreateFlags();
			framebufferInfo.renderPass = renderPass;
			framebufferInfo.attachmentCount = attachments.size();
			framebufferInfo.width = swapchainBundle.extent.width;
			framebufferInfo.height = swapchainBundle.extent.height;
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.layers = 1;

			try {
				swapchainBundle.frames[i].frameBuffer = engine->logicalDevice.createFramebuffer(framebufferInfo);
			}
			catch (vk::SystemError) {
				std::cerr << "Error whilst creating a framebuffer" << std::endl;
				std::abort();
			}
		}
	}

	void Renderer::createDepthBuffers()
	{
		for (SwapChainFrame& frame : swapchainBundle.frames)
		{
			frame.depthFormat = swapchainBundle.depthFormat;

			// Create depth image
			vk::ImageCreateInfo depthImageCreateInfo(
				vk::ImageCreateFlags(),                      // flags
				vk::ImageType::e2D,                          // imageType
				frame.depthFormat,                                 // format
				vk::Extent3D(swapchainBundle.extent.width, swapchainBundle.extent.height, 1),  // extent
				1,                                           // mipLevels
				1,                                           // arrayLayers
				vk::SampleCountFlagBits::e1,                 // samples
				vk::ImageTiling::eOptimal,                   // tiling
				vk::ImageUsageFlagBits::eDepthStencilAttachment,  // usage
				vk::SharingMode::eExclusive,                 // sharingMode
				0,                                           // queueFamilyIndexCount
				nullptr,                                     // pQueueFamilyIndices
				vk::ImageLayout::eUndefined                  // initialLayout
			);

			frame.depthBuffer = engine->logicalDevice.createImage(depthImageCreateInfo);

			// Allocate memory for the depth image
			vk::MemoryRequirements memRequirements = engine->logicalDevice.getImageMemoryRequirements(frame.depthBuffer);
			vk::MemoryAllocateInfo allocInfo(memRequirements.size, findMemoryType(memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal));
			frame.depthBufferMemory = engine->logicalDevice.allocateMemory(allocInfo);
			engine->logicalDevice.bindImageMemory(frame.depthBuffer, frame.depthBufferMemory, 0);

			// Create image view for depth image
			vk::ImageViewCreateInfo depthImageViewCreateInfo(
				vk::ImageViewCreateFlags(),       // flags
				frame.depthBuffer,                 // image
				vk::ImageViewType::e2D,           // viewType
				frame.depthFormat,                       // format
				vk::ComponentMapping(),            // components
				vk::ImageSubresourceRange(
					vk::ImageAspectFlagBits::eDepth,  // aspectMask
					0,                                 // baseMipLevel
					1,                                 // levelCount
					0,                                 // baseArrayLayer
					1                                  // layerCount
				)
			);

			frame.depthBufferView = engine->logicalDevice.createImageView(depthImageViewCreateInfo);
		}
	}

	// Helper function to find memory type
	uint32_t Renderer::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
		vk::PhysicalDeviceMemoryProperties memProperties = engine->device.getMemoryProperties();

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		throw std::exception("Failed to find suitable memory type!");
	}

	void Renderer::CreateImageViews()
	{
		std::vector <vk::Image> images = engine->logicalDevice.getSwapchainImagesKHR(swapchainBundle.swapchain);
		swapchainBundle.frames.resize(images.size());
		for (size_t i = 0; i < images.size(); i++)
		{
			vk::ImageViewCreateInfo imageViewCreateInfo = {};
			imageViewCreateInfo.image = images[i];
			imageViewCreateInfo.viewType = vk::ImageViewType::e2D;
			imageViewCreateInfo.components.r = vk::ComponentSwizzle::eIdentity;
			imageViewCreateInfo.components.g = vk::ComponentSwizzle::eIdentity;
			imageViewCreateInfo.components.b = vk::ComponentSwizzle::eIdentity;
			imageViewCreateInfo.components.a = vk::ComponentSwizzle::eIdentity;
			imageViewCreateInfo.format = vk::Format::eB8G8R8A8Unorm;

			imageViewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
			imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
			imageViewCreateInfo.subresourceRange.levelCount = 1;
			imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
			imageViewCreateInfo.subresourceRange.layerCount = 1;

			swapchainBundle.frames[i].image = images[i];
			swapchainBundle.frames[i].view = engine->logicalDevice.createImageView(imageViewCreateInfo);
		}
		maxFramesInFlight = swapchainBundle.frames.size();
		frameNumber = 0;
	}
}