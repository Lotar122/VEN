#include "Memory.hpp"
#include "Classes/Engine/Engine.hpp"

namespace nihil::graphics {
	BufferData& Memory::CreateBuffer(uint32_t size, vk::BufferUsageFlags usage, Engine* engine)
	{
		vk::BufferCreateInfo createInfo = {};
		createInfo.flags = vk::BufferCreateFlags();
		createInfo.size = size;
		createInfo.sharingMode = vk::SharingMode::eExclusive;
		createInfo.usage = usage;

		BufferData buffer;
		buffer.buffer = engine->get->logicalDevice->createBuffer(createInfo);
		AllocateBufferMemory(buffer, engine);

		return buffer;
	}

	uint32_t Memory::FindMemoryTypeIndex(Engine* engine, uint32_t supportedMemoryIndex, vk::MemoryPropertyFlags properties)
	{
		vk::PhysicalDeviceMemoryProperties memoryProperties = engine->get->physicalDevice->getMemoryProperties();

		for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
		{
			bool supported{ static_cast<bool>(supportedMemoryIndex & (1 << i)) };
			bool sufficient{ (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties };
			if (supported && sufficient)
			{
				return i;
			}
		}
	}

	void Memory::AllocateBufferMemory(BufferData& buffer, Engine* engine)
	{
		vk::MemoryRequirements memoryRequirements = engine->get->logicalDevice->getBufferMemoryRequirements(buffer.buffer);

		vk::MemoryAllocateInfo allocInfo = {};
		allocInfo.allocationSize = memoryRequirements.size;
		allocInfo.memoryTypeIndex = Memory::FindMemoryTypeIndex(
			engine, memoryRequirements.memoryTypeBits,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
		);
		buffer.memory = engine->get->logicalDevice->allocateMemory(allocInfo);
		engine->get->logicalDevice->bindBufferMemory(buffer.buffer, buffer.memory, 0);
	}
}