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
		Camera(glm::vec3 _position, glm::vec3 _target, glm::vec3 _up)
		{
			yaw = -90.0f;
			pitch = 0.0f;

			position = _position;
			worldUp = _up;

			front = glm::normalize(_target - position);
			right = glm::normalize(glm::cross(right, worldUp));
			upVector = glm::normalize(glm::cross(right, front));
		}

		void setPerspectiveProjection(float fovy, float aspect, float near, float far);
		void setPerspectiveProjection(float fovy, float near, float far, Engine* engine);
		void setOrthographicProjection(float left, float right, float top, float bottom, float near, float far);
		void setOrthographicProjection(float aspect, float top, float bottom, float near, float far);
		void setOrthographicProjection(float top, float bottom, float near, float far, Engine* engine);

		inline void move(glm::vec3 _moveBy) { position += _moveBy; }
		inline void moveTo(glm::vec3 _moveTo) { position = _moveTo; }

		void rotate(float _pitch, float _yaw);
		void setRotation(float _pitch, float _yaw);

		inline glm::mat4 getProjectionMatrix() const { return projectionMatrix; }
		inline glm::mat4 getViewMatrix() const { return glm::lookAtLH(position, position + front, upVector); }
	private:
		glm::mat4 projectionMatrix = glm::mat4(1.0f);
		glm::mat4 viewMatrix = glm::mat4(1.0f);

		glm::vec3 position;
		glm::vec3 front;
		glm::vec3 upVector;
		glm::vec3 right;
		glm::vec3 worldUp;

		float yaw, pitch;

		inline void clampPitch()
		{
			if (pitch > 89.0f) pitch = 89.0f;
			if (pitch < -89.0f) pitch = -89.0f;
		}

		void updateCameraVectors();
	};
}