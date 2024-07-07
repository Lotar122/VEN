#pragma once

#include "nihil-standard/nstd.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vulkan/vulkan.hpp>
#include "Classes/Buffer/Buffer.hpp"

namespace nihil::graphics {
	class Engine;

	class Model 
	{
	public:
		std::vector<float> vertices;
		std::vector<uint32_t> indices;

		glm::mat4 deafultTransform;

		Engine* engine;

		std::string name = "";
		uint32_t deafultPipeline = 0;
		uint32_t instancedPipeline = 0;
		uint32_t index = 0;

		nstd::OBJ obj;

		Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>* vBuffer;
		Buffer<uint32_t, vk::BufferUsageFlagBits::eIndexBuffer>* iBuffer;

		Model(Engine* _engine, glm::mat4 _deafultTransform = glm::mat4(1.0f));

		Model(Engine* _engine, const std::string path, glm::mat4 _deafultTransform = glm::mat4(1.0f));

		void Load(const std::string path);

		inline void setInstancedPipeline(uint32_t _pipeline) { instancedPipeline = _pipeline; }
		inline void setDeafultPipeline(uint32_t _pipeline) { deafultPipeline = _pipeline; }
	};
}