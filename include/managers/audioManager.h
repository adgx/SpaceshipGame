#pragma once
#include "log.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <AL/alc.h>
#include <AL/al.h>

namespace SpaceEngine{
    class AudioManager {
        public:
            void Initialize();
            void Shutdown();

            void LoadSound(const std::string& name, const std::string& filePath);
            void PlaySound(const std::string& name);

            void PlayMusic(const std::string& name, bool loop);
            void StopMusic();

            void SetVolume(float volume); // da 0.0f a 1.0f

        private:
            ALuint GetAvailableSource();
            ALCdevice* m_device = nullptr;
            ALCcontext* m_context = nullptr;

            std::vector<ALuint> m_sources;

            //cache per suoni e musica
            std::unordered_map<std::string, ALuint> m_soundBuffers;

            ALuint m_musicSource = 0;
    };
}