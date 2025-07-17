#include "Swapchain.hpp"

#include "Classes/RenderPass/RenderPass.hpp"
#include "utils.hpp"

using namespace nihil::graphics;

Swapchain::Swapchain(App* _app, vk::PresentModeKHR _presentMode, uint8_t _prefferedImageCount, uint64_t _sampleCount, Engine* _engine)
{
	assert(_app != nullptr);
	assert(_engine != nullptr);

	app = _app;
	app->access();
	app->addEventListener(this, Listeners::onResize);

	engine = _engine;
	presentMode = _presentMode;
	prefferedImageCount = _prefferedImageCount;

	sampleCount = _sampleCount;

	extent.width = app->width;
	extent.height = app->height;

	app->endAccess();

	#pragma region SURFACE_FORMAT
	std::vector<vk::SurfaceFormatKHR> surfaceFormats = engine->_physicalDevice().getSurfaceFormatsKHR(engine->_surfaceKHR());

	bool foundFormat = false;

	if (surfaceFormats.size() == 1 && surfaceFormats[0].format == vk::Format::eUndefined) {
		surfaceFormat = { vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear };
	}

	// Preferred formats and color spaces
	for (const auto& format : surfaceFormats) {
		if (format.format == vk::Format::eB8G8R8A8Unorm && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
			surfaceFormat = format; // Return preferred format if found
			foundFormat = true;
			Logger::Log("Found the preffered format.");
		}
	}

	// Fallback to the first available format
	if(!foundFormat) surfaceFormat = surfaceFormats[0];

	if(!foundFormat) Logger::Warn("Couldn't find the preffered format, falling back to any available format.");

	imageFormat = surfaceFormat.format;
	depthFormat = vk::Format::eD32Sfloat;

	#pragma endregion
}

Swapchain::Frame* Swapchain::acquireNextFrame(uint32_t* _imageIndex, uint32_t* _frameIndex)
{
	vk::Result discardResult = engine->_device().waitForFences(1, &frames[frameIndex].inFlightFence, VK_TRUE, UINT64_MAX);
	discardResult = engine->_device().resetFences(1, &frames[frameIndex].inFlightFence);

	uint32_t imageIndex = 0;

	vk::Result result = engine->_device().acquireNextImageKHR(swapchain.getRes(), UINT64_MAX, frames[frameIndex].imageAvailable, nullptr, &imageIndex);

	if (frameIndex != imageIndex)
	{
		//discardResult = engine->_device().waitForFences(1, &frames[imageIndex].inFlightFence, VK_TRUE, UINT64_MAX);
		//discardResult = engine->_device().resetFences(1, &frames[imageIndex].inFlightFence);
	}
	
	if(result == vk::Result::eSuboptimalKHR || result == vk::Result::eErrorOutOfDateKHR)
	{
		recreate();

		return nullptr;
	}

	else if(result != vk::Result::eSuccess)
	{
		Logger::Exception("Failed to acquire the next image.");
	}

	if(_imageIndex) *_imageIndex = imageIndex;
	if(_frameIndex) *_frameIndex = frameIndex;

	return &frames[imageIndex];
}

void Swapchain::presentFrame(Frame& frame, uint32_t& imageIndex)
{
	vk::PresentInfoKHR presentInfo = {};
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &engine->_swapchain()->_frames()[frameIndex].renderFinished;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapchain.getResP();
	presentInfo.pImageIndices = &imageIndex;

	vk::Result presentResult = engine->_presentQueue().presentKHR(presentInfo);

	// Handle swapchain recreation if necessary
	if (presentResult == vk::Result::eErrorOutOfDateKHR || presentResult == vk::Result::eSuboptimalKHR) {
		recreate();
	}

	frameIndex = (frameIndex + 1) % imageCount;
}

void Swapchain::create(std::pair<uint32_t, uint32_t> _queueFamilyIndices, RenderPass* _basicRenderPass, bool recreation)
{
	assert(_basicRenderPass != nullptr);
	assert(engine->_device() != nullptr);

	assert(
		static_cast<vk::SampleCountFlagBits>(sampleCount) == vk::SampleCountFlagBits::e1 ||
		static_cast<vk::SampleCountFlagBits>(sampleCount) == vk::SampleCountFlagBits::e2 ||
		static_cast<vk::SampleCountFlagBits>(sampleCount) == vk::SampleCountFlagBits::e4 ||
		static_cast<vk::SampleCountFlagBits>(sampleCount) == vk::SampleCountFlagBits::e8 ||
		static_cast<vk::SampleCountFlagBits>(sampleCount) == vk::SampleCountFlagBits::e16 ||
		static_cast<vk::SampleCountFlagBits>(sampleCount) == vk::SampleCountFlagBits::e32 ||
		static_cast<vk::SampleCountFlagBits>(sampleCount) == vk::SampleCountFlagBits::e64
	);

	vk::SampleCountFlagBits typedSampleCount = static_cast<vk::SampleCountFlagBits>(sampleCount);

	app->access();

	extent.width = app->width;
	extent.height = app->height;

	app->endAccess();

	basicRenderPass = _basicRenderPass;
	queueFamilyIndices = _queueFamilyIndices;

	#pragma region IMAGE_COUNT
	vk::SurfaceCapabilitiesKHR surfaceCapabilities = engine->_physicalDevice().getSurfaceCapabilitiesKHR(engine->_surfaceKHR());
	transform = surfaceCapabilities.currentTransform;

	if(surfaceCapabilities.maxImageCount == 0)
	{
		imageCount = std::max((uint8_t)surfaceCapabilities.minImageCount, (uint8_t)prefferedImageCount);
	}
	else
	{
		imageCount = std::min((uint8_t)surfaceCapabilities.maxImageCount, (uint8_t)std::max((uint8_t)surfaceCapabilities.minImageCount, prefferedImageCount));
	}

	#pragma endregion
	
	uint32_t queueFamilyIndicesArr[] = { queueFamilyIndices.second, queueFamilyIndices.first };

	vk::SharingMode sharingMode;

	if(queueFamilyIndicesArr[0] == queueFamilyIndicesArr[1])
	{
		sharingMode = vk::SharingMode::eExclusive;
	}
	else
	{
		sharingMode = vk::SharingMode::eConcurrent;
	}

	extent.width = std::clamp(extent.width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
    extent.height = std::clamp(extent.height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);

	vk::SwapchainCreateInfoKHR swapchainCreateInfo(
		{},                               // Flags
		engine->_surfaceKHR(),                          // Surface
		imageCount,                       // Min image count
		surfaceFormat.format,              // Image format
		surfaceFormat.colorSpace,          // Color space
		vk::Extent2D(extent.width, extent.height),                       // Image extent
		1,                                // Image array layers
		vk::ImageUsageFlagBits::eColorAttachment, // Image usage
		sharingMode,     // Sharing mode
		2,                                // Queue family index count
		queueFamilyIndicesArr,               // Queue family indices
		transform, // Pre-transform
		alpha, // Composite alpha
		presentMode,                // Present mode
		VK_TRUE,                          // Clipped
		swapchain.getRes()                           // Old swapchain
	);

	imageFormat = surfaceFormat.format;
	depthFormat = vk::Format::eD32Sfloat;

	try {
		if(recreation) swapchain.reassign(engine->_device().createSwapchainKHR(swapchainCreateInfo));
		else swapchain.assignRes(engine->_device().createSwapchainKHR(swapchainCreateInfo), engine->_device());
	}
	catch(vk::SystemError& err) {
		Logger::Exception(err.what());
	}

	std::vector<vk::Image> images = engine->_device().getSwapchainImagesKHR(swapchain.getRes());

	imageCount = images.size();
	frames.resize(imageCount);

	for(int i = 0; i < imageCount; i++)
	{
		Frame& frame = frames[i];

		frame.resolved = images[i];

		vk::ImageViewCreateInfo resolvedViewCreateInfo(
			{},                      // flags
			frame.resolved,                   // image
			vk::ImageViewType::e2D,  // type
			imageFormat,    // format
			{},                      // component mapping (default identity)
			{                        // subresource range
				vk::ImageAspectFlagBits::eColor, // aspect mask (color for swapchain)
				0, 1,  // baseMipLevel, levelCount
				0, 1   // baseArrayLayer, layerCount
			}
		);

		// Create image view
		vk::ImageView resolvedView = engine->_device().createImageView(resolvedViewCreateInfo);
		frame.resolvedView = resolvedView;

		vk::ImageCreateInfo multisampledImageCreateInfo = {
			{}, vk::ImageType::e2D, imageFormat,
			{extent.width, extent.height, 1},
			1, 1, typedSampleCount,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment,
			vk::SharingMode::eExclusive,
			{}, vk::ImageLayout::eUndefined
		};

		frame.multisampled = engine->_device().createImage(multisampledImageCreateInfo);
		
		vk::MemoryRequirements multisampledMemRequirements = engine->_device().getImageMemoryRequirements(frame.multisampled);
		vk::MemoryAllocateInfo multisampledMemoryAllocInfo(multisampledMemRequirements.size, findMemoryType(engine->_physicalDevice(), multisampledMemRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal));
		frame.multisampledImageMemory = engine->_device().allocateMemory(multisampledMemoryAllocInfo);
		engine->_device().bindImageMemory(frame.multisampled, frame.multisampledImageMemory, 0);

		vk::ImageViewCreateInfo multisampledViewCreateInfo(
			{},                      // flags
			frame.multisampled,                   // image
			vk::ImageViewType::e2D,  // type
			imageFormat,    // format
			{},                      // component mapping (default identity)
			{                        // subresource range
				vk::ImageAspectFlagBits::eColor, // aspect mask (color for swapchain)
				0, 1,  // baseMipLevel, levelCount
				0, 1   // baseArrayLayer, layerCount
			}
		);

		// Create multisampledImage view
		vk::ImageView multisampledView = engine->_device().createImageView(multisampledViewCreateInfo);
		frame.multisampledView = multisampledView;

		// Rest of the frame setup, (DONE! framebuffer), depthubffer, depthview, depthmemory, (DONE! commandBuffers), (DONE! commandPools)

		//Depth
		vk::ImageCreateInfo depthImageCreateInfo(
			vk::ImageCreateFlags(),                      // flags
			vk::ImageType::e2D,                          // imageType
			depthFormat,                                 // format
			vk::Extent3D(extent.width, extent.height, 1),  // extent
			1,                                           // mipLevels
			1,                                           // arrayLayers
			typedSampleCount,                 // samples
			vk::ImageTiling::eOptimal,                   // tiling
			vk::ImageUsageFlagBits::eDepthStencilAttachment,  // usage
			vk::SharingMode::eExclusive,                 // sharingMode
			0,                                           // queueFamilyIndexCount
			nullptr,                                     // pQueueFamilyIndices
			vk::ImageLayout::eUndefined                  // initialLayout
		);

		frame.depthBuffer = engine->_device().createImage(depthImageCreateInfo);

		vk::MemoryRequirements depthMemRequirements = engine->_device().getImageMemoryRequirements(frame.depthBuffer);
		vk::MemoryAllocateInfo depthMemoryAllocInfo(depthMemRequirements.size, findMemoryType(engine->_physicalDevice(), depthMemRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal));
		frame.depthBufferMemory = engine->_device().allocateMemory(depthMemoryAllocInfo);
		engine->_device().bindImageMemory(frame.depthBuffer, frame.depthBufferMemory, 0);

		vk::ImageViewCreateInfo depthImageViewCreateInfo(
			vk::ImageViewCreateFlags(),       // flags
			frame.depthBuffer,                 // image
			vk::ImageViewType::e2D,           // viewType
			depthFormat,                       // format
			vk::ComponentMapping(),            // components
			vk::ImageSubresourceRange(
				vk::ImageAspectFlagBits::eDepth,  // aspectMask
				0,                                 // baseMipLevel
				1,                                 // levelCount
				0,                                 // baseArrayLayer
				1                                  // layerCount
			)
		);

		frame.depthBufferView = engine->_device().createImageView(depthImageViewCreateInfo);

		//FrameBuffer
		std::array<vk::ImageView, 3> views = {frame.multisampledView, frame.depthBufferView, frame.resolvedView};

		vk::FramebufferCreateInfo framebufferInfo(
			{},                                // No special flags
			basicRenderPass->_renderPass(),                   // ✅ Render pass this framebuffer is compatible with
			3,                                 // ✅ Number of attachments (e.g., one for color and one for depth)
			views.data(),                   // ✅ Attachments (array of vk::ImageView)
			extent.width,             // ✅ Framebuffer width
			extent.height,            // ✅ Framebuffer height
			1                                  // ✅ Number of layers (1 for 2D images)
		);
		
		frame.frameBuffer = engine->_device().createFramebuffer(framebufferInfo);

		vk::CommandPoolCreateInfo poolInfo{};
		poolInfo.queueFamilyIndex = queueFamilyIndices.second;
		poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer; // Allow command buffer resets

		frame.commandPool = engine->_device().createCommandPool(poolInfo);

		vk::CommandBufferAllocateInfo commandBufferAllocateInfo{};
		commandBufferAllocateInfo.commandPool = frame.commandPool;
		commandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
		commandBufferAllocateInfo.commandBufferCount = 1;

		frame.commandBuffer = engine->_device().allocateCommandBuffers(commandBufferAllocateInfo)[0]; //the function returns a vector, but we only allocate one command buffer hence we put "[0]" at the end

		vk::SemaphoreCreateInfo imageAvailableCreateInfo = {};
        frame.imageAvailable = engine->_device().createSemaphore(imageAvailableCreateInfo);

		vk::SemaphoreCreateInfo renderFinishedCreateInfo = {};
        frame.renderFinished = engine->_device().createSemaphore(renderFinishedCreateInfo);

        // Create Fence (starts in signaled state)
        vk::FenceCreateInfo fenceCreateInfo = {};
		fenceCreateInfo.flags = vk::FenceCreateFlags() | vk::FenceCreateFlagBits::eSignaled;
        frame.inFlightFence = engine->_device().createFence(fenceCreateInfo);
	}
}

void Swapchain::recreate()
{
	engine->_device().waitIdle();
	engine->_renderQueue().waitIdle();

	for(Frame& f : frames)
	{
		engine->_device().destroyImageView(f.resolvedView);
		engine->_device().destroyFramebuffer(f.frameBuffer);
		engine->_device().freeCommandBuffers(f.commandPool, f.commandBuffer);
		engine->_device().destroyCommandPool(f.commandPool);
		engine->_device().destroySemaphore(f.imageAvailable);
		engine->_device().destroySemaphore(f.renderFinished);
		engine->_device().destroyFence(f.inFlightFence);

		engine->_device().destroyImageView(f.depthBufferView);
		engine->_device().destroyImage(f.depthBuffer);
		engine->_device().freeMemory(f.depthBufferMemory);

		engine->_device().destroyImageView(f.multisampledView);
		engine->_device().destroyImage(f.multisampled);
		engine->_device().freeMemory(f.multisampledImageMemory);
	}

	//create new swapchain
	create(queueFamilyIndices, basicRenderPass, true);

	frameIndex = 0;
}

void Swapchain::onResize()
{
	recreate();
}

Swapchain::~Swapchain()
{
	for (Frame& f : frames)
	{
		engine->_device().destroyImageView(f.resolvedView);
		engine->_device().destroyFramebuffer(f.frameBuffer);
		engine->_device().freeCommandBuffers(f.commandPool, f.commandBuffer);
		engine->_device().destroyCommandPool(f.commandPool);
		engine->_device().destroySemaphore(f.imageAvailable);
		engine->_device().destroySemaphore(f.renderFinished);
		engine->_device().destroyFence(f.inFlightFence);

		engine->_device().destroyImageView(f.depthBufferView);
		engine->_device().destroyImage(f.depthBuffer);
		engine->_device().freeMemory(f.depthBufferMemory);

		engine->_device().destroyImageView(f.multisampledView);
		engine->_device().destroyImage(f.multisampled);
		engine->_device().freeMemory(f.multisampledImageMemory);
	}
}