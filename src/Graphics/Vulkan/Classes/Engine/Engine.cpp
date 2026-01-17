#include "Engine.hpp"

#include "Constants.hpp"
#include "vulkan/vulkan_handles.hpp"

#include <filesystem>
#include <fstream>

using namespace nihil::graphics;
using namespace nihil;

Engine::Engine(App* _app, EngineArgs& args)
{
	Logger::Log("Creating the Engine (Vroom vroom)");

	assert(_app != nullptr);

	app = _app;
	directory = args.directory;

	if (!std::filesystem::exists(directory))
    {
        std::filesystem::create_directories(directory);
    }

	if (!std::filesystem::exists(directory + "/ShaderCache"))
    {
        std::filesystem::create_directories(directory + "/ShaderCache");
    }

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

	transferFence.assignRes(device.getRes().createFence(transferFenceCreateInfo), device);

	auto pipelineCacheInfo = std::move(createPipelineCache(device, directory));

	pipelineCache.assignRes(pipelineCacheInfo.first, device);

	pipelineCacheData = std::move(pipelineCacheInfo.second);
}

Engine::~Engine() 
{
	pipelineCacheData = std::move(getPipelineCacheData(device, pipelineCache));
	savePipelineCacheData(directory, pipelineCacheData);

	pipelineCache.destroy();

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

std::vector<std::byte> Engine::loadPipelineCacheData(const std::string& path)
{
	std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) [[unlikely]] return std::vector<std::byte>();

    size_t size = static_cast<size_t>(file.tellg());

	std::vector<std::byte> pipelineCacheData;
	pipelineCacheData.resize(size);

	file.seekg(0);
    file.read(reinterpret_cast<char*>(pipelineCacheData.data()), size);

	return pipelineCacheData;
}

std::pair<vk::PipelineCache, std::vector<std::byte>> Engine::createPipelineCache(vk::Device _device, const std::string& directory)
{
	std::vector<std::byte> pipelineCacheData = loadPipelineCacheData(directory + "/pipelineCache.bin");

	vk::PipelineCacheCreateInfo cacheInfo{};
	if (!pipelineCacheData.empty()) [[likely]]
	{
		Logger::Log("Pipeline cache found.");
		cacheInfo.initialDataSize = pipelineCacheData.size();
		cacheInfo.pInitialData = pipelineCacheData.data();
	}

	vk::PipelineCache pipelineCache = _device.createPipelineCache(cacheInfo);

	return std::make_pair(pipelineCache, pipelineCacheData);
}

std::vector<std::byte> Engine::getPipelineCacheData(vk::Device _device, vk::PipelineCache _pipelineCache)
{
	auto pipelineCacheInfo = _device.getPipelineCacheData(_pipelineCache);

	size_t pipelineCacheDataSize = pipelineCacheInfo.size();

	std::vector<std::byte> pipelineCacheData;
	pipelineCacheData.resize(pipelineCacheDataSize);

	std::memcpy(pipelineCacheData.data(), pipelineCacheInfo.data(), pipelineCacheDataSize);

	return pipelineCacheData;
}

void Engine::savePipelineCacheData(const std::string& directory, const std::vector<std::byte>& _pipelineCacheData)
{
	Logger::Log("Saving pipeline cache to: {}/pipelineCache.bin", directory);
	std::ofstream file(directory + "/pipelineCache.bin", std::ios::binary | std::ios::trunc);

	file.write(reinterpret_cast<const char*>(_pipelineCacheData.data()), _pipelineCacheData.size());
}