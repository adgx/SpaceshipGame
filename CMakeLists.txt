cmake_minimum_required(VERSION 3.10.0)
project(SpaceshipGame LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(OSBitness 32)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(OSBitness 64)
endif()
#Require C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
set(FETCHCONTENT_BASE_DIR "${PROJECT_SOURCE_DIR}/extern") 

#set on when you want to release the project 
set(PRODUCTION_BUILD OFF CACHE BOOL "Production build" FORCE)

if(PRODUCTION_BUILD)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)
endif()

if(MSVC)
    add_compile_options(/arch:AVX2)
endif()

set(OutputDir "${CMAKE_SOURCE_DIR}/bin/${CMAKE_SYSTEM_NAME}${OSBitness}/${CMAKE_BUILD_TYPE}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${OutputDir}/static_libs")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${OutputDir}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${OutputDir}")

#use the --config Debug option for select the configuration  
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

#modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(FindDirsCmakeFile)

#Fetch GLFW
Fetchcontent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw
    GIT_TAG 3.4
)
FetchContent_MakeAvailable(glfw)

#Fetch GLAD
FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad
    GIT_TAG v2.0.8
)
FetchContent_MakeAvailable(glad)

#Fetch glm
FetchContent_Declare(
    glm 
    GIT_REPOSITORY https://github.com/g-truc/glm
    GIT_TAG 1.0.2
)
FetchContent_MakeAvailable(glm)

#Fetch spdlog
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog
    GIT_TAG v1.16.0
)
FetchContent_MakeAvailable(spdlog)

#Fetch assimp
FetchContent_Declare(
    assimp 
    GIT_REPOSITORY https://github.com/assimp/assimp
    GIT_TAG v6.0.2
)
FetchContent_MakeAvailable(assimp)

#Fetch FreeType
FetchContent_Declare(
    freetype 
    GIT_REPOSITORY https://gitlab.freedesktop.org/freetype/freetype
    GIT_TAG VER-2-14-1
)

#Fetch OpenAL
FetchContent_Declare(
    openal 
    GIT_REPOSITORY https://github.com/kcat/openal-soft
    GIT_TAG 1.24.3
)

add_subdirectory("${glad_SOURCE_DIR}/cmake" glad_cmake)
glad_add_library(glad_gl_core_33 REPRODUCIBLE LOADER API gl:core=3.3)

find_package(OpenGL REQUIRED)

#include headers project
include_directories("${PROJECT_SOURCE_DIR}/include/")
#include headers externs
#Glad headers
include_directories("${PROJECT_SOURCE_DIR}/build/gladsources/glad_gl_core_33/include")
#GLFW headers
include_directories("${PROJECT_SOURCE_DIR}/extern/glfw-src/include")
#spdlog headers
include_directories("${PROJECT_SOURCE_DIR}/extern/spdlog-src/include")

# Add subdirectories
add_subdirectory("extern")
add_subdirectory("src")
add_subdirectory("test")
