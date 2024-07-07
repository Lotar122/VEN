#include "Engine.hpp"
#include "Classes/Buffer/Buffer.hpp"

#include <filesystem>

#ifdef _DEBUG
#define DEBUGFLAG true
#else
#define DEBUGFLAG false
#endif

using namespace nihil::graphics;

void Engine::Draw(std::vector<nstd::Component>& modelArr) {
	renderer->Draw(modelArr);
	commandDataManager.deleteAll();
}

Engine::Engine(bool _debug)
{
	debug = _debug;

	std::cout << YELLOW << "[Setup]" << RESET << "Constructor called, preparing the engine: " << std::endl;

	get = new Proxy(
		&instance,
		&device,
		&logicalDevice
	);
}
Engine::Engine()
{
	debug = false;

	std::cout << YELLOW << "[Setup]" << RESET << "Constructor called, preparing the engine: " << std::endl;

	get = new Proxy(
		&instance,
		&device,
		&logicalDevice
	);
}
Engine::~Engine()
{
	delete renderer;
	logicalDevice.waitIdle();

	//destroy all resources that were registered
	{
		for (vk::Pipeline& x : pipelineStorage)
		{
			logicalDevice.destroyPipeline(x);
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
	}

	logicalDevice.waitIdle();
	logicalDevice.destroy();
	instance.destroySurfaceKHR(surface);
	instance.destroy();

	//REMEMBER
	//undestroyed resources, unfinished setups, ETC.
	//cause nvoglv64.dll crashes
}

void Engine::Setup()
{
	if (app == NULL) { std::cerr << "App is nullptr" << std::endl; std::abort(); }
	//SetupDeafult();

	VulkanInstanceCreateInfo instanceInfo = {};
	instanceInfo.appName = *app->get->name;
	instanceInfo.appVersion = *app->get->appVersion;
	instanceInfo.vulkanVersion = *app->get->vulkanVersion;
	//deafult false
	instanceInfo.validationLayers = true;

	//Creation of the vulkan instance
	CreateVulkanInstance(instanceInfo);

	CreateVulkanSurface(app->get->window);

	PickPhysicalDevice();

	CreateVulkanQueues();

	CreateVulkanLogicalDevice();

	renderer = new Renderer(this);
}

void Engine::setApp(App* _app)
{
	app = _app;
}