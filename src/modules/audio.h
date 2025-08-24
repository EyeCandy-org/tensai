#ifndef TENSAI_AUDIO_H
#define TENSAI_AUDIO_H

#include <unordered_map>
#include <memory>
#include "../resources/sound.h"
#include "../resources/music.h"
#include <SDL2/SDL_mixer.h>

class Audio {
private:
    std::unordered_map<std::string, std::shared_ptr<Sound>> sounds;
    std::unordered_map<std::string, std::shared_ptr<Music>> musics;
    
public:
    Audio();
    ~Audio();
    
    std::shared_ptr<Sound> loadSound(const std::string& path);
    std::shared_ptr<Music> loadMusic(const std::string& path);
    
    void playSound(std::shared_ptr<Sound> sound, int volume = 128, int channel = -1);
    void playMusic(std::shared_ptr<Music> music, int loops = -1);
    
    void pauseMusic();
    void resumeMusic();
    void stopMusic();
    void setMusicVolume(int volume);
    
    bool isMusicPlaying() const;
    bool isMusicPaused() const;
};

#endif // TENSAI_AUDIO_H