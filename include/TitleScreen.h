#pragma once
#include <vector>
#include "scene.h"

namespace SpaceEngine{

    enum class TitleResult {
        NONE,           // Nessuna azione (l'utente sta solo muovendo il mouse)
        PLAY,           // Ha cliccato "Nuova Partita"
        OPTIONS,        // Ha cliccato "Opzioni"
        LEADERBOARD,    // Ha cliccato "Classifica"
        EXIT            // Ha cliccato "Esci"
    };

    // --- Struttura del bottone (che usiamo internamente) ---
    struct MenuButton {
        float x, y;
        float w, h;
        unsigned int textureID;
        // unsigned int textureHoverID, se vogliamo cambiare texture al passaggio del mouse
        TitleResult action;
        bool isHovered;
    };

    class TitleScreen : public Scene
    {
    public:
        TitleScreen(PhysicsManager* pPhyManager);
        ~TitleScreen();

        TitleResult getInput();
        
    private:
        void Init();
        bool StartNewGame(); //bool per avere conferma che la funzione sia stata lanciata correttamente
        bool OpenOptions();
        bool OpenLeaderboard();
        bool ExitGame();
        
        //unsigned int texStart, texOptions, texLeaderboard, texExit, texLogo;
        //std::vector<MenuButton> m_buttons;
    };
}