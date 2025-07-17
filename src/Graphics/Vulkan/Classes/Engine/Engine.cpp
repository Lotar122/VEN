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
	physicalDevice = PickPhysicalDevice(instance, args.dPrefs);

	surface.assignRes(GetSurfaceKHR(_app, instance), instance);

	app->endAccess();

	vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();

	auto queueInfo = CreateVKQueueInfo(instance, physicalDevice, surface);

	std::vector<const char*> requiredExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
	device.assignRes(CreateVKLogicalDevice(physicalDevice, queueInfo, requiredExtensions));

	auto queues = GetVKQueues(device, queueInfo);

	presentQueue = std::get<0>(queues.first);
	renderQueue = std::get<1>(queues.first);
	transferQueue = std::get<2>(queues.first);

	presentQueueIndex = std::get<0>(queues.second);
	renderQueueIndex = std::get<1>(queues.second);
	transferQueueIndex = std::get<2>(queues.second);

	mainCommandPool.assignRes(CreateVKCommandPool(device, renderQueueIndex), device);
	mainCommandBuffer.assignRes(CreateVKCommandBuffer(device, mainCommandPool), device, mainCommandPool);

	vk::FenceCreateInfo transferFenceCreateInfo = {};
	transferFenceCreateInfo.sType = vk::StructureType::eFenceCreateInfo;
	transferFenceCreateInfo.pNext = nullptr;
	transferFenceCreateInfo.flags = vk::FenceCreateFlagBits::eSignaled;
	//transferFenceCreateInfo.flags = {};

	transferFence.assignRes(device.getRes().createFence(transferFenceCreateInfo), device);
}

Engine::~Engine() 
{
	transferFence.destroy();

	mainCommandBuffer.destroy();
	mainCommandPool.destroy();

    device.destroy();
	surface.destroy();
    instance.destroy();
};

void Engine::createRenderer()
{
	renderer = new Renderer(this);

	Logger::Log("Renderer Created!");
}

vk::CommandPool Engine::CreateVKCommandPool(vk::Device _device, uint32_t _renderQueueIndex)
{
	vk::CommandPoolCreateInfo poolInfo = {
	vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
	_renderQueueIndex
	};

	return _device.createCommandPool(poolInfo);
}

vk::CommandBuffer Engine::CreateVKCommandBuffer(vk::Device _device, vk::CommandPool commandPool)
{
	vk::CommandBufferAllocateInfo allocInfo = {
		commandPool,
		vk::CommandBufferLevel::ePrimary,
		1
	};

	return _device.allocateCommandBuffers(allocInfo)[0];
}