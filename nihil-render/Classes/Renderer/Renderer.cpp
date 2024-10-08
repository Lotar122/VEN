#include "Renderer.hpp"

#include "Classes/Engine/Engine.hpp"
#include "Classes/Engine/Engine.Structs.InData.hpp"

#include "Classes/Buffer/Buffer.hpp"

namespace nihil::graphics {

	Renderer::Renderer(Engine* _engine) 
	{
		engine = _engine;

		famind = engine->famind;
		graphicsQueue = engine->graphicsQueue;
		presentQueue = engine->presentQueue;
		uniqueFAMIND = engine->uniqueFAMIND;
		copyLogicalDevice = engine->logicalDevice;

		SwapchainConfigCreateInfo swapchainConfigCreateInfo = {};
		swapchainConfigCreateInfo.preferredBuffering = BufferingMode::eTriple;
		std::cout << engine->app->get->width << std::endl;
		swapchainConfigCreateInfo.windowWidth = *engine->app->get->width;
		swapchainConfigCreateInfo.windowHeight = *engine->app->get->height;

		SwapchainConfiguration swapchainConfig = CreateSwapchainConfiguration(swapchainConfigCreateInfo);

		CreateSwapchain(swapchainConfig);

		engine->swapchain = &swapchainBundle;

		CreateImageViews();

		swapchainBundle.depthFormat = vk::Format::eD32Sfloat;
		createDepthBuffers();

		engine->finishPrimarySetup();

		//customize shaders in future with the component system
		//PipelineSetup();

		createCommandPools();
		createFrameCommandBuffers();
		createMainCommandBuffer();
		createDepthBuffers();
		createFrameBuffers();
		createSyncObjects();

		swapchainBundle.depthFormat = vk::Format::eD32Sfloat;

		clearScreenPass = createClearScreenRenderPass(engine->logicalDevice, swapchainBundle.format, swapchainBundle.depthFormat);

		//loadingThread.join();

		engine->finishSetup();
	}
	Renderer::~Renderer() 
	{
		copyLogicalDevice.waitIdle();
		copyLogicalDevice.destroyCommandPool(commandPool);

		//destroySwapchain();

		//REMEMBER
		//undestroyed resources, unfinished setups, ETC.
		//cause nvoglv64.dll crashes
	}

	void Renderer::destroySwapchain()
	{
		std::cout << std::endl << YELLOW << "[Setup]" << RESET << " Destroying the swapchain " << GREEN << "[###]" << std::endl;
		copyLogicalDevice.waitIdle();
		for (const SwapChainFrame& frame : swapchainBundle.frames)
		{
			copyLogicalDevice.destroySemaphore(frame.imageAvailable);
			copyLogicalDevice.destroySemaphore(frame.renderFinished);
			copyLogicalDevice.destroyFence(frame.inFlightFence);
			copyLogicalDevice.destroyImageView(frame.view);
			copyLogicalDevice.destroyFramebuffer(frame.frameBuffer);
			copyLogicalDevice.destroyImage(frame.depthBuffer);
			copyLogicalDevice.destroyImageView(frame.depthBufferView);
			copyLogicalDevice.freeMemory(frame.depthBufferMemory);
		}
		copyLogicalDevice.destroySwapchainKHR(swapchainBundle.swapchain);
	}
}