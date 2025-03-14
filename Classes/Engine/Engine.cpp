#include "Engine.hpp"

#include "Constants.hpp"

using namespace nihil::graphics;
using namespace nihil;

Engine::Engine(App* _app, EngineArgs& args)
{
	Logger::Log("Creating the Engine (Vroom vroom)");

	assert(_app != nullptr);

	app = _app;

	app->access();

	app->addEventListener(this, Listeners::onResize);

	Platform platform = getPlatform();

	instance.assignRes(CreateVKInstance(args, platform));
	physicalDevice = PickPhysicalDevice(instance.getRes(), args.dPrefs);

	surface.assignRes(GetSurfaceKHR(_app), instance.getRes());

	app->endAccess();

	vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();

	Logger::Log(std::string("Continuing with the Device: ") + std::string(properties.deviceName));

	auto queueInfo = CreateVKQueueInfo(instance.getRes(), physicalDevice, surface.getRes());

	std::vector<const char*> requiredExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
	device.assignRes(CreateVKLogicalDevice(physicalDevice, queueInfo, requiredExtensions));

	auto queues = GetVKQueues(device.getRes(), queueInfo);

	presentQueue = queues.first.first;
	renderQueue = queues.first.second;

	presentQueueIndex = queues.second.first;
	renderQueueIndex = queues.second.second;
}

Engine::~Engine() 
{
    device.destroy();
	surface.destroy();
    instance.destroy();
};

void Engine::createRenderer()
{
	renderer = new Renderer(this);

	Logger::Log("Renderer Created!");
}