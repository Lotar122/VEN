#pragma once

#include "Buffer.hpp"
#include "Classes/Engine/Engine.hpp"

#include <type_traits>

namespace nihil::graphics {
	template<typename T, vk::BufferUsageFlagBits usageT>
	Buffer<T, usageT>::Buffer(Engine* _engine, std::vector<T>& _Data, vk::BufferUsageFlagBits _usage)
	{
		engine = _engine;
		usage = _usage;
		Data = _Data;

		if ((int)usageT != 0) usage = usageT;
		buffer = Memory::CreateBuffer(sizeof(T) * Data.size(), usage, engine);
		void* memoryLocation = engine->get->logicalDevice->mapMemory(buffer.memory, 0, sizeof(T) * Data.size());
		memcpy(memoryLocation, Data.data(), sizeof(T) * Data.size());
		engine->get->logicalDevice->unmapMemory(buffer.memory);
	}
	template<typename T, vk::BufferUsageFlagBits usageT>
	Buffer<T, usageT>::~Buffer()
	{
		engine->get->logicalDevice->waitIdle();
		engine->get->logicalDevice->destroyBuffer(buffer.buffer);
		engine->get->logicalDevice->freeMemory(buffer.memory);
	}

	template<typename T, vk::BufferUsageFlagBits usageT>
	void Buffer<T, usageT>::refresh(std::vector<T> _Data)
	{
		bufferSize = static_cast<uint32_t>(_Data.size() * sizeof(T));
		Data = _Data;
		buffer = Memory::CreateBuffer(bufferSize, usage, engine);

		void* data;
		engine->get->logicalDevice->mapMemory(buffer.memory, 0, bufferSize, {}, &data);

		memcpy(data, _Data.data(), static_cast<size_t>(bufferSize));

		engine->get->logicalDevice->unmapMemory(buffer.memory);
	}
}