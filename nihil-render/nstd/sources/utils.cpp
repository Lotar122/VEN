#include "../headers/utils.hpp"

namespace nihil::nstd {
	bool isInteger(float x)
	{
		return std::floor(x) == x;
	}
	bool isInteger(double x)
	{
		return std::floor(x) == x;
	}
	bool isInteger(int x)
	{
		return true;
	}

	std::vector<float> flattenMatrix4x4(glm::mat4 matrix)
	{
		std::vector<float> res(16);
		uint8_t c = 0;
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				res[c] = matrix[i][j];
				c++;
			}
		}

		return res;
	}

	std::string vecFloatToString(std::vector<float>& vec)
	{
		std::string res;
		for (int i = 0; i < vec.size(); i++)
		{
			res += '/n';
			res += std::to_string(vec[i]);
		}

		return res;
	}
}