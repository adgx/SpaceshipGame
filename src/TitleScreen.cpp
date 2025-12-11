#include "titleScreen.h"
#include "inputManager.h"
#include "windowManager.h"
#include "log.h"


// Helper to load an image file and create an OpenGL texture. Returns 0 on failure.
static unsigned int loadTextureFromFile(const char* path) {
    int width = 0, height = 0, channels = 4;
    unsigned char* data = stbi_load(path, &width, &height, &channels, 0);
    if (!data) {
        SPACE_ENGINE_ERROR("Failed to load texture: {}", path);
        return 0;
    }        
    GLenum format = GL_RGB;
    unsigned int tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

    //glTexParameteri(GL_TEXTURE_2D, ...); // Set texture parameters as needed

    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);
    return tex;
}

namespace SpaceEngine{

    TitleScreen::TitleScreen(){
        SPACE_ENGINE_DEBUG("Title Screen opening...");
    }

    TitleScreen::~TitleScreen(){
        if (texStart)      glDeleteTextures(1, &texStart);
        if (texOptions)    glDeleteTextures(1, &texOptions);
        if (texLeaderboard)glDeleteTextures(1, &texLeaderboard);
        if (texExit)       glDeleteTextures(1, &texExit);
        if (texLogo)       glDeleteTextures(1, &texLogo);
        SPACE_ENGINE_DEBUG("Going from Title Screen to another scene...");
    }

    void TitleScreen::Init(){
        SPACE_ENGINE_DEBUG("Loading textures...");

        texStart      = loadTextureFromFile("assets/textures/Startbtn.png");
        texOptions    = loadTextureFromFile("assets/textures/Optionsbtn.png");
        texLeaderboard= loadTextureFromFile("assets/textures/Leaderboardbtn.png");
        texExit       = loadTextureFromFile("assets/textures/Exitbtn.png");
        texLogo       = loadTextureFromFile("assets/textures/Logo.png");

        float btnW = 200.0f; //TODO: cambiare in base alla dimensione dei bottoni
        float btnH = 50.0f;

        float logoW = 400.0f; //TODO: cambiare in base alla dimensione del logo
        float logoH = 100.0f;

        m_buttons.clear();

        float posX = (800.0f - btnW) / 2.0f;
                                            //TODO: cambiare 800 con la larghezza della finestra
        logoX = (800.0f - logoW) / 2.0f;
        logoY = 50.0f;

        m_buttons.push_back({ posX, 200.0f, btnW, btnH, texStart,      TitleResult::PLAY,        false });
        m_buttons.push_back({ posX, 270.0f, btnW, btnH, texOptions,    TitleResult::OPTIONS,     false });
        m_buttons.push_back({ posX, 340.0f, btnW, btnH, texLeaderboard,TitleResult::LEADERBOARD, false });
        m_buttons.push_back({ posX, 410.0f, btnW, btnH, texExit,       TitleResult::EXIT,        false });
        SPACE_ENGINE_INFO("Bottoni caricati: {}", m_buttons.size())
    }

    void TitleScreen::Render(){
        SPACE_ENGINE_DEBUG("Rendering elements...");
        //TODO
    }

    TitleResult TitleScreen::getInput(){
        SPACE_ENGINE_DEBUG("Managing input from user");
        int mx = Mouse::x; 
        int my = Mouse::y;

        for (auto& btn : m_buttons) {
        
        bool isInsideX = (mx >= btn.x && mx <= (btn.x + btn.w));
        bool isInsideY = (my >= btn.y && my <= (btn.y + btn.h));

        if (isInsideX && isInsideY) {
            btn.isHovered = true; //se vogliamo illuminare il bottone al passaggio del mouse
            if(Mouse::buttonDown(0)){
                switch (btn.action) {
                    case TitleResult::PLAY:
                        SPACE_ENGINE_DEBUG("New Game starting...");
                        return TitleResult::PLAY;
                        break;

                    case TitleResult::OPTIONS:
                        SPACE_ENGINE_DEBUG("Opening options");
                        return TitleResult::OPTIONS;
                        break;

                    case TitleResult::LEADERBOARD:
                        SPACE_ENGINE_DEBUG("Opening Leaderboard");
                        return TitleResult::LEADERBOARD;
                        break;

                    case TitleResult::EXIT:
                        SPACE_ENGINE_DEBUG("Quitting Game");
                        return TitleResult::EXIT;
                        break;
                        
                    default:
                        break;
                }
            }    
        }else {
            btn.isHovered = false;
        }
        return TitleResult::NONE;
    }  
}
}