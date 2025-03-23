# CMakeList.txt : CMake project for nihil-render-cmake, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.10)

project ("VEN")

# Manually set the glfw3 installed directory. Only on windows. Couldn't get it to work another way.
if(WIN32) 
	set(glfw3_DIR "./vcpkg_installed/x64-windows-static/share/glfw3/") 
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
	# do not define the macro
    # target_compile_definitions(my_executable PRIVATE ENABLE_LOGGING)
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(my_executable PRIVATE ENABLE_LOGGING)
endif()

# Find Vulkan and GLFW
find_package(Vulkan REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

file(GLOB_RECURSE SOURCES 
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_executable(VEN ${SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

target_include_directories(VEN 
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/includes
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/src
	${CMAKE_CURRENT_SOURCE_DIR}/src/Graphics
	${CMAKE_CURRENT_SOURCE_DIR}/src/Engine
	${CMAKE_CURRENT_SOURCE_DIR}/src/Standard
	${CMAKE_CURRENT_SOURCE_DIR}/src/Core
)

#link_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/lib)
# link_directories(${CMAKE_CURRENT_SOURCE_DIR}/nstd)

target_link_libraries(VEN PUBLIC
	Vulkan::Vulkan
	glfw
)

target_compile_features(VEN PRIVATE cxx_std_20)
