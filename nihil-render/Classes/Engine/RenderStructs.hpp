#pragma once
#define GLM_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#include <glm/glm.hpp>

//this allows for push constants which are 256B
//without it the engine will deafult to 128B push constants, which dont really work. but are needed for weaker gpus
#define BIG_PUSH_CONSTANTS

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