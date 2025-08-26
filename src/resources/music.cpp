#include "music.h"
#include <cstdio>
#include <cstdlib>

Music::Music(const std::string &path) {
  music = Mix_LoadMUS(path.c_str());
  if (!music) {
    fprintf(stderr, "Warning: Error loading music: %s\n", Mix_GetError());
  }
}

Music::~Music() {
  if (music)
    Mix_FreeMusic(music);
}
