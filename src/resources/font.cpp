#include "font.h"
#include <cstdio>
#include <cstdlib>

Font::Font(const std::string &path, int size) : size(size) {
  font = TTF_OpenFont(path.c_str(), size);
  if (!font) {
    fprintf(stderr, "Warning: Error loading font: %s\n", path.c_str());
  }
}

Font::~Font() {
  if (font)
    TTF_CloseFont(font);
}

