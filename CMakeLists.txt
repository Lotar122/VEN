cmake_minimum_required (VERSION 3.10)

project ("VEN")

# Manually set the glfw3 installed directory. Only on windows. Couldn't get it to work another way.
if(WIN32) 
	set(glfw3_DIR "${CMAKE_SOURCE_DIR}/vcpkg_installed/x64-windows-static/share/glfw3/") 
endif()

set(CMAKE_CONFIGURATION_TYPES "Debug;Release;NativeRelease" CACHE STRING "" FORCE)

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

if(MSVC)
    # Basic optimization flags for MSVC
    set(MSVC_FLAGS "/O2 /GL /LTCG /EHsc /MD /Gw /Gy /Oi /Ot /Zc:inline /fp:fast")

    # Detect architecture
    # CMAKE_SYSTEM_PROCESSOR is 'AMD64', 'ARM64', etc.
    # CMAKE_VS_PLATFORM_NAME is 'x64', 'Win32', 'ARM64', etc. (Visual Studio only)
    string(TOUPPER "${CMAKE_SYSTEM_PROCESSOR}" SYSTEM_ARCH)
    message(STATUS "MSVC target architecture: ${SYSTEM_ARCH}")

    set(MSVC_ARCH_FLAGS "")

    if(SYSTEM_ARCH MATCHES "AMD64" OR SYSTEM_ARCH MATCHES "X64")
        set(MSVC_ARCH_FLAGS "/arch:AVX2")
    elseif(SYSTEM_ARCH MATCHES "ARM64")
        message(STATUS "MSVC ARM64 target detected — skipping /arch: flags")
    elseif(SYSTEM_ARCH MATCHES "X86" OR SYSTEM_ARCH MATCHES "WIN32")
        set(MSVC_ARCH_FLAGS "/arch:SSE2")  # SSE2 is default but explicit is fine
    else()
        message(WARNING "Unknown MSVC architecture: ${SYSTEM_ARCH}")
    endif()

    # Combine and set flags
    set(CMAKE_CXX_FLAGS_RELEASE "${MSVC_FLAGS} ${MSVC_ARCH_FLAGS}")
    set(CMAKE_C_FLAGS_RELEASE "${MSVC_FLAGS} ${MSVC_ARCH_FLAGS}")

    set(CMAKE_CXX_FLAGS_NATIVERELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
    set(CMAKE_C_FLAGS_NATIVERELEASE "${CMAKE_C_FLAGS_RELEASE}")

    set(CMAKE_EXE_LINKER_FLAGS_NATIVERELEASE "" CACHE STRING "" FORCE)
endif()

if(LINUX AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	set(GNU_FLAGS "-O2 -flto -fstrict-aliasing -fprefetch-loop-arrays -fipa-pta -pthread -ftree-vectorize -fopt-info-vec-optimized -fomit-frame-pointer -fgraphite-identity -floop-nest-optimize -fno-math-errno -ffp-contract=fast")
	set(CMAKE_CXX_FLAGS_RELEASE "${GNU_FLAGS}")
	set(CMAKE_C_FLAGS_RELEASE "${GNU_FLAGS}")

	set(CMAKE_CXX_FLAGS_NATIVERELEASE "${GNU_FLAGS} -march=native -mtune=native")
	set(CMAKE_C_FLAGS_NATIVERELEASE "${GNU_FLAGS} -march=native -mtune=native")

    set(CMAKE_EXE_LINKER_FLAGS_NATIVERELEASE "" CACHE STRING "" FORCE)
endif()

if((LINUX OR WIN32) AND (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "ClangCl"))
    set(CLANG_FLAGS "-O2 -flto=full -fstrict-aliasing -pthread -Rpass=loop-vectorize -fomit-frame-pointer -fslp-vectorize -fno-math-errno -ffp-contract=fast")
    set(CMAKE_CXX_FLAGS_RELEASE "${CLANG_FLAGS}")
    set(CMAKE_C_FLAGS_RELEASE "${CLANG_FLAGS}")

    set(CMAKE_CXX_FLAGS_NATIVERELEASE "${CLANG_FLAGS} -march=native -mtune=native")
    set(CMAKE_C_FLAGS_NATIVERELEASE "${CLANG_FLAGS} -march=native -mtune=native")

    set(CMAKE_EXE_LINKER_FLAGS_NATIVERELEASE "" CACHE STRING "" FORCE)
endif()

if(APPLE AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # Apple Clang has more restrictions and lags behind mainline Clang
    set(APPLE_CLANG_FLAGS "-O2 -flto -fstrict-aliasing -pthread -fomit-frame-pointer -fslp-vectorize -fno-math-errno -ffp-contract=fast")
    set(CMAKE_CXX_FLAGS_RELEASE "${APPLE_CLANG_FLAGS}")
    set(CMAKE_C_FLAGS_RELEASE "${APPLE_CLANG_FLAGS}")

    set(CMAKE_CXX_FLAGS_NATIVERELEASE "${APPLE_CLANG_FLAGS} -march=native -mtune=native")
    set(CMAKE_C_FLAGS_NATIVERELEASE "${APPLE_CLANG_FLAGS} -march=native -mtune=native")

    set(CMAKE_EXE_LINKER_FLAGS_NATIVERELEASE "" CACHE STRING "" FORCE)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    message("Compiler flags [VEN] are: ${CMAKE_CXX_FLAGS_RELEASE}")
elseif(CMAKE_BUILD_TYPE STREQUAL "NativeRelease")
    message("Compiler flags [VEN] are: ${CMAKE_CXX_FLAGS_NATIVERELEASE}")
endif()

# Find Vulkan and GLFW
find_package(Vulkan REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)

# Try vcpkg first (only works if toolchain is active)
find_package(unofficial-shaderc CONFIG QUIET)

# Try system CMake config
if (NOT unofficial_shaderc_FOUND)
    find_package(shaderc CONFIG QUIET)
endif()

# Try system pkg-config as final fallback
if (NOT unofficial_shaderc_FOUND AND NOT shaderc_FOUND)
    find_package(PkgConfig QUIET)
    if (PkgConfig_FOUND)
        pkg_check_modules(SHADERC QUIET shaderc)
    endif()
endif()

add_library(VEN_shaderc INTERFACE)

if (unofficial_shaderc_FOUND)
    target_link_libraries(VEN_shaderc
        INTERFACE unofficial::shaderc::shaderc
    )
elseif (shaderc_FOUND)
    target_link_libraries(VEN_shaderc
        INTERFACE shaderc
    )
elseif (SHADERC_FOUND)
    target_include_directories(VEN_shaderc
        INTERFACE ${SHADERC_INCLUDE_DIRS}
    )
    target_link_libraries(VEN_shaderc
        INTERFACE ${SHADERC_LIBRARIES}
    )
else()
    message(FATAL_ERROR
        "Shaderc not found. "
        "Install via vcpkg or system package manager."
    )
endif()

if(LINUX)
	find_path(STB_INCLUDE_DIR "stb_image.h"
        PATH_SUFFIXES stb
        PATHS /usr/include
    )
else()
	find_path(STB_INCLUDE_DIRS "stb_image.h")
endif()

message("Stb at: ${STB_INCLUDE_DIR}")
message("Using C++ compiler: ${CMAKE_CXX_COMPILER}\nUsing C compiler: ${CMAKE_C_COMPILER}")

file(GLOB_RECURSE SOURCES 
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

set(COMPILE_TARGET "Executable" CACHE STRING "Choose build target: Executable or Lib")

add_library(VEN ${SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/src/VEN.hpp")

target_include_directories(VEN 
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/includes
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/src
	${CMAKE_CURRENT_SOURCE_DIR}/src/Graphics
	${CMAKE_CURRENT_SOURCE_DIR}/src/Graphics/Vulkan
	${CMAKE_CURRENT_SOURCE_DIR}/src/Engine
	${CMAKE_CURRENT_SOURCE_DIR}/src/Standard
	${CMAKE_CURRENT_SOURCE_DIR}/src/Core
)

target_include_directories(VEN PRIVATE ${Stb_INCLUDE_DIR})

target_link_libraries(VEN PUBLIC
	Vulkan::Vulkan
	glfw
    VEN_shaderc
    OpenSSL::Crypto
)

target_compile_features(VEN PRIVATE cxx_std_20)
