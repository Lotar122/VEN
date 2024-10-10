#include "Engine.hpp"
#include "Classes/Buffer/Buffer.hpp"

#include <filesystem>
#include <iostream>

#ifdef _DEBUG
#define DEBUGFLAG true
#else
#define DEBUGFLAG false
#endif

using namespace nihil::graphics;

void Engine::Draw(Camera& camera) {
	renderer->Draw(camera);
	commandDataManager.reset();
}

Engine::Engine(bool _debug, nstd::MemoryArena* _globalArena)
{
	debug = _debug;
	globalArena = _globalArena;

	std::cout << YELLOW << "[Setup]" << RESET << "Constructor called, preparing the engine: " << std::endl;

	get = new Proxy(
		&instance,
		&device,
		&logicalDevice
	);
}
Engine::Engine(nstd::MemoryArena* _globalArena)
{
	debug = false;
	globalArena = _globalArena;

	std::cout << YELLOW << "[Setup]" << RESET << "Constructor called, preparing the engine: " << std::endl;

	get = new Proxy(
		&instance,
		&device,
		&logicalDevice
	);
}

void Engine::CreateRenderPass(SwapChainBundle* swapchainBundle)
{
	if (this->debug) std::cout << YELLOW << "[Setup]" << MAGENTA << "->" << YELLOW << "[Pipeline]" << RESET << "Creating a renderpass ";

	std::vector<vk::AttachmentDescription> renderPassAttachments;

	//Create render pass (move to a seperate function in future)
	//in class declaration: vk::RenderPass renderPass;
	vk::AttachmentDescription colorAttachment = {};
	colorAttachment.flags = vk::AttachmentDescriptionFlags();
	colorAttachment.format = swapchainBundle->format;
	colorAttachment.samples = vk::SampleCountFlagBits::e1;
	colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
	colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

	renderPassAttachments.push_back(colorAttachment);

	swapchainBundle->depthFormat = vk::Format::eD32Sfloat;

	vk::AttachmentDescription depthAttachment = {};
	depthAttachment.format = swapchainBundle->depthFormat;
	depthAttachment.samples = vk::SampleCountFlagBits::e1;
	depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	depthAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	depthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	depthAttachment.initialLayout = vk::ImageLayout::eUndefined;
	depthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

	renderPassAttachments.push_back(depthAttachment);

	vk::AttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

	vk::AttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

	vk::SubpassDescription subpass = {};
	subpass.flags = vk::SubpassDescriptionFlags();
	subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	vk::SubpassDependency dependency(
		VK_SUBPASS_EXTERNAL, 0,
		vk::PipelineStageFlagBits::eEarlyFragmentTests,
		vk::PipelineStageFlagBits::eLateFragmentTests,
		vk::AccessFlagBits::eDepthStencilAttachmentWrite,
		vk::AccessFlagBits::eDepthStencilAttachmentRead
	);

	vk::RenderPassCreateInfo renderpassInfo = {};
	renderpassInfo.flags = vk::RenderPassCreateFlags();
	renderpassInfo.attachmentCount = renderPassAttachments.size();
	renderpassInfo.pAttachments = renderPassAttachments.data();
	renderpassInfo.subpassCount = 1;
	renderpassInfo.pSubpasses = &subpass;
	renderpassInfo.dependencyCount = 1;
	renderpassInfo.pDependencies = &dependency;

	try {
		renderPass = this->logicalDevice.createRenderPass(renderpassInfo);
	}
	catch (vk::SystemError err) {
		std::cerr << RED << "[###]" << RESET << std::endl;
		this->error = true;
		this->finishPipelineSetup();
		throw std::exception(err.what());
	}
	std::cout << GREEN << "[###]" << RESET << std::endl;
}

Engine::~Engine()
{
	renderer->destroySwapchain();
	logicalDevice.waitIdle();

	//destroy all resources that were registered
	{
		for (PipelineBundle& x : pipelineStorage)
		{
			logicalDevice.destroyPipeline(x.pipeline);
		}
		for (BufferBase*& x : bufferStorage)
		{
			delete x;
		}
		for (vk::Image& x : imageStorage)
		{
			logicalDevice.destroyImage(x);
		}
		for (vk::ImageView& x : imageViewStorage)
		{
			logicalDevice.destroyImageView(x);
		}
		for (vk::DeviceMemory& x : memoryStorage)
		{
			logicalDevice.unmapMemory(x);
		}
		for (vk::ShaderModule& x : shaderModuleStorage)
		{
			logicalDevice.destroyShaderModule(x);
		}
		for (vk::PipelineLayout& x : pipelineLayoutStorage)
		{
			logicalDevice.destroyPipelineLayout(x);
		}
		for (vk::RenderPass& x : renderPassStorage)
		{
			logicalDevice.destroyRenderPass(x);
		}

		delete get;
		renderer->~Renderer();
	}

	shaderManager.reset();

	std::cout << "all resources deleted" << std::endl;

	logicalDevice.waitIdle();
	logicalDevice.destroy();
	instance.destroySurfaceKHR(surface);
	instance.destroy();

	//REMEMBER
	//undestroyed resources, unfinished setups, ETC.
	//cause nvoglv64.dll crashes

	shaderManager.free();
	commandDataManager.free();
}

void Engine::Setup(bool validation)
{
	if (app == NULL) { std::cerr << "App is nullptr" << std::endl; throw std::exception("App is nullptr"); }
	//SetupDeafult();

	VulkanInstanceCreateInfo instanceInfo = {};
	instanceInfo.appName = *app->get->name;
	instanceInfo.appVersion = *app->get->appVersion;
	instanceInfo.vulkanVersion = *app->get->vulkanVersion;
	//deafult false
	instanceInfo.validationLayers = validation;

	//Creation of the vulkan instance
	CreateVulkanInstance(instanceInfo);

	CreateVulkanSurface(app->get->window);

	PickPhysicalDevice();

	CreateVulkanQueues();

	CreateVulkanLogicalDevice();

	renderer = new (globalArena->allocate<Renderer>()) Renderer(this);

	swapchain = &renderer->swapchainBundle;
}

void Engine::setApp(App* _app)
{
	app = _app;
}