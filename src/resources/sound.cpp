#include "sound.h"
#include <cstdio>
#include <cstdlib>

Sound::Sound(const std::string &path) {
  chunk = Mix_LoadWAV(path.c_str());
  if (!chunk) {
    fprintf(stderr, "Warning: Error loading sound WAV: %s\n", Mix_GetError());
  }
}

Sound::~Sound() {
  if (chunk)
    Mix_FreeChunk(chunk);
}
