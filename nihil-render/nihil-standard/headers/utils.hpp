#pragma once

#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <string>

namespace nihil::nstd {
	bool isInteger(float x);
	bool isInteger(double x);
	bool isInteger(int x);

	std::vector<float> flattenMatrix4x4(glm::mat4 matrix);

	template<typename T>
	std::vector<T> appendVector(std::vector<T>& appendant, std::vector<T>& append);

	std::string vecFloatToString(std::vector<float>& vec);

	#include "../sources/utils.tpp"
}