#include "log.h"
#include "managers/logManager.h"
#include "shader.h"
#include "managers/windowManager.h"


int main()
{
    SpaceEngine::Managers::LogManager logManager{};
    SpaceEngine::Managers::Window winManager{};
    SpaceEngine::ShaderManager shManager{};
    logManager.Initialize();
    winManager.Initialize();
    shManager.Inizialize();
    SPACE_ENGINE_DEBUG("Test the shaderManager");
    SPACE_ENGINE_DEBUG("Test the shader: simple");
    SPACE_ENGINE_ASSERT(SpaceEngine::ShaderManager::createShaderProgram("simple"), "null pointer shader");
    SPACE_ENGINE_ASSERT(SpaceEngine::ShaderManager::findShaderProgram("simple"), "Shader not found");
    SPACE_ENGINE_DEBUG("Test the shader: simpleTex");
    SPACE_ENGINE_ASSERT(SpaceEngine::ShaderManager::createShaderProgram("simpleTex"), "null pointer shader");
    SPACE_ENGINE_ASSERT(SpaceEngine::ShaderManager::findShaderProgram("simpleTex"), "Shader not found");
    SPACE_ENGINE_INFO("Test done");
    shManager.Shutdown();
    winManager.Shutdown();
    logManager.Shutdown();

    return 0;
}