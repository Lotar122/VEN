﻿# CMakeList.txt : CMake project for nihil-render-cmake, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.10)

project ("VEN")

# Find Vulkan and GLFW
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

file(GLOB_RECURSE SOURCES 
	"Classes/*.cpp"
)

add_executable(VEN ${SOURCES} "main.cpp")

target_include_directories(VEN 
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/includes
	${CMAKE_CURRENT_SOURCE_DIR}
)

#link_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/lib)
# link_directories(${CMAKE_CURRENT_SOURCE_DIR}/nstd)

target_link_libraries(VEN PUBLIC
	/usr/lib/libvulkan.so
	glfw
)

target_compile_features(VEN PRIVATE cxx_std_20)
