#include "sound.h"

Sound::Sound(const std::string &path) { chunk = Mix_LoadWAV(path.c_str()); }

Sound::~Sound() {
  if (chunk)
    Mix_FreeChunk(chunk);
}