#pragma once
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <spirv_cross/spirv_glsl.hpp>

//!!! move to nstd
namespace nihil::graphics {
	//the class (static) that provides functions to work with the SPIR-V format
	class SPIRV
	{
	public:
		//loads the contents of a .spv file to a vector of chars
		static std::vector<char> LoadSPVFile(std::string filepath);
		static std::vector<char> CompileGLSLFile(std::string filepath);
	};

}