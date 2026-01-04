#Logmanager
add_library(LogManager STATIC logManager.cpp)
target_include_directories(LogManager PRIVATE ${CMAKE_SOURCE_DIR}/include/managers)
target_link_libraries(LogManager PUBLIC spdlog
                        PUBLIC glad_gl_core_33)
set_target_properties(LogManager PROPERTIES FOLDER "LogManager")
#macros
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message("Macro SPACE_ENGINE_DEBUG defined")
    target_compile_definitions(LogManager PUBLIC SPACE_ENGINE_DEBUG_CONFIG=1)

if(WIN32)
    message("Macro SPACE_ENGINE_PLATFORM_WINDOWS defined")
    target_compile_definitions(LogManager PUBLIC SPACE_ENGINE_PLATFORM_WINDOWS=1)
endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    message("Macro SPACE_ENGINE_DEBUG defined")
    target_compile_definitions(LogManager PUBLIC SPACE_ENGINE_RELEASE_CONFIG=1)
endif()
#MACRO for the resouces
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(LogManager PUBLIC RESOURCE_PATH="${CMAKE_SOURCE_DIR}/res/")
    target_compile_definitions(LogManager PUBLIC SHADERS_PATH="${CMAKE_SOURCE_DIR}/assets/shaders/")
    target_compile_definitions(LogManager PUBLIC MESHES_PATH="${CMAKE_SOURCE_DIR}/assets/meshes/")
    target_compile_definitions(LogManager PUBLIC TEXTURES_PATH="${CMAKE_SOURCE_DIR}/assets/textures/")
    target_compile_definitions(LogManager PUBLIC FONTS_PATH="${CMAKE_SOURCE_DIR}/assets/fonts/")
    target_compile_definitions(LogManager PUBLIC AUDIO_PATH="${CMAKE_SOURCE_DIR}/assets/audio/")
    target_compile_definitions(LogManager PUBLIC PRODUCTION_BUILD=0)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(LogManager PUBLIC RESOURCE_PATH="./assets/")
    target_compile_definitions(LogManager PUBLIC SHADERS_PATH="./assets/shaders/")
    target_compile_definitions(LogManager PUBLIC MESHES_PATH="./assets/meshes/")
    target_compile_definitions(LogManager PUBLIC TEXTURES_PATH="./assets/textures/")
    target_compile_definitions(LogManager PUBLIC FONTS_PATH="./assets/fonts/")
    target_compile_definitions(LogManager PUBLIC AUDIO_PATH="./assets/audio/")
    target_compile_definitions(LogManager PUBLIC PRODUCTION_BUILD=1)
endif()

#WindowManager
add_library(WindowManager STATIC windowManager.cpp)
target_include_directories(WindowManager PRIVATE ${CMAKE_SOURCE_DIR}/include/managers)
target_link_libraries(WindowManager PUBLIC glfw
    PUBLIC glad_gl_core_33
    PRIVATE OpenGL::GL
    PUBLIC Utils
    PRIVATE LogManager
    PUBLIC AudioManager)
set_target_properties(WindowManager PROPERTIES FOLDER "WindowManager")

#InputManager
add_library(InputManager STATIC inputManager.cpp)
target_include_directories(InputManager PRIVATE ${CMAKE_SOURCE_DIR}/include/managers)
target_link_libraries(InputManager PRIVATE WindowManager
    PRIVATE LogManager)
set_target_properties(InputManager PROPERTIES FOLDER "InputManager")

#AudioManager
add_library(AudioManager STATIC audioManager.cpp)
target_include_directories(AudioManager PUBLIC 
    ${CMAKE_SOURCE_DIR}/include/managers
    ${openal_SOURCE_DIR}/include
)
target_link_libraries(AudioManager 
    PUBLIC OpenAL::OpenAL
    PUBLIC LogManager)
set_target_properties(AudioManager PROPERTIES FOLDER "AudioManager")
#SceneManager
add_library(SceneManager STATIC sceneManager.cpp ../scene.cpp)
target_include_directories(SceneManager PRIVATE ${CMAKE_SOURCE_DIR}/include/managers
                                        PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(SceneManager PRIVATE LogManager
                                    PRIVATE Mesh
                                    PRIVATE UI
                                    PUBLIC AudioManager
                                    )
set_target_properties(SceneManager PROPERTIES FOLDER "SceneManager")