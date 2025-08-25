#ifndef TENSAI_TEXTURE_H
#define TENSAI_TEXTURE_H

#include <SDL2/SDL.h>

class Texture {
public:
  SDL_Texture *texture = nullptr;
  int width = 0, height = 0;
  ~Texture();
};

#endif // TENSAI_TEXTURE_H