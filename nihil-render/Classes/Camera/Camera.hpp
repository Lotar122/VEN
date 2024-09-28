#pragma once

#define GLM_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace nihil::graphics
{
	class Engine;

	class Camera
	{
	public:
		void setPerspectiveProjection(float fovy, float aspect, float near, float far);
		void setPerspectiveProjection(float fovy, float near, float far, Engine* engine);
		void setOrthographicProjection(float left, float right, float top, float bottom, float near, float far);
		void setOrthographicProjection(float aspect, float top, float bottom, float near, float far);
		void setOrthographicProjection(float top, float bottom, float near, float far, Engine* engine);
		glm::mat4 getMatrix() const { return projectionMatrix; };
	private:
		glm::mat4 projectionMatrix = glm::mat4(1.0f);
	};
}