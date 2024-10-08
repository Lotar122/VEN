#pragma once
#include "nihil-render/nihil.hpp"

namespace nihil::engine {
	class Object {
	public:
		graphics::Model* model;
		glm::mat4 renderingData = glm::mat4(1.0f);
		glm::vec3 rotation = glm::vec3(0.0f);
		glm::vec3 position = glm::vec3(0.0f);

		void setPosition(glm::vec3 _position)
		{
			glm::vec3 positionDiff = _position - position;
			renderingData = glm::translate(renderingData, positionDiff);

			std::cout << positionDiff.x << " " << positionDiff.y << " " << positionDiff.z << std::endl;

			position = _position;
		}
		void setRotation(glm::vec3 _rotation)
		{
			//glm::vec3 rotationDiff = _rotation - rotation;
			float rotX = _rotation.x - rotation.x;
			float rotY = _rotation.y - rotation.y;
			float rotZ = _rotation.z - rotation.z;

			renderingData = glm::rotate(renderingData, glm::radians(rotX), glm::vec3(1.0f, 0.0f, 0.0f));
			renderingData = glm::rotate(renderingData, glm::radians(rotY), glm::vec3(0.0f, 1.0f, 0.0f));
			renderingData = glm::rotate(renderingData, glm::radians(rotZ), glm::vec3(0.0f, 0.0f, 1.0f));

			std::cout << rotX << " " << rotY << " " << rotZ << std::endl;

			rotation = _rotation;
		}
		void rotate(glm::vec3 _rotateBy)
		{
			rotation.x += _rotateBy.x;
			rotation.y += _rotateBy.y;
			rotation.z += _rotateBy.z;

			renderingData = glm::rotate(renderingData, glm::radians(_rotateBy.x), glm::vec3(1.0f, 0.0f, 0.0f));
			renderingData = glm::rotate(renderingData, glm::radians(_rotateBy.y), glm::vec3(0.0f, 1.0f, 0.0f));
			renderingData = glm::rotate(renderingData, glm::radians(_rotateBy.z), glm::vec3(0.0f, 0.0f, 1.0f));
		}
	};
}