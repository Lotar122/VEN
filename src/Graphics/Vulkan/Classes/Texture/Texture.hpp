#pragma once

#include <stb_image_include.h>

#include <cstdint>

#include <vulkan/vulkan.hpp>

#include "Classes/Logger/Logger.hpp"
#include "Classes/Buffer/Buffer.hpp"
#include "Classes/Engine/Engine.hpp"

#include "Classes/Resources/Resources.hpp"

#include "Classes/Asset/Asset.hpp"
#include "Classes/AssetUsage/AssetUsage.hpp"

namespace nihil::graphics
{
	class Texture : public Asset
	{
	public:
		alignas(Buffer<
			unsigned char, vk::BufferUsageFlagBits::eTransferSrc, static_cast<vk::MemoryPropertyFlags::MaskType>(vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)
		>) unsigned char stagingBufferMemory[sizeof(Buffer<
			unsigned char, vk::BufferUsageFlagBits::eTransferSrc, static_cast<vk::MemoryPropertyFlags::MaskType>(vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)
		>)];

		Resource<vk::ImageView> imageView;
		Resource<vk::Image> image;

		vk::MemoryAllocateInfo imageMemoryAllocInfo;
		vk::MemoryRequirements imageMemoryRequirements;

		Engine* engine = nullptr;

		unsigned char* imageData = nullptr;

		size_t width = 0, height = 0, channels = 0;
		size_t size = 0;

		Buffer<
			unsigned char, vk::BufferUsageFlagBits::eTransferSrc, static_cast<vk::MemoryPropertyFlags::MaskType>(vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)
		>* stagingBuffer = nullptr;

		vk::DeviceMemory imageMemory;
		vk::ImageLayout currentLayout = vk::ImageLayout::eUndefined;

		bool onGPU = false;

		Texture(const std::string& path, AssetUsage _assetUsage, Engine* _engine, uint8_t desiredChannels = 4);

		Texture(const char* data, size_t _width, size_t _height, uint8_t _channels, AssetUsage _assetUsage, Engine* _engine, uint8_t desiredChannels = 4);

		vk::DescriptorSetLayoutBinding  getDescriptorSetLayoutBinding(vk::ShaderStageFlagBits shaderStage, uint32_t binding);

		vk::DescriptorImageInfo getDescriptorInfo();

		void moveToGPU();

		void freeFromGPU();

		void update(const std::vector<unsigned char>& _data);

		void update(const unsigned char* _data, size_t _size);

		//TODO: function to read the GPU texture data

		~Texture();
	};
}