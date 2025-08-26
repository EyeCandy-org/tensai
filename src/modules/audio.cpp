#include "audio.h"
#include <cstdio>
#include <cstdlib>

Audio::Audio() {
  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
    fprintf(stderr, "Warning: Error initializing SDL_mixer: %s\n", Mix_GetError());
  }
}

Audio::~Audio() { Mix_CloseAudio(); }

std::shared_ptr<Sound> Audio::loadSound(const std::string &path) {
  auto sound = std::make_shared<Sound>(path);
  if (sound && sound->chunk) {
    sounds[path] = sound;
    return sound;
  }
  fprintf(stderr, "Warning: Error loading sound: %s\n", path.c_str());
  return nullptr;
}

std::shared_ptr<Music> Audio::loadMusic(const std::string &path) {
  auto music = std::make_shared<Music>(path);
  if (music && music->music) {
    musics[path] = music;
    return music;
  }
  fprintf(stderr, "Warning: Error loading music: %s\n", path.c_str());
  return nullptr;
}

void Audio::playSound(std::shared_ptr<Sound> sound, int volume, int channel) {
  if (sound && sound->chunk) {
    Mix_VolumeChunk(sound->chunk, volume);
    if (Mix_PlayChannel(channel, sound->chunk, 0) == -1) {
      fprintf(stderr, "Error playing sound: %s\n", Mix_GetError());
      exit(1);
    }
  }
}

void Audio::playMusic(std::shared_ptr<Music> music, int loops) {
  if (music && music->music) {
    if (Mix_PlayMusic(music->music, loops) == -1) {
      fprintf(stderr, "Error playing music: %s\n", Mix_GetError());
      exit(1);
    }
  }
}

void Audio::pauseMusic() {
  Mix_PauseMusic();
}

void Audio::resumeMusic() {
  Mix_ResumeMusic();
}

void Audio::stopMusic() {
  Mix_HaltMusic();
}

void Audio::setMusicVolume(int volume) {
  Mix_VolumeMusic(volume);
}

bool Audio::isMusicPlaying() const { return Mix_PlayingMusic(); }
bool Audio::isMusicPaused() const { return Mix_PausedMusic(); }

