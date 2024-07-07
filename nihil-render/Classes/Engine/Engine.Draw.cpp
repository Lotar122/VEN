#include "Engine.hpp"

void deleteInstancedDrawData(void* x)
{
	delete (nihil::graphics::InstancedDrawData*)x;
}
void deleteBufferDrawData(void* x)
{
	delete (nihil::graphics::BufferDrawData*)x;
}
void deleteModelDrawData(void* x)
{
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
		nstd::PtrManagerDeleteFunction deleteFN = deleteInstancedDrawData;
		commandDataManager.registerType(typeid(InstancedDrawData*), deleteFN);

		InstancedDrawData* data = new InstancedDrawData();
		data->model = model;
		data->instanceBuffer = instanceBuffer;

		DrawCommand drawCommand = {};
		drawCommand.commandType = CommandType::DrawInstanced;
		drawCommand.data = (void*)data;
		drawCommand.targetPipeline = pipeline;

		commandQueue.push_back(drawCommand);

		commandDataManager.addPointer(typeid(InstancedDrawData*), (void*)data);
	}

	void Engine::queueBufferDraw(
		Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>* vertexBuffer,
		Buffer<uint32_t, vk::BufferUsageFlagBits::eIndexBuffer>* indexBuffer,
		uint32_t pipeline
	)
	{
		nstd::PtrManagerDeleteFunction deleteFN = deleteBufferDrawData;
		commandDataManager.registerType(typeid(BufferDrawData*), deleteFN);

		BufferDrawData* data = new BufferDrawData();
		data->indexBuffer = indexBuffer;
		data->vertexBuffer = vertexBuffer;

		DrawCommand drawCommand = {};
		drawCommand.commandType = CommandType::DrawBuffer;
		drawCommand.data = (void*)data;
		drawCommand.targetPipeline = pipeline;

		commandQueue.push_back(drawCommand);

		commandDataManager.addPointer(typeid(BufferDrawData*), (void*)data);
	}

	void Engine::queueModelDraw(
		Model* model,
		uint32_t pipeline
	)
	{
		nstd::PtrManagerDeleteFunction deleteFN = deleteModelDrawData;
		commandDataManager.registerType(typeid(ModelDrawData*), deleteFN);

		ModelDrawData* data = new ModelDrawData();
		data->model = model;

		DrawCommand drawCommand = {};
		drawCommand.commandType = CommandType::DrawModel;
		drawCommand.data = (void*)data;
		drawCommand.targetPipeline = pipeline;

		commandQueue.push_back(drawCommand);

		commandDataManager.addPointer(typeid(ModelDrawData*), (void*)data);
	}
}