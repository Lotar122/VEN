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
	projectionMatrix = glm::perspectiveRH(fovy, aspect, near, far);
}

void Camera::setPerspectiveProjection(float fovy, float near, float far, Engine* engine)
{
	float aspect = engine->getAspectRatio();
	assert(glm::abs(aspect - std::numeric_limits<float>::epsilon() > 0.0f));
	projectionMatrix = glm::perspectiveRH(fovy, aspect, near, far);
}