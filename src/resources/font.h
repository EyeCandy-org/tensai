#ifndef TENSAI_FONT_H
#define TENSAI_FONT_H

#include <SDL2/SDL_ttf.h>
#include <string>

class Font {
public:
    TTF_Font* font = nullptr;
    int size;
    Font(const std::string& path, int size);
    ~Font();
};

#endif // TENSAI_FONT_H