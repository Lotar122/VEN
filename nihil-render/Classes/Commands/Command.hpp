#pragma once
#include <iostream>

namespace nihil::graphics 
{
	class Model;
	enum class CommandType {
		DrawBuffer,
		DrawInstanced,
		DrawModel
	};
	struct DrawCommand {
		CommandType commandType;
		void* data;
		uint32_t targetPipeline;
	};
	
	struct InstancedDrawData {
		Model* model;
		Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>* instanceBuffer;
	};
	struct BufferDrawData {
		Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>* vertexBuffer;
		Buffer<uint32_t, vk::BufferUsageFlagBits::eIndexBuffer>* indexBuffer;
	};
	struct ModelDrawData {
		Model* model;
	};
}