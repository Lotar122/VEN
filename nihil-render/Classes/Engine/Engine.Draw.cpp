#include "Engine.hpp"

static void deleteInstancedDrawData(void* x)
{
	std::cout << "instanced draw deleting" << std::endl;
	delete (nihil::graphics::InstancedDrawData*)x;
}
static void deleteBufferDrawData(void* x)
{
	std::cout << "buffer draw deleting" << std::endl;
	delete (nihil::graphics::BufferDrawData*)x;
}
static void deleteModelDrawData(void* x)
{
	std::cout << "model draw deleting" << std::endl;
	delete (nihil::graphics::ModelDrawData*)x;
}

namespace nihil::graphics
{
	void Engine::queueInstancedDraw(
		Model* model,
		Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>* instanceBuffer,
		uint32_t pipeline
	)
	{
		InstancedDrawData* data = commandDataManager.allocate<InstancedDrawData>();
		*data = InstancedDrawData();
		std::cout << "DataP: " << data << std::endl;
		data->model = model;
		data->instanceBuffer = instanceBuffer;

		DrawCommand drawCommand = {};
		drawCommand.commandType = CommandType::DrawInstanced;
		drawCommand.data = (void*)data;
		drawCommand.targetPipeline = pipeline;

		commandQueue.push_back(drawCommand);
	}

	void Engine::queueBufferDraw(
		Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>* vertexBuffer,
		Buffer<uint32_t, vk::BufferUsageFlagBits::eIndexBuffer>* indexBuffer,
		uint32_t pipeline
	)
	{
		BufferDrawData* data = commandDataManager.allocate<BufferDrawData>();
		*data = BufferDrawData();
		data->indexBuffer = indexBuffer;
		data->vertexBuffer = vertexBuffer;

		DrawCommand drawCommand = {};
		drawCommand.commandType = CommandType::DrawBuffer;
		drawCommand.data = (void*)data;
		drawCommand.targetPipeline = pipeline;

		commandQueue.push_back(drawCommand);
	}

	void Engine::queueModelDraw(
		Model* model,
		uint32_t pipeline
	)
	{
		ModelDrawData* data = commandDataManager.allocate<ModelDrawData>();
		*data = ModelDrawData();
		data->model = model;

		DrawCommand drawCommand = {};
		drawCommand.commandType = CommandType::DrawModel;
		drawCommand.data = (void*)data;
		drawCommand.targetPipeline = pipeline;

		commandQueue.push_back(drawCommand);
	}
}