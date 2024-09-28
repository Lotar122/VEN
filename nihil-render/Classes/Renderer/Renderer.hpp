#pragma once

#include <iostream>
#include "Classes/Engine/Engine.Structs.InData.hpp"
#include "nstd/nstd.hpp"
#include "Classes/Buffer/Buffer.hpp"
#include "Classes/InstanceData/InstanceData.hpp"
#include "Classes/Commands/Command.hpp"

#include "Classes/Camera/Camera.hpp"

namespace nihil::graphics {
	class Engine;
	class Scene;
	class Model;

	class Renderer {
		friend class Engine;
	public:
		Renderer(Engine* _engine);
		~Renderer();

		//creates a shader module
		void CreateShaderModule(std::string filepath, vk::Device device, vk::ShaderModule** ppShaderModule);

		//Draw
		void Draw(Camera& camera);

		void drawInstanced(
			Model* model,
			vk::CommandBuffer& _commandBuffer,
			Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>* instanceBuffer,
			PipelineBundle* pBundle,
			Camera* camera
		);
		void drawModel(
			Model* model,
			vk::CommandBuffer& _commandBuffer,
			PipelineBundle* pBundle,
			Camera* camera
		);
		void drawBuffer(
			Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>* _vertexBuffer,
			Buffer<uint32_t, vk::BufferUsageFlagBits::eIndexBuffer>* _indexBuffer,
			vk::CommandBuffer& _commandBuffer,
			PipelineBundle* pBundle,
			Camera* camera,
			Model* model = NULL
		);

		nstd::OBJ* objobject;
	private:
		/*
		* @brief Creates a swapchain
		*
		* @param createInfo - config creation info
		* @return inforamtion to create the swapchain
		*/
		SwapchainConfiguration CreateSwapchainConfiguration(SwapchainConfigCreateInfo createInfo);
		/*
		* @brief Creates a swapchain
		*
		* @param createInfo - information for the creation of the swapchain
		* @return None
		*/
		void CreateSwapchain(SwapchainConfiguration createInfo);
		/*
		* @brief Creates a swapchain
		*
		* @param createInfo - inforamtion to create the swapchain
		* @return None
		*/
		void CreateImageViews();
		//destroy the swapchain
		void destroySwapchain();
		//create the flow-control
		void createSyncObjects();
		//create framebuffers
		void createFrameBuffers();
		//create depthbuffers
		void createDepthBuffers();
		//make frame commandbuffers
		void createFrameCommandBuffers();
		//make main commandbuffer
		void createMainCommandBuffer();
		//make commandpools
		void createCommandPools();
		//reacreate swapchain
		void RecreateSwapchain();

		void executeCommandQueue(std::vector<DrawCommand>* commandQueue, vk::CommandBuffer& commandBuffer, uint32_t imageIndex, Camera& camera);

		uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);

		QueueFamilyIndices famind;
		std::vector<uint32_t> uniqueFAMIND;

		vk::Queue graphicsQueue;
		vk::Queue presentQueue;

		SwapChainSupportDetails support;
		vk::SurfaceFormatKHR surfaceFormat;
		vk::PresentModeKHR presentMode;
		vk::SwapchainCreateInfoKHR swapCreateInfo;

		std::vector<vk::AttachmentDescription> renderPassAttachments;

		SwapChainBundle swapchainBundle{};

		uint32_t imageCount;
	private:
		std::chrono::steady_clock::time_point lastFrameTime{};

		vk::CommandPool commandPool;
		vk::CommandBuffer commandBuffer;

		vk::Viewport viewport = {};
		vk::Rect2D scissor = {};

		uint32_t maxFramesInFlight, frameNumber;

		Engine* engine;

		SwapchainConfiguration swapchainConfiguration;

		//can only be used for resource deletion in the destructor!!!
		vk::Device copyLogicalDevice;
	};
}