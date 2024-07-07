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
			glm::vec3 rotationDiff = _rotation - rotation;
			float rotX = rotationDiff.x;
			float rotY = rotationDiff.y;
			float rotZ = rotationDiff.z;

			renderingData = glm::rotate(renderingData, rotX, glm::vec3(1.0f, 0.0f, 0.0f));
			renderingData = glm::rotate(renderingData, rotY, glm::vec3(0.0f, 1.0f, 0.0f));
			renderingData = glm::rotate(renderingData, rotZ, glm::vec3(0.0f, 0.0f, 1.0f));

			std::cout << rotationDiff.x << " " << rotationDiff.y << " " << rotationDiff.z << std::endl;

			rotation = _rotation;
		}
	};
}