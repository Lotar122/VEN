#pragma once

#include <stb_image.h>

#include <cstdint>

#include <vulkan/vulkan.hpp>

#include "Logger.hpp"
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
		Engine* engine = nullptr;
		bool onGPU = false;
		unsigned char* imageData = nullptr;
		size_t width = 0, height = 0, channels = 0;
		size_t size = 0;
		alignas(Buffer<
			unsigned char, vk::BufferUsageFlagBits::eTransferSrc, static_cast<vk::MemoryPropertyFlags::MaskType>(vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)
		>) unsigned char stagingBufferMemory[sizeof(Buffer<
			unsigned char, vk::BufferUsageFlagBits::eTransferSrc, static_cast<vk::MemoryPropertyFlags::MaskType>(vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)
		>)];
		Buffer<
			unsigned char, vk::BufferUsageFlagBits::eTransferSrc, static_cast<vk::MemoryPropertyFlags::MaskType>(vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)
		>* stagingBuffer = nullptr;
		Resource<vk::Image> image;
		vk::DeviceMemory imageMemory;
		vk::ImageLayout currentLayout = vk::ImageLayout::eUndefined;
		Resource<vk::ImageView> imageView;

		vk::MemoryRequirements imageMemoryRequirements;
		vk::MemoryAllocateInfo imageMemoryAllocInfo;

		Texture(const std::string& path, AssetUsage _assetUsage, Engine* _engine, uint8_t desiredChannels = 4) : Asset(_assetUsage, _engine)
		{
			assert(_engine != nullptr);
			engine = _engine;

			int _width, _height, _channels;
			unsigned char* temp = stbi_load(path.c_str(), &_width, &_height, &_channels, desiredChannels);

			width = _width;
			height = _height;
			channels = _channels;

			if (channels != desiredChannels)
			{
				Logger::Warn("Stb had loaded a 3 channel file. (jpg, jpeg etc.) please use formats that support alpha. Using formats that do not may cause visual errors. File: {}", path);
			}

			size = width * height * desiredChannels;

			imageData = new unsigned char[size];

			std::memcpy(imageData, temp, size);
			stbi_image_free(temp);

			if (!imageData) Logger::Exception("Failed to load image to create texture.");

			stagingBuffer = new (stagingBufferMemory) Buffer<
				unsigned char, vk::BufferUsageFlagBits::eTransferSrc, static_cast<vk::MemoryPropertyFlags::MaskType>(vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)
			>(imageData, size, engine);

			vk::ImageCreateInfo textureImageCreateInfo = {
				{}, vk::ImageType::e2D, vk::Format::eR8G8B8A8Srgb,
				{ (uint32_t)width, (uint32_t)height, 1 },
				1, 1, vk::SampleCountFlagBits::e1,
				vk::ImageTiling::eOptimal,
				vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
				vk::SharingMode::eExclusive,
				{}, vk::ImageLayout::eUndefined
			};

			image.assignRes(engine->_device().createImage(textureImageCreateInfo), engine->_device());

			imageMemoryRequirements = engine->_device().getImageMemoryRequirements(image);
			imageMemoryAllocInfo = vk::MemoryAllocateInfo(imageMemoryRequirements.size, findMemoryTypeIndex(engine->_physicalDevice().getMemoryProperties(), imageMemoryRequirements, vk::MemoryPropertyFlagBits::eDeviceLocal));
		
			moveToGPU();

			vk::ImageViewCreateInfo imageViewCreateInfo(
				{}, image, vk::ImageViewType::e2D,
				vk::Format::eR8G8B8A8Srgb, {},
				{ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
			);

			imageView.assignRes(engine->_device().createImageView(imageViewCreateInfo), engine->_device());

			//freeFromGPU();
		}

		Texture(const char* data, size_t _width, size_t _height, uint8_t _channels, AssetUsage _assetUsage, Engine* _engine, uint8_t desiredChannels = 4) : Asset(_assetUsage, _engine)
		{
			assert(_engine != nullptr);
			engine = _engine;

			width = _width;
			height = _height;
			channels = _channels;

			if (channels != desiredChannels)
			{
				Logger::Warn("Loaded a 3 channel file. (jpg, jpeg etc.) please use formats that support alpha. Using formats that do not may cause visual errors or crashes. File: [Loaded from memory, {}]", reinterpret_cast<void*>(imageData));
			}

			size = width * height * desiredChannels;

			imageData = new unsigned char[size];

			std::memcpy(imageData, data, size);

			if (!imageData) Logger::Exception("Failed to load image to create texture.");

			stagingBuffer = new (stagingBufferMemory) Buffer<
				unsigned char, vk::BufferUsageFlagBits::eTransferSrc, static_cast<vk::MemoryPropertyFlags::MaskType>(vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)
			>(imageData, size, engine);

			vk::ImageCreateInfo textureImageCreateInfo = {
				{}, vk::ImageType::e2D, vk::Format::eR8G8B8A8Srgb,
				{ (uint32_t)width, (uint32_t)height, 1 },
				1, 1, vk::SampleCountFlagBits::e1,
				vk::ImageTiling::eOptimal,
				vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
				vk::SharingMode::eExclusive,
				{}, vk::ImageLayout::eUndefined
			};

			image.assignRes(engine->_device().createImage(textureImageCreateInfo), engine->_device());

			imageMemoryRequirements = engine->_device().getImageMemoryRequirements(image);
			imageMemoryAllocInfo = vk::MemoryAllocateInfo(imageMemoryRequirements.size, findMemoryTypeIndex(engine->_physicalDevice().getMemoryProperties(), imageMemoryRequirements, vk::MemoryPropertyFlagBits::eDeviceLocal));

			moveToGPU();

			vk::ImageViewCreateInfo imageViewCreateInfo(
				{}, image, vk::ImageViewType::e2D,
				vk::Format::eR8G8B8A8Srgb, {},
				{ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
			);

			imageView.assignRes(engine->_device().createImageView(imageViewCreateInfo), engine->_device());

			//freeFromGPU();
		}

		vk::DescriptorSetLayoutBinding  getDescriptorSetLayoutBinding(vk::ShaderStageFlagBits shaderStage, uint32_t binding)
		{
			vk::DescriptorSetLayoutBinding textureBinding{};
			textureBinding.binding = binding;
			textureBinding.descriptorType = vk::DescriptorType::eSampledImage;
			textureBinding.descriptorCount = 1;
			textureBinding.stageFlags = shaderStage;
			textureBinding.pImmutableSamplers = nullptr;

			return textureBinding;
		}

		vk::DescriptorImageInfo getDescriptorInfo()
		{
			vk::DescriptorImageInfo textureInfo{
				{},      // vk::Sampler (or {} if not needed)
				imageView,    // vk::ImageView (or {} if not needed)
				currentLayout
			};

			return textureInfo;
		}

		void moveToGPU()
		{
			if (onGPU) return;

			imageMemory = engine->_device().allocateMemory(imageMemoryAllocInfo), engine->_device();
			engine->_device().bindImageMemory(image, imageMemory, 0);

			stagingBuffer->moveToGPU();

			vk::Result discardResult = engine->_device().waitForFences(engine->_transferFence(), true, UINT64_MAX);
			discardResult = engine->_device().resetFences(1, &engine->_transferFence());

			vk::CommandBufferBeginInfo beginInfo{};
			engine->_mainCommandBuffer().begin(beginInfo);

			vk::ImageMemoryBarrier barrier;

			if (currentLayout == vk::ImageLayout::eUndefined)
			{
				// Transition to TRANSFER_DST_OPTIMAL
				barrier = vk::ImageMemoryBarrier(
					{}, vk::AccessFlagBits::eTransferWrite,
					vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
					VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
					image, { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
				);

				engine->_mainCommandBuffer().pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, {}, {}, {}, barrier);
			}
			else if (currentLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
			{
				// Transition to TRANSFER_DST_OPTIMAL
				barrier = vk::ImageMemoryBarrier(
					{}, vk::AccessFlagBits::eTransferWrite,
					vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eTransferDstOptimal,
					VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
					image, { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
				);

				engine->_mainCommandBuffer().pipelineBarrier(vk::PipelineStageFlagBits::eFragmentShader, vk::PipelineStageFlagBits::eTransfer, {}, {}, {}, barrier);
			}

			vk::BufferImageCopy region(
				0, 0, 0,
				{ vk::ImageAspectFlagBits::eColor, 0, 0, 1 },
				{ 0, 0, 0 },
				{ (uint32_t)width, (uint32_t)height, 1 }
			);

			engine->_mainCommandBuffer().copyBufferToImage(stagingBuffer->_buffer(), image, vk::ImageLayout::eTransferDstOptimal, region);

			barrier = vk::ImageMemoryBarrier(
				vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderRead,
				vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
				VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
				image, { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
			);

			engine->_mainCommandBuffer().pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, barrier);

			engine->_mainCommandBuffer().end();

			vk::SubmitInfo submitInfo = {};
			submitInfo.waitSemaphoreCount = 0;
			submitInfo.pWaitSemaphores = nullptr;

			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &engine->_mainCommandBuffer();

			discardResult = engine->_renderQueue().submit(1, &submitInfo, engine->_transferFence());

			onGPU = true;

			currentLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		}

		void freeFromGPU()
		{
			engine->_device().freeMemory(imageMemory);
			stagingBuffer->freeFromGPU();
			onGPU = false;

			currentLayout = vk::ImageLayout::eUndefined;
		}

		void update(const std::vector<unsigned char>& _data)
		{
			assert(_data.size() == size);

			std::memcpy(imageData, _data.data(), size);

			bool wasOnGPU = onGPU;

			moveToGPU();

			stagingBuffer->update(imageData, size);

			vk::Result discardResult = engine->_device().waitForFences(engine->_transferFence(), true, UINT64_MAX);
			discardResult = engine->_device().resetFences(1, &engine->_transferFence());

			vk::CommandBufferBeginInfo beginInfo{};
			engine->_mainCommandBuffer().begin(beginInfo);

			vk::ImageMemoryBarrier barrier;

			if (currentLayout == vk::ImageLayout::eUndefined)
			{
				// Transition to TRANSFER_DST_OPTIMAL
				barrier = vk::ImageMemoryBarrier(
					{}, vk::AccessFlagBits::eTransferWrite,
					vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
					VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
					image, { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
				);

				engine->_mainCommandBuffer().pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, {}, {}, {}, barrier);
			}
			else if (currentLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
			{
				// Transition to TRANSFER_DST_OPTIMAL
				barrier = vk::ImageMemoryBarrier(
					{}, vk::AccessFlagBits::eTransferWrite,
					vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eTransferDstOptimal,
					VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
					image, { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
				);

				engine->_mainCommandBuffer().pipelineBarrier(vk::PipelineStageFlagBits::eFragmentShader, vk::PipelineStageFlagBits::eTransfer, {}, {}, {}, barrier);
			}

			vk::BufferImageCopy region(
				0, 0, 0,
				{ vk::ImageAspectFlagBits::eColor, 0, 0, 1 },
				{ 0, 0, 0 },
				{ (uint32_t)width, (uint32_t)height, 1 }
			);

			engine->_mainCommandBuffer().copyBufferToImage(stagingBuffer->_buffer(), image, vk::ImageLayout::eTransferDstOptimal, region);

			barrier = vk::ImageMemoryBarrier(
				vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderRead,
				vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
				VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
				image, { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
			);

			engine->_mainCommandBuffer().pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, barrier);

			engine->_mainCommandBuffer().end();

			vk::SubmitInfo submitInfo = {};
			submitInfo.waitSemaphoreCount = 0;
			submitInfo.pWaitSemaphores = nullptr;

			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &engine->_mainCommandBuffer();

			discardResult = engine->_renderQueue().submit(1, &submitInfo, engine->_transferFence());

			currentLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

			if (!wasOnGPU) freeFromGPU();
		}

		void update(const unsigned char* _data, size_t _size)
		{
			assert(_size == size);

			std::memcpy(imageData, _data, size);

			bool wasOnGPU = onGPU;

			moveToGPU();

			stagingBuffer->update(imageData, size);

			vk::Result discardResult = engine->_device().waitForFences(engine->_transferFence(), true, UINT64_MAX);
			discardResult = engine->_device().resetFences(1, &engine->_transferFence());

			vk::CommandBufferBeginInfo beginInfo{};
			engine->_mainCommandBuffer().begin(beginInfo);

			vk::ImageMemoryBarrier barrier;

			if (currentLayout == vk::ImageLayout::eUndefined)
			{
				// Transition to TRANSFER_DST_OPTIMAL
				barrier = vk::ImageMemoryBarrier(
					{}, vk::AccessFlagBits::eTransferWrite,
					vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
					VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
					image, { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
				);

				engine->_mainCommandBuffer().pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, {}, {}, {}, barrier);
			}
			else if (currentLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
			{
				// Transition to TRANSFER_DST_OPTIMAL
				barrier = vk::ImageMemoryBarrier(
					{}, vk::AccessFlagBits::eTransferWrite,
					vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eTransferDstOptimal,
					VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
					image, { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
				);

				engine->_mainCommandBuffer().pipelineBarrier(vk::PipelineStageFlagBits::eFragmentShader, vk::PipelineStageFlagBits::eTransfer, {}, {}, {}, barrier);
			}

			vk::BufferImageCopy region(
				0, 0, 0,
				{ vk::ImageAspectFlagBits::eColor, 0, 0, 1 },
				{ 0, 0, 0 },
				{ (uint32_t)width, (uint32_t)height, 1 }
			);

			engine->_mainCommandBuffer().copyBufferToImage(stagingBuffer->_buffer(), image, vk::ImageLayout::eTransferDstOptimal, region);

			barrier = vk::ImageMemoryBarrier(
				vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderRead,
				vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
				VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
				image, { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
			);

			engine->_mainCommandBuffer().pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, barrier);

			engine->_mainCommandBuffer().end();

			vk::SubmitInfo submitInfo = {};
			submitInfo.waitSemaphoreCount = 0;
			submitInfo.pWaitSemaphores = nullptr;

			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &engine->_mainCommandBuffer();

			discardResult = engine->_renderQueue().submit(1, &submitInfo, engine->_transferFence());

			currentLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

			if (!wasOnGPU) freeFromGPU();
		}

		//TODO: function to read the GPU texture data

		~Texture()
		{
			delete[] imageData;
			stagingBuffer->~Buffer();

			if(onGPU) engine->_device().freeMemory(imageMemory);
		}
	};
}