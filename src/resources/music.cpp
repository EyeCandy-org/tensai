#include "music.h"

Music::Music(const std::string &path) { music = Mix_LoadMUS(path.c_str()); }

Music::~Music() {
  if (music)
    Mix_FreeMusic(music);
}