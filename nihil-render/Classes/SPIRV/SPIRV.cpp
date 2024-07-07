#include "SPIRV.hpp"

using namespace nihil::graphics;

std::vector<char> SPIRV::LoadSPVFile(std::string filepath)
{
	std::ifstream file{ filepath, std::ios::ate | std::ios::binary };
	if (!file.is_open())
	{
		return std::vector<char>(0);
	}

	size_t filesize = static_cast<size_t>(file.tellg());

	std::vector<char> buffer(filesize);
	file.seekg(0);
	file.read(buffer.data(), filesize);
	file.close();
	return buffer;
}

std::vector<char> SPIRV::CompileGLSLFile(std::string filepath)
{
	return std::vector<char>(0);
}