#include "utils/utils.h"
#include "managers/windowManager.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <algorithm>
#include <vector>

// Rimosso: #include <filesystem> non serve più

namespace SpaceEngine
{
    // --- Helper per le stringhe (sostituisce toWide se servisse, ma qui semplifichiamo) ---
    // Manteniamo la logica semplice basata su char standard per evitare problemi di encoding misti

    void Utils::applyRatioScreenRes(Vector2 anchor, Vector2 pos, float &outScale, Vector2 &outOffset, Vector2 &outPos)
    {
        float scaleX = WindowManager::width / static_cast<float>(REF_WIDTH);
        float scaleY = WindowManager::height / static_cast<float>(REF_HEIGHT);
        outScale = std::min(scaleX, scaleY);

        outOffset.x = (WindowManager::width - REF_WIDTH * outScale) * 0.5f;
        outOffset.y = (WindowManager::height - REF_HEIGHT * outScale) * 0.5f;

        float anchorX = anchor.x * REF_WIDTH;
        float anchorY = anchor.y * REF_HEIGHT;

        outPos.x = (anchorX + pos.x) * outScale + outOffset.x;
        outPos.y = (anchorY + pos.y) * outScale + outOffset.y;
    }

    // --- REIMPLEMENTAZIONE SENZA FILESYSTEM ---

    bool Utils::fileExists(const std::string &path)
    {
        // Metodo nativo Windows molto più rapido e sicuro di FindFirstFile + filesystem
        DWORD attrib = GetFileAttributesA(path.c_str());

        return (attrib != INVALID_FILE_ATTRIBUTES &&
                !(attrib & FILE_ATTRIBUTE_DIRECTORY));
    }

    bool Utils::directoryExists(const std::string &pathDir)
    {
        DWORD attrib = GetFileAttributesA(pathDir.c_str());

        return (attrib != INVALID_FILE_ATTRIBUTES &&
                (attrib & FILE_ATTRIBUTE_DIRECTORY));
    }

    std::string Utils::getFullPath(const std::string &dir, const aiString &path)
    {
        std::string p(path.data);

#if 0
        if (p.empty()) return "";

        // 1. Normalizza gli slash (importante per Windows/Assimp)
        std::replace(p.begin(), p.end(), '\\', '/');

        // --- STRATEGIA DI CARICAMENTO A TENTATIVI ---

        // TENTATIVO 1: Percorso originale relativo al modello
        // Esempio: assets/models/ + texture.png -> assets/models/texture.png
        std::string pathStep1 = joinPaths(dir, p);
        if (fileExists(pathStep1)) {
            return pathStep1;
        }

        // Preparazione per i tentativi successivi: prendiamo solo il nome del file pulito
        // Esempio: se p era "C:/Utenti/Bob/Desktop/texture.png", diventa "texture.png"
        std::string filename = getFileNameFormPath(p);

        // TENTATIVO 2: Cerca il file "sciolto" nella stessa cartella del modello
        // Utile se il modello aveva percorsi assoluti rotti
        std::string pathStep2 = joinPaths(dir, filename);
        if (fileExists(pathStep2)) {
            return pathStep2;
        }

        // TENTATIVO 3 (QUELLO CHE TI MANCA): Cerca nella cartella textures globale
        // Esempio: assets/textures/ + texture.png
        // NOTA: Assicurati che il path sia relativo all'EXE (./assets/textures/)
        std::string pathStep3 = joinPaths("./assets/textures/", filename);
        if (fileExists(pathStep3)) {
            return pathStep3;
        }

        // TENTATIVO 4: Cerca nella cartella textures senza ./ (a volte serve)
        std::string pathStep4 = joinPaths("assets/textures/", filename);
        if (fileExists(pathStep4)) {
            return pathStep4;
        }

        // Se fallisce tutto, ritorna il primo tentativo così l'errore in console
        // ti dirà dove ha provato a cercare inizialmente.
        return pathStep1;
#else
        p = getFileNameFormPath(p);

        std::string fullPath = dir + "/" + p;

        return fullPath;
#endif
    }

    std::string Utils::getDirFromFilename(const std::string &fileName)
    {
        size_t slashIndex = fileName.find_last_of("/\\");

        if (slashIndex == std::string::npos)
        {
            return ".";
        }
        else if (slashIndex == 0)
        {
            return "/";
        }
        else
        {
            return fileName.substr(0, slashIndex);
        }
    }

    std::string Utils::getExtension(const char *name)
    {
        std::string nameStr(name);
        size_t dotLoc = nameStr.find_last_of('.');

        if (dotLoc != std::string::npos)
        {
            return nameStr.substr(dotLoc);
        }
        return "";
    }

    // Sostituisce std::filesystem::path(p).filename().string()
    std::string Utils::getFileNameFormPath(const std::string &path)
    {
        // Cerca l'ultimo separatore (supporta sia slash che backslash)
        size_t pivot = path.find_last_of("/\\");

        // Se non trova separatori, l'intero path è il nome del file
        if (pivot == std::string::npos)
        {
            return path;
        }

        // Restituisce tutto ciò che c'è dopo il separatore
        return path.substr(pivot + 1);
    }

    // Sostituisce std::filesystem::path(p).stem().string()
    std::string Utils::getFileNameNoExt(const std::string &filePath)
    {
        // Prima isoliamo il nome del file dal percorso completo
        std::string filename = getFileNameFormPath(filePath);

        // Poi cerchiamo l'ultimo punto
        size_t lastDot = filename.find_last_of('.');

        // Se non c'è punto, restituisce il nome intero.
        // Se c'è, restituisce la sottostringa da 0 al punto.
        if (lastDot == std::string::npos)
        {
            return filename;
        }
        return filename.substr(0, lastDot);
    }

    // Sostituisce l'operatore / di filesystem
    std::string Utils::joinPaths(const std::string &a, const std::string &b)
    {
        if (a.empty())
            return b;
        if (b.empty())
            return a;

        char lastChar = a.back();
        // Controlla se 'a' finisce già con un separatore
        bool aHasSlash = (lastChar == '\\' || lastChar == '/');
        bool bHasSlash = (b.front() == '\\' || b.front() == '/');
        if (aHasSlash && bHasSlash)
        {
            // Se entrambi hanno lo slash (es: "dir/" e "/file"), togliamone uno
            return a + b.substr(1);
        }
        else if (!aHasSlash && !bHasSlash)
        {
            // Se nessuno ha lo slash, aggiungiamolo (usa / che funziona ovunque)
            return a + '/' + b;
        }

        // Altrimenti aggiunge il separatore di Windows standard
        return a + b;
    }

    //-------------------------------------------------//
    //-----------------------PRNG----------------------//
    //-------------------------------------------------//
    uint32_t PRNG::m_state = 1239131;

    uint32_t PRNG::getNumber()
    {
        m_state = xorShift(m_state);
        return m_state;
    }

    uint32_t PRNG::xorShift(uint32_t value) 
    {
        value ^= value << 13;
        value ^= value >> 17;
        value ^= value << 4;
        return value;
    }

    
}