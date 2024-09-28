# CMakeList.txt : CMake project for nihil-engine-cmake, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("nihil-engine")

file(GLOB_RECURSE SOURCES 
	"Classes/*.cpp"
)

# Add source to this project's executable.
add_executable (nihil-engine "main.cpp" ${SOURCES})

target_include_directories(nihil-engine
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/nihil-render/thirdparty/includes
	${CMAKE_CURRENT_SOURCE_DIR}/nihil-render
)

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/nihil-render/thirdparty/lib)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/nihil-render/nstd)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/nihil-render)

target_link_libraries(nihil-engine PUBLIC
	# couldn't get this to work another way
	${CMAKE_CURRENT_SOURCE_DIR}/nihil-render/nstd/nstd.lib
	${CMAKE_CURRENT_SOURCE_DIR}/nihil-render/thirdparty/lib/vulkan-1.lib
	${CMAKE_CURRENT_SOURCE_DIR}/nihil-render/thirdparty/lib/glfw3_mt.lib
	${CMAKE_CURRENT_SOURCE_DIR}/nihil-render/nihil-render.lib
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET nihil-engine PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
