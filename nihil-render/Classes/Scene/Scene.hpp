#pragma once
#define GLM_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#include <glm/glm.hpp>
#include <vector>

namespace nihil::graphics {
	class Scene
	{
	public:
		std::vector<glm::vec3> triangleVertices;
		Scene()
		{
			for (float x = -1.0f; x < 1.0f; x += 0.2f)
			{
				for (float y = -1.0f; y < 1.0f; y += 0.2f)
				{
					triangleVertices.push_back(glm::vec3(x, y, 0.0f));
				}
			}
		}
	};
}