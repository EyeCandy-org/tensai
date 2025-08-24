#include "texture.h"

Texture::~Texture() { if (texture) SDL_DestroyTexture(texture); }