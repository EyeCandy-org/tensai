#ifndef TENSAI_MUSIC_H
#define TENSAI_MUSIC_H

#include <SDL2/SDL_mixer.h>
#include <string>

class Music {
public:
    Mix_Music* music = nullptr;
    Music(const std::string& path);
    ~Music();
};

#endif // TENSAI_MUSIC_H