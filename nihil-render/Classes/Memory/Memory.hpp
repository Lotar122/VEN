#pragma once
#include <vulkan/vulkan.hpp>

namespace nihil::graphics {
	struct BufferData {
		vk::Buffer buffer;
		vk::DeviceMemory memory;
	};
	class Engine;
	class Memory
	{
	public:
		static BufferData& CreateBuffer(uint32_t size, vk::BufferUsageFlags usage, Engine* engine);

		static uint32_t FindMemoryTypeIndex(Engine* engine, uint32_t supportedMemoryIndex, vk::MemoryPropertyFlags properties);

		static void AllocateBufferMemory(BufferData& buffer, Engine* engine);
	};
}