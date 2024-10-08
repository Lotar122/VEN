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

		vk::RenderPass createClearScreenRenderPass(vk::Device device, vk::Format colorFormat, vk::Format depthFormat) {
			// Define the color attachment description
			vk::AttachmentDescription colorAttachment = {};
			colorAttachment.setFormat(colorFormat);
			colorAttachment.setSamples(vk::SampleCountFlagBits::e1);
			colorAttachment.setLoadOp(vk::AttachmentLoadOp::eClear); // Clear the attachment
			colorAttachment.setStoreOp(vk::AttachmentStoreOp::eStore);
			colorAttachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
			colorAttachment.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
			colorAttachment.setInitialLayout(vk::ImageLayout::eUndefined);
			colorAttachment.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

			// Define the depth attachment description
			vk::AttachmentDescription depthAttachment = {};
			depthAttachment.setFormat(depthFormat);
			depthAttachment.setSamples(vk::SampleCountFlagBits::e1);
			depthAttachment.setLoadOp(vk::AttachmentLoadOp::eClear); // Clear the depth buffer
			depthAttachment.setStoreOp(vk::AttachmentStoreOp::eDontCare);
			depthAttachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
			depthAttachment.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
			depthAttachment.setInitialLayout(vk::ImageLayout::eUndefined);
			depthAttachment.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

			// Define the color attachment reference
			vk::AttachmentReference colorAttachmentRef = {};
			colorAttachmentRef.setAttachment(0);
			colorAttachmentRef.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

			// Define the depth attachment reference
			vk::AttachmentReference depthAttachmentRef = {};
			depthAttachmentRef.setAttachment(1);
			depthAttachmentRef.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

			// Define the subpass description
			vk::SubpassDescription subpass = {};
			subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
			subpass.setColorAttachmentCount(1);
			subpass.setPColorAttachments(&colorAttachmentRef);
			subpass.setInputAttachmentCount(0);
			subpass.setPDepthStencilAttachment(&depthAttachmentRef);

			vk::SubpassDependency dependency(
				VK_SUBPASS_EXTERNAL, 0,
				vk::PipelineStageFlagBits::eEarlyFragmentTests,
				vk::PipelineStageFlagBits::eLateFragmentTests,
				vk::AccessFlagBits::eDepthStencilAttachmentWrite,
				vk::AccessFlagBits::eDepthStencilAttachmentRead
			);

			// Define the render pass creation info
			vk::RenderPassCreateInfo renderPassInfo = {};
			renderPassInfo.setAttachmentCount(2);
			vk::AttachmentDescription attachments[] = { colorAttachment, depthAttachment };
			renderPassInfo.setPAttachments(attachments);
			renderPassInfo.setSubpassCount(1);
			renderPassInfo.setPSubpasses(&subpass);
			renderPassInfo.dependencyCount = 1;
			renderPassInfo.pDependencies = &dependency;

			// Create the render pass
			vk::RenderPass renderPass;
			if (device.createRenderPass(&renderPassInfo, nullptr, &renderPass) != vk::Result::eSuccess) {
				throw std::runtime_error("Failed to create render pass!");
			}

			return renderPass;
		}

		vk::RenderPass clearScreenPass;

		void clearScreen(vk::CommandBuffer commandBuffer, vk::RenderPass renderPass, vk::Framebuffer framebuffer, vk::Extent2D extent) {
			// Define the clear colors
			vk::ClearValue clearColor = vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}); // Black
			vk::ClearValue clearDepth = vk::ClearDepthStencilValue(1.0f, 0); // Depth clear value (1.0f for farthest)

			vk::ClearValue clears[] = {
				clearColor, clearDepth
			};

			// Create a render pass begin info structure
			vk::RenderPassBeginInfo renderPassInfo(
				renderPass,
				framebuffer,
				vk::Rect2D({ 0, 0 }, extent),
				2, // Number of clear values
				clears
			);

			// Begin the render pass
			commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

			// End the render pass
			commandBuffer.endRenderPass();
		}

		//can only be used for resource deletion in the destructor!!!
		vk::Device copyLogicalDevice;
	};
}