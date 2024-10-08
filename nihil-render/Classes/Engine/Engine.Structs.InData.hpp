#pragma once
#include <iostream>
#include <vector>
#include <iostream>
#include <set>
#include <optional>
#include <vulkan/vulkan.hpp>

namespace nihil::graphics {
	using SwapchainConfiguration = vk::SwapchainCreateInfoKHR;

	//!!! move to nstd
	struct Version {
		uint32_t variant;
		uint32_t major;
		uint32_t minor;
		uint32_t patch;

		void make_version(uint32_t _variant, uint32_t _major, uint32_t _minor, uint32_t _patch)
		{
			variant = _variant;
			major = _major;
			minor = _minor;
			patch = _patch;
		}
	};

	struct VertexAttribute {
		uint32_t binding;
		uint32_t location;

		vk::Format format;
		uint32_t offset;
	};
	struct VertexBindingInformation {
		vk::VertexInputRate inputRate;
		uint32_t stride;
	};
	struct PipelineInfo {
		std::vector<vk::VertexInputBindingDescription> bindingDesc;
		std::vector<vk::VertexInputAttributeDescription> attributeDesc;

		vk::ShaderModule* vertexShader;
		vk::ShaderModule* fragmentShader;
	};
	struct PipelineBundle {
		vk::Pipeline pipeline;
		vk::RenderPass renderPass;
		vk::PipelineLayout layout;

		bool operator==(const PipelineBundle& other) const 
		{
			return this->pipeline == other.pipeline && this->layout == other.layout && this->renderPass == other.renderPass;
		}
		bool operator!=(const PipelineBundle& other) const
		{
			return !(this->pipeline == other.pipeline && this->layout == other.layout && this->renderPass == other.renderPass);
		}
	};
	enum class AttachmentType
	{
		Depth,
		Color
	};
	struct Attachment
	{
		vk::SampleCountFlagBits sampleCount;
		vk::AttachmentLoadOp loadOp;
		vk::AttachmentLoadOp stencilLoadOp;
		vk::AttachmentStoreOp storeOp;
		vk::AttachmentStoreOp stencilStoreOp;

		vk::ImageLayout initialLayout;
		vk::ImageLayout finalLayout;

		AttachmentType type;
	};

	//for now only a color attachment and a depth attachment are supported
	struct RenderPassInfo
	{
		std::vector<Attachment> attachments;


	};

	struct VulkanInstanceCreateInfo {
		Version appVersion;
		Version vulkanVersion;
		std::string appName;
		bool validationLayers = false;
	};

	enum class BufferingMode {
		eSingle = 1,
		eDouble = 2,
		eTriple = 3,
		eQuadruple = 4
	};

	struct SwapchainConfigCreateInfo {
		uint16_t windowWidth;
		uint16_t windowHeight;
		BufferingMode preferredBuffering;
	};

	struct SwapchainSupportDetails {
		vk::SurfaceCapabilitiesKHR capabilities;
		std::vector<vk::SurfaceFormatKHR> formats;
		std::vector<vk::PresentModeKHR> presentModes;
	};

	struct WindowData {
		int width;
		int height;
		std::string name;
	};
	struct QueueFamilyIndices {
		std::optional<uint32_t> graphics;
		std::optional<uint32_t> present;

		bool isComplete()
		{
			return graphics.has_value() && present.has_value();
		}
	};
	struct SwapChainSupportDetails {
		vk::SurfaceCapabilitiesKHR capabilities;
		std::vector<vk::SurfaceFormatKHR> formats;
		std::vector<vk::PresentModeKHR> presentModes;
	};
	struct SwapChainFrame {
		vk::Image image;
		vk::ImageView view;
		vk::Framebuffer frameBuffer;
		vk::CommandBuffer commandBuffer;
		vk::Image depthBuffer;
		vk::DeviceMemory depthBufferMemory;
		vk::ImageView depthBufferView;
		vk::Format depthFormat;
		uint16_t width;
		uint16_t height;

		vk::Semaphore imageAvailable, renderFinished;
		vk::Fence inFlightFence;
	};
	struct SwapChainBundle {
		vk::SwapchainKHR swapchain;
		std::vector<SwapChainFrame> frames;
		vk::Format format;
		vk::Format depthFormat;
		vk::Extent2D extent;
	};
}