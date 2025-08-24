#include "font.h"

Font::Font(const std::string& path, int size) : size(size) {
    font = TTF_OpenFont(path.c_str(), size);
}

Font::~Font() { if (font) TTF_CloseFont(font); }