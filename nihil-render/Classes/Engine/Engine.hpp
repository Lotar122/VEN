#pragma once
#define GRAPHICS_INCLUDES
#include <vulkan/vulkan.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <set>
#include <optional>
#include <vector>
#include "implicit_cast.hpp"
#include "Classes/SPIRV/SPIRV.hpp"

//!!! Enable colors
#define USE_COLORS
#include "TerminalColors.hpp"

#include "Classes/App/App.hpp"
#include "Engine.Structs.InData.hpp"

#include "RenderStructs.hpp"

#include "Classes/Scene/Scene.hpp"

#define GLM_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Classes/Renderer/Renderer.hpp"
#include "Classes/Buffer/Buffer.hpp"

#include "Classes/Commands/Command.hpp"

#include "Classes/Camera/Camera.hpp"

#include "nstd/headers/Memory.hpp"

#define commandDataArenaSize 16'777'216
#define shaderPoolSize 0

namespace nihil::graphics {

    enum class ResourceType {
        Buffer,
        Image,
        ImageView,
        Pipeline,
        PipelineLayout,
        RenderPass,
        ShaderModule
    };

	struct EngineCreateInfo {

	};

	class Engine
	{
		friend class Renderer;
	private:
		struct Proxy {
			const vk::Instance* instance;
			const vk::PhysicalDevice* physicalDevice;
			const vk::Device* logicalDevice;

			Proxy(
				const vk::Instance* instance,
				const vk::PhysicalDevice* physicalDevice,
				const vk::Device* logicalDevice
			)
			{
				this->instance = instance;
				this->logicalDevice = logicalDevice;
				this->physicalDevice = physicalDevice;
			}
		};

	public:
		App* app = NULL;
		bool shouldClose = false;
		bool debug = false;
        Renderer* renderer = NULL;

		Engine(bool _debug);
		Engine();
		~Engine();

		void setApp(App* _app);

		//Setup (for now just calls setupdeafult)
		void Setup(bool validation);

		Proxy* get;

		nstd::OBJ objobject;

		void Draw(Camera& camera);

		inline uint32_t GetFormatLenght(vk::Format format)
		{
            switch (format) {
            case vk::Format::eUndefined:
                return 0; // Handle undefined format appropriately
            case vk::Format::eR4G4UnormPack8:
                return 1 * sizeof(uint8_t); // Handle R4G4 format
            case vk::Format::eR4G4B4A4UnormPack16:
            case vk::Format::eB4G4R4A4UnormPack16:
                return 2 * sizeof(uint8_t); // Handle R4G4B4A4 or B4G4R4A4 format
            case vk::Format::eR5G6B5UnormPack16:
            case vk::Format::eB5G6R5UnormPack16:
                return 2 * sizeof(uint8_t); // Handle R5G6B5 or B5G6R5 format
            case vk::Format::eR5G5B5A1UnormPack16:
            case vk::Format::eB5G5R5A1UnormPack16:
            case vk::Format::eA1R5G5B5UnormPack16:
                return 2 * sizeof(uint8_t); // Handle R5G5B5A1, B5G5R5A1, or A1R5G5B5 format
            case vk::Format::eR8Unorm:
            case vk::Format::eR8Snorm:
            case vk::Format::eR8Uscaled:
            case vk::Format::eR8Sscaled:
            case vk::Format::eR8Uint:
            case vk::Format::eR8Sint:
            case vk::Format::eR8Srgb:
                return 1 * sizeof(uint8_t); // Handle R8 format
            case vk::Format::eR8G8Unorm:
            case vk::Format::eR8G8Snorm:
            case vk::Format::eR8G8Uscaled:
            case vk::Format::eR8G8Sscaled:
            case vk::Format::eR8G8Uint:
            case vk::Format::eR8G8Sint:
            case vk::Format::eR8G8Srgb:
                return 2 * sizeof(uint8_t); // Handle R8G8 format
            case vk::Format::eR8G8B8Unorm:
            case vk::Format::eR8G8B8Snorm:
            case vk::Format::eR8G8B8Uscaled:
            case vk::Format::eR8G8B8Sscaled:
            case vk::Format::eR8G8B8Uint:
            case vk::Format::eR8G8B8Sint:
            case vk::Format::eR8G8B8Srgb:
                return 3 * sizeof(uint8_t); // Handle R8G8B8 format
            case vk::Format::eB8G8R8Unorm:
            case vk::Format::eB8G8R8Snorm:
            case vk::Format::eB8G8R8Uscaled:
            case vk::Format::eB8G8R8Sscaled:
            case vk::Format::eB8G8R8Uint:
            case vk::Format::eB8G8R8Sint:
            case vk::Format::eB8G8R8Srgb:
                return 3 * sizeof(uint8_t); // Handle B8G8R8 format
            case vk::Format::eR8G8B8A8Unorm:
            case vk::Format::eR8G8B8A8Snorm:
            case vk::Format::eR8G8B8A8Uscaled:
            case vk::Format::eR8G8B8A8Sscaled:
            case vk::Format::eR8G8B8A8Uint:
            case vk::Format::eR8G8B8A8Sint:
            case vk::Format::eR8G8B8A8Srgb:
                return 4 * sizeof(uint8_t); // Handle R8G8B8A8 format
            case vk::Format::eB8G8R8A8Unorm:
            case vk::Format::eB8G8R8A8Snorm:
            case vk::Format::eB8G8R8A8Uscaled:
            case vk::Format::eB8G8R8A8Sscaled:
            case vk::Format::eB8G8R8A8Uint:
            case vk::Format::eB8G8R8A8Sint:
            case vk::Format::eB8G8R8A8Srgb:
                return 4 * sizeof(uint8_t); // Handle B8G8R8A8 format
            case vk::Format::eA8B8G8R8UnormPack32:
            case vk::Format::eA8B8G8R8SnormPack32:
            case vk::Format::eA8B8G8R8UscaledPack32:
            case vk::Format::eA8B8G8R8SscaledPack32:
            case vk::Format::eA8B8G8R8UintPack32:
            case vk::Format::eA8B8G8R8SintPack32:
            case vk::Format::eA8B8G8R8SrgbPack32:
                return 4 * sizeof(uint8_t); // Handle A8B8G8R8 format
            case vk::Format::eA2R10G10B10UnormPack32:
            case vk::Format::eA2R10G10B10SnormPack32:
            case vk::Format::eA2R10G10B10UscaledPack32:
            case vk::Format::eA2R10G10B10SscaledPack32:
            case vk::Format::eA2R10G10B10UintPack32:
            case vk::Format::eA2R10G10B10SintPack32:
                return 4 * sizeof(uint8_t); // Handle A2R10G10B10 format
            case vk::Format::eA2B10G10R10UnormPack32:
            case vk::Format::eA2B10G10R10SnormPack32:
            case vk::Format::eA2B10G10R10UscaledPack32:
            case vk::Format::eA2B10G10R10SscaledPack32:
            case vk::Format::eA2B10G10R10UintPack32:
            case vk::Format::eA2B10G10R10SintPack32:
                return 4 * sizeof(uint8_t); // Handle A2B10G10R10 format
            case vk::Format::eR16Unorm:
            case vk::Format::eR16Snorm:
            case vk::Format::eR16Uscaled:
            case vk::Format::eR16Sscaled:
            case vk::Format::eR16Uint:
            case vk::Format::eR16Sint:
            case vk::Format::eR16Sfloat:
                return 1 * sizeof(uint16_t); // Handle R16 format
            case vk::Format::eR16G16Unorm:
            case vk::Format::eR16G16Snorm:
            case vk::Format::eR16G16Uscaled:
            case vk::Format::eR16G16Sscaled:
            case vk::Format::eR16G16Uint:
            case vk::Format::eR16G16Sint:
            case vk::Format::eR16G16Sfloat:
                return 2 * sizeof(uint16_t); // Handle R16G16 format
            case vk::Format::eR16G16B16Unorm:
            case vk::Format::eR16G16B16Snorm:
            case vk::Format::eR16G16B16Uscaled:
            case vk::Format::eR16G16B16Sscaled:
            case vk::Format::eR16G16B16Uint:
            case vk::Format::eR16G16B16Sint:
            case vk::Format::eR16G16B16Sfloat:
                return 3 * sizeof(uint16_t); // Handle R16G16B16 format
            case vk::Format::eR16G16B16A16Unorm:
            case vk::Format::eR16G16B16A16Snorm:
            case vk::Format::eR16G16B16A16Uscaled:
            case vk::Format::eR16G16B16A16Sscaled:
            case vk::Format::eR16G16B16A16Uint:
            case vk::Format::eR16G16B16A16Sint:
            case vk::Format::eR16G16B16A16Sfloat:
                return 4 * sizeof(uint16_t); // Handle R16G16B16A16 format
            case vk::Format::eR32Uint:
            case vk::Format::eR32Sint:
            case vk::Format::eR32Sfloat:
                return 1 * sizeof(uint32_t); // Handle R32 format
            case vk::Format::eR32G32Uint:
            case vk::Format::eR32G32Sint:
            case vk::Format::eR32G32Sfloat:
                return 2 * sizeof(uint32_t); // Handle R32G32 format
            case vk::Format::eR32G32B32Uint:
            case vk::Format::eR32G32B32Sint:
            case vk::Format::eR32G32B32Sfloat:
                return 3 * sizeof(uint32_t); // Handle R32G32B32 format
            case vk::Format::eR32G32B32A32Uint:
            case vk::Format::eR32G32B32A32Sint:
            case vk::Format::eR32G32B32A32Sfloat:
                return 4 * sizeof(uint32_t); // Handle R32G32B32A32 format
            case vk::Format::eR64Uint:
            case vk::Format::eR64Sint:
            case vk::Format::eR64Sfloat:
                return 1 * sizeof(uint64_t); // Handle R64 format
            case vk::Format::eR64G64Uint:
            case vk::Format::eR64G64Sint:
            case vk::Format::eR64G64Sfloat:
                return 2 * sizeof(uint64_t); // Handle R64G64 format
            case vk::Format::eR64G64B64Uint:
            case vk::Format::eR64G64B64Sint:
            case vk::Format::eR64G64B64Sfloat:
                return 3 * sizeof(uint64_t); // Handle R64G64B64 format
            case vk::Format::eR64G64B64A64Uint:
            case vk::Format::eR64G64B64A64Sint:
            case vk::Format::eR64G64B64A64Sfloat:
                return 4 * sizeof(uint64_t); // Handle R64G64B64A64 format
            default:
                return 0; // Handle unknown format or other formats not covered above
            }
		}

        PipelineInfo CreatePipelineConfiguration(std::vector<VertexAttribute> attributes, std::vector<VertexBindingInformation> bindingInfo, vk::ShaderModule* vertexShader, vk::ShaderModule* fragmentShader);
        PipelineBundle CreatePipeline(PipelineInfo pipelineInfoN, vk::RenderPass _renderPass);

        vk::RenderPass CreateRenderPass(RenderPassInfo info, SwapChainBundle* swapchainBundle);

        uint32_t registerPipeline(PipelineBundle pipeline);
        PipelineBundle* getPipeline(uint32_t index);

        void registerObjectForDeletion(BufferBase* buffer);
        void registerObjectForDeletion(vk::Image image);
        void registerObjectForDeletion(vk::ImageView imageView);
        void registerObjectForDeletion(vk::DeviceMemory memory);
        void registerObjectForDeletion(vk::ShaderModule shaderModule);
        void registerObjectForDeletion(vk::PipelineLayout pipelineLayout);
        void registerObjectForDeletion(vk::RenderPass renderPass);

        void queueInstancedDraw(
            Model* model,
            Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>* instanceBuffer,
            uint32_t pipeline
        );
        void queueBufferDraw(
            Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>* vertexBuffer,
            Buffer<uint32_t, vk::BufferUsageFlagBits::eIndexBuffer>* indexBuffer,
            uint32_t pipeline
        );
        void queueModelDraw(
            Model* model,
            uint32_t pipeline
        );

        vk::RenderPass renderPass;

        int targetFPS = 120;
        std::chrono::microseconds frameDuration = std::chrono::microseconds(1000000 / targetFPS);

        SwapChainBundle* swapchain;

        inline float getAspectRatio() const {
            return (float)swapchain->extent.width / (float)swapchain->extent.height;
        }
	private:
        std::vector<BufferBase*> bufferStorage;
        std::vector<vk::Image> imageStorage;
        std::vector<vk::ImageView> imageViewStorage;
        std::vector<vk::DeviceMemory> memoryStorage;
        std::vector<vk::ShaderModule> shaderModuleStorage;
        std::vector<vk::PipelineLayout> pipelineLayoutStorage;
        std::vector<vk::RenderPass> renderPassStorage;
        std::vector<PipelineBundle> pipelineStorage;

        nstd::MemoryArena commandDataManager = nstd::MemoryArena(commandDataArenaSize);
        nstd::MemoryPool shaderManager = nstd::MemoryPool(sizeof(vk::ShaderModule), 256);
		/*
		* @brief Creates a Vulkan Instance
		*
		* @param createInfo - Information for the creation of a vulkan instance
		* @return None
		*/
		void CreateVulkanInstance(VulkanInstanceCreateInfo createInfo);
		/*
		* @brief Picks the Device for you
		*
		* @param None
		* @return None
		*/
		void PickPhysicalDevice();
		/*
		* @brief Creates a Vulkan Surface
		*
		* @param window - a pointer to a GLFW window thats used by your application
		* @return None
		*/
		void CreateVulkanSurface(GLFWwindow* window);
		/*
		* @brief Creates a Vulkan logical dedvice
		*
		* @param None
		* @return None
		*/
		void CreateVulkanLogicalDevice();
		/*
		* @brief Creates the Present Queue and the Render Queue
		*
		* @param None
		* @return None
		*/
		void CreateVulkanQueues();

        void CreateRenderPass(SwapChainBundle* swapchainBundle);

		vk::Instance instance;
		vk::PhysicalDevice device;
		vk::SurfaceKHR surface;
		QueueFamilyIndices famind;
		std::vector<uint32_t> uniqueFAMIND;
		vk::Device logicalDevice;

		vk::Queue graphicsQueue;
		vk::Queue presentQueue;

        std::vector<DrawCommand> commandQueue;

		bool error = false;

		inline void finishSetup()
		{
			if (error)
			{
				std::cerr << std::endl << YELLOW << "[Setup]" << RESET << "Engine setup summary: " << RED << "[###]" << RESET << std::endl << std::endl;
				std::cerr << BLUE << "[====================================================================================================================]" << RESET << std::endl << std::endl;
			}
			else
			{
				std::cout << std::endl << YELLOW << "[Setup]" << RESET << "Engine setup summary: " << GREEN << "[###]" << RESET << std::endl << std::endl;
				std::cout << BLUE << "[====================================================================================================================]" << RESET << std::endl << std::endl;
			}
		}
		inline void finishPipelineSetup()
		{
			if (error)
			{
				std::cerr << std::endl << YELLOW << "[Setup]" << MAGENTA << "->" << YELLOW << "[Pipeline]" << RESET << "Pipeline setup summary: " << RED << "[###]" << RESET << std::endl << std::endl;
				std::cerr << BLUE << "[====================================================================================================================]" << RESET << std::endl << std::endl;
			}
			else
			{
				std::cout << std::endl << YELLOW << "[Setup]" << MAGENTA << "->" << YELLOW << "[Pipeline]" << RESET << "Pipeline setup summary: " << GREEN << "[###]" << RESET << std::endl << std::endl;
				std::cout << BLUE << "[====================================================================================================================]" << RESET << std::endl << std::endl;
			}
		}
		inline void finishPrimarySetup()
		{
			if (error)
			{
				std::cerr << std::endl << YELLOW << "[Setup]" << RESET << "Primary Engine setup summary: " << RED << "[###]" << RESET << std::endl << std::endl;
				std::cerr << BLUE << "[====================================================================================================================]" << RESET << std::endl << std::endl;
			}
			else
			{
				std::cout << std::endl << YELLOW << "[Setup]" << RESET << "Primary Engine setup summary: " << GREEN << "[###]" << RESET << std::endl << std::endl;
				std::cout << BLUE << "[====================================================================================================================]" << RESET << std::endl << std::endl;
			}
		}

		inline bool isSuitable(const vk::PhysicalDevice& device)
		{
			if (!(device.getProperties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu));
			const std::vector<const char*> requestedExtensions = {
				VK_KHR_SWAPCHAIN_EXTENSION_NAME
			};
			std::set<std::string> requiredExtensions(requestedExtensions.begin(), requestedExtensions.end());
			for (const vk::ExtensionProperties& extension : device.enumerateDeviceExtensionProperties())
			{
				requiredExtensions.erase(extension.extensionName);
			}
            if (requiredExtensions.empty() && !(device.getProperties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu))
            {
                std::cout << "Continuing with an integrated GPU" << std::endl;
            }
			return requiredExtensions.empty();
		}
	};
};