#pragma once
#define GLM_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#include <glm/glm.hpp>

#ifdef BIG_PUSH_CONSTANTS
namespace nihil::graphics {
	//pack the data
#pragma pack(push, 1)
	struct ObjectData {
		glm::mat4 pre;
		glm::mat4 proj;
		glm::mat4 view;
		glm::mat4 model;
	};
#pragma pack(pop)
}
#else
namespace nihil::graphics {
	//pack the data
#pragma pack(push, 1)
	struct ObjectData {
		glm::mat4 pre;
		glm::mat4 proj;
	};
#pragma pack(pop)
}
#endif