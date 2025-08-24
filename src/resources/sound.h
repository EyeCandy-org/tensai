#ifndef TENSAI_SOUND_H
#define TENSAI_SOUND_H

#include <SDL2/SDL_mixer.h>
#include <string>

class Sound {
public:
    Mix_Chunk* chunk = nullptr;
    Sound(const std::string& path);
    ~Sound();
};

#endif // TENSAI_SOUND_H