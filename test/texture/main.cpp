#include "log.h"
#include "managers/logManager.h"
#include "managers/windowManager.h"
#include "texture.h"


int main()
{
    SpaceEngine::Managers::LogManager logManager{};
    SpaceEngine::Managers::Window winManager{};
    SpaceEngine::TextureManager texManager{};
    logManager.Initialize();
    winManager.Initialize();
    texManager.Initialize();
    SPACE_ENGINE_DEBUG("Test the textureManager");
    SPACE_ENGINE_DEBUG("Test the opening image: nebula");
    SPACE_ENGINE_ASSERT(SpaceEngine::TextureManager::load(TEXTURES_PATH "/Nebula.png"), "null pointer");
    SPACE_ENGINE_ASSERT(SpaceEngine::TextureManager::findTexture("Nebula.png"), "Texture not found");
    SPACE_ENGINE_INFO("Test done");
    texManager.Shutdown();
    winManager.Shutdown();
    logManager.Shutdown();

    return 0;
}