#include "Camera.hpp"
#define GLM_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Classes/Engine/Engine.hpp"

using namespace nihil::graphics;

void Camera::setOrthographicProjection(float left, float right, float top, float bottom, float near, float far)
{
	projectionMatrix = glm::ortho(left, right, top, bottom, near, far);
}

void Camera::setOrthographicProjection(float aspect, float top, float bottom, float near, float far)
{
	assert(glm::abs(aspect - std::numeric_limits<float>::epsilon() > 0.0f));
	projectionMatrix = glm::ortho(-aspect, aspect, top, bottom, near, far);
}

void Camera::setOrthographicProjection(float top, float bottom, float near, float far, Engine* engine)
{
	float aspect = engine->getAspectRatio();
	assert(glm::abs(aspect - std::numeric_limits<float>::epsilon() > 0.0f));
	projectionMatrix = glm::ortho(-aspect, aspect, top, bottom, near, far);
}

void Camera::setPerspectiveProjection(float fovy, float aspect, float near, float far)
{
	assert(glm::abs(aspect - std::numeric_limits<float>::epsilon() > 0.0f));
	projectionMatrix = glm::perspectiveLH(glm::radians(fovy), 1.0f, near, far);
}

void Camera::setPerspectiveProjection(float fovy, float near, float far, Engine* engine)
{
	float aspect = engine->getAspectRatio();
	assert(glm::abs(aspect - std::numeric_limits<float>::epsilon() > 0.0f));
	projectionMatrix = glm::perspectiveLH(glm::radians(fovy), 1.0f, near, far);
}

void Camera::updateCameraVectors()
{
	glm::vec3 _front;
	_front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	_front.y = sin(glm::radians(pitch));
	_front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	front = glm::normalize(_front);

	right = glm::normalize(glm::cross(front, worldUp));
	upVector = glm::normalize(glm::cross(right, front));
}

void Camera::rotate(float _pitch, float _yaw)
{
	yaw += _yaw;
	pitch = _pitch;
	clampPitch();
	updateCameraVectors();
}

void Camera::setRotation(float _pitch, float _yaw)
{
	yaw = _yaw;
	pitch = _pitch;
	clampPitch();
	updateCameraVectors();
}