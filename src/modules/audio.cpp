#include "audio.h"

Audio::Audio() {
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
}

Audio::~Audio() {
    Mix_CloseAudio();
}

std::shared_ptr<Sound> Audio::loadSound(const std::string& path) {
    auto sound = std::make_shared<Sound>(path);
    if (sound->chunk) {
        sounds[path] = sound;
        return sound;
    }
    return nullptr;
}

std::shared_ptr<Music> Audio::loadMusic(const std::string& path) {
    auto music = std::make_shared<Music>(path);
    if (music->music) {
        musics[path] = music;
        return music;
    }
    return nullptr;
}

void Audio::playSound(std::shared_ptr<Sound> sound, int volume, int channel) {
    if (sound && sound->chunk) {
        Mix_VolumeChunk(sound->chunk, volume);
        Mix_PlayChannel(channel, sound->chunk, 0);
    }
}

void Audio::playMusic(std::shared_ptr<Music> music, int loops) {
    if (music && music->music) {
        Mix_PlayMusic(music->music, loops);
    }
}

void Audio::pauseMusic() { Mix_PauseMusic(); }

void Audio::resumeMusic() { Mix_ResumeMusic(); }

void Audio::stopMusic() { Mix_HaltMusic(); }

void Audio::setMusicVolume(int volume) { Mix_VolumeMusic(volume); }

bool Audio::isMusicPlaying() const { return Mix_PlayingMusic(); }

bool Audio::isMusicPaused() const { return Mix_PausedMusic(); }