#include "audioManager.h"
#include "log.h"

#define DR_WAV_IMPLEMENTATION
#include "utils/dr_wav.h"

namespace SpaceEngine
{
    /*void PlayTestBeep(ALCdevice* device) {
        // Frequenza 440Hz (La), durata 1 secondo
        const int sampleRate = 44100;
        const int duration = 1; 
        const int size = sampleRate * duration;
        std::vector<short> bufferData(size);

        for (int i = 0; i < size; ++i) {
            // Onda sinusoidale semplice
            bufferData[i] = static_cast<short>(32760 * sin((2.0 * 3.14159 * 440.0 * i) / sampleRate));
        }

        ALuint buffer;
        alGenBuffers(1, &buffer);
        alBufferData(buffer, AL_FORMAT_MONO16, bufferData.data(), bufferData.size() * sizeof(short), sampleRate);

        ALuint source;
        alGenSources(1, &source);
        alSourcei(source, AL_BUFFER, buffer);
        alSourcei(source, AL_SOURCE_RELATIVE, AL_TRUE); // Suona "in testa"
        alSourcef(source, AL_GAIN, 1.0f); // Volume massimo
        
        alSourcePlay(source);
        
        SPACE_ENGINE_INFO("--- TEST BEEP START ---");
        
        // Attendiamo che finisca (solo per test, blocca il thread per 1 secondo!)
        // In un gioco vero non si fa, ma qui serve per capire.
        ALint state;
        do {
            alGetSourcei(source, AL_SOURCE_STATE, &state);
        } while (state == AL_PLAYING);

        SPACE_ENGINE_INFO("--- TEST BEEP END ---");

        alDeleteSources(1, &source);
        alDeleteBuffers(1, &buffer);
    }*/

    void AudioManager::Initialize()
    {
        m_device = alcOpenDevice(nullptr);
        if (!m_device) {
            SPACE_ENGINE_FATAL("OpenAL: Failed to open default device.");
            return;
        }

        m_context = alcCreateContext(m_device, nullptr);
        if (!m_context || !alcMakeContextCurrent(m_context)) {
            SPACE_ENGINE_FATAL("OpenAL: Failed to create/set context.");
            return;
        }

        alGenSources(1, &m_musicSource);

        SPACE_ENGINE_INFO("AudioManager (OpenAL) Initialized.");
    }

    void AudioManager::Shutdown()
    {
        alSourceStop(m_musicSource);
        alDeleteSources(1, &m_musicSource);

        for (ALuint source : m_sources) {
            alDeleteSources(1, &source);
        }
        m_sources.clear();

        for (auto& pair : m_soundBuffers) {
            alDeleteBuffers(1, &pair.second);
        }
        m_soundBuffers.clear();

        alcMakeContextCurrent(nullptr);
        if (m_context) alcDestroyContext(m_context);
        if (m_device) alcCloseDevice(m_device);

        SPACE_ENGINE_INFO("AudioManager Shutdown.");
    }

    void AudioManager::LoadSound(const std::string& name, const std::string& filePath)
    {
        if (m_soundBuffers.find(name) != m_soundBuffers.end()) {
            SPACE_ENGINE_WARN("Audio '{}' already loaded.", name);
            return;
        }

        // Caricamento WAV con dr_wav
        unsigned int channels;
        unsigned int sampleRate;
        drwav_uint64 totalPCMFrameCount;
        
        short* pSampleData = drwav_open_file_and_read_pcm_frames_s16(
            filePath.c_str(), 
            &channels, 
            &sampleRate, 
            &totalPCMFrameCount, 
            nullptr
        );

        if (pSampleData == nullptr) {
            SPACE_ENGINE_ERROR("Failed to load audio file: {}", filePath);
            return;
        }

        /*SPACE_ENGINE_INFO("File: {}", filePath);
        SPACE_ENGINE_INFO(" > Channels: {}", channels);         
        SPACE_ENGINE_INFO(" > SampleRate: {}", sampleRate);
        SPACE_ENGINE_INFO(" > Frames: {}", totalPCMFrameCount);per verificare che i file audio non siano vuoti o corrotti*/

        ALenum format;
        if (channels == 1) format = AL_FORMAT_MONO16;
        else if (channels == 2) format = AL_FORMAT_STEREO16;
        else {
            SPACE_ENGINE_ERROR("Unsupported channel count in: {}", filePath);
            drwav_free(pSampleData, nullptr);
            return;
        }

        ALuint bufferID;
        alGenBuffers(1, &bufferID);
        alBufferData(bufferID, format, pSampleData, (ALsizei)(totalPCMFrameCount * channels * sizeof(short)), sampleRate);

        drwav_free(pSampleData, nullptr);

        m_soundBuffers[name] = bufferID;
        SPACE_ENGINE_INFO("Loaded Sound: {}", name);
    }

    ALuint AudioManager::GetAvailableSource()
    {
        for (ALuint source : m_sources) {
            ALint state;
            alGetSourcei(source, AL_SOURCE_STATE, &state);
            if (state != AL_PLAYING) {
                return source;
            }
        }

        ALuint newSource;
        alGenSources(1, &newSource);
        m_sources.push_back(newSource);
        return newSource;
    }

    void AudioManager::PlaySound(const std::string& name)
    {
        if (m_soundBuffers.find(name) == m_soundBuffers.end()) {
            SPACE_ENGINE_ERROR("Sound not found: {}", name);
            return;
        }

        ALuint buffer = m_soundBuffers[name];
        ALuint source = GetAvailableSource();

        alSourcei(source, AL_BUFFER, buffer);
        alSourcei(source, AL_LOOPING, AL_FALSE); 
        alSourcei(source, AL_SOURCE_RELATIVE, AL_TRUE); //TODO: da cambiare se vogliamo suoni 3D(proiettili direzzionali, ecc)

        alSourcef(source, AL_GAIN, 1.0f);
        alSourcePlay(source);
    }

    void AudioManager::PlayMusic(const std::string& name, bool loop)
    {
        if (m_soundBuffers.find(name) == m_soundBuffers.end()) return;

        alSourceStop(m_musicSource);

        ALuint buffer = m_soundBuffers[name];
        alSourcei(m_musicSource, AL_BUFFER, buffer);
        alSourcei(m_musicSource, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
        alSourcei(m_musicSource, AL_SOURCE_RELATIVE, AL_TRUE); //TODO: da cambiare se vogliamo musica 3D(probabilmente no)
        alSourcef(m_musicSource, AL_GAIN, 0.5f); // Volume musica al 50% di default
        alSourcePlay(m_musicSource);

        ALint state;
        alGetSourcei(m_musicSource, AL_SOURCE_STATE, &state);
        if (state != AL_PLAYING) {
            SPACE_ENGINE_ERROR("OpenAL Error: Music source failed to play! State: {}", state);
        }
    }

    void AudioManager::StopMusic()
    {
        alSourceStop(m_musicSource);
    }

    void AudioManager::SetVolume(float volume)
    {
        alListenerf(AL_GAIN, volume);
    }
};