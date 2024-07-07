#include "Model.hpp"
#include "Classes/Engine/Engine.hpp"
#include "nihil-standard/nstd.hpp"

namespace nihil::graphics {
	Model::Model(Engine* _engine, glm::mat4 _deafultTransform)
	{
		engine = _engine;
		deafultTransform = _deafultTransform;
		name = "./resources/models/cube.obj";

		obj.Load("./resources/models/cube.obj", nstd::LoadBinObj::DontCare, engine->app->screenRatio);

		vertices = obj.verticesRender;
		indices = obj.indicesRender;

		vBuffer = new Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>(engine, vertices);
		iBuffer = new Buffer<uint32_t, vk::BufferUsageFlagBits::eIndexBuffer>(engine, indices);
	}
	Model::Model(Engine* _engine, const std::string path, glm::mat4 _deafultTransform)
	{
		engine = _engine;
		deafultTransform = _deafultTransform;
		name = path;

		std::cout << &engine->app->screenRatio << std::endl;
		obj.Load(path, nstd::LoadBinObj::DontCare, engine->app->screenRatio);

		vertices = obj.verticesRender;
		indices = obj.indicesRender;

		vBuffer = new Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>(engine, vertices);
		iBuffer = new Buffer<uint32_t, vk::BufferUsageFlagBits::eIndexBuffer>(engine, indices);
	}

	//, Buffer<InstanceData, vk::BufferUsageFlagBits::eVertexBuffer>* _instanceBuffer

	void Model::Load(const std::string path)
	{
		obj.Load(path, nstd::LoadBinObj::DontCare, engine->app->screenRatio);

		vertices = obj.verticesRender;
		indices = obj.indicesRender;
		name = path;

		delete vBuffer;
		delete iBuffer;

		vBuffer = new Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>(engine, vertices);
		iBuffer = new Buffer<uint32_t, vk::BufferUsageFlagBits::eIndexBuffer>(engine, indices);
	}
}