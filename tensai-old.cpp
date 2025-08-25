#include <napi.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h> 
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <unordered_map>
#include <vector>
#include <string>
#include <memory>
#include <chrono>
#include <random>
#include <cmath>
#include <algorithm>
#include <thread>
class Vec2 {
public:
    float x, y;
    Vec2(float x = 0, float y = 0) : x(x), y(y) {}
    Vec2 operator+(const Vec2& other) const { return Vec2(x + other.x, y + other.y); }
    Vec2 operator-(const Vec2& other) const { return Vec2(x - other.x, y - other.y); }
    Vec2 operator*(float scalar) const { return Vec2(x * scalar, y * scalar); }
    Vec2 operator*(const Vec2& other) const { return Vec2(x * other.x, y * other.y); }
    float length() const { return sqrt(x * x + y * y); }
    Vec2 normalize() const { float len = length(); return len > 0 ? Vec2(x/len, y/len) : Vec2(0, 0); }
};

class Color {
public:
    Uint8 r, g, b, a;
    Color(Uint8 r = 255, Uint8 g = 255, Uint8 b = 255, Uint8 a = 255) : r(r), g(g), b(b), a(a) {}
};

class Transform {
public:
    Vec2 position{0, 0};
    Vec2 scale{1, 1};
    float rotation = 0.0f;
    Vec2 origin{0, 0};
    SDL_Point getSDLOrigin() const { return {(int)origin.x, (int)origin.y}; }
    double getRotationDegrees() const { return rotation * 180.0 / M_PI; }
};

class Texture {
public:
    SDL_Texture* texture = nullptr;
    int width = 0, height = 0;
    ~Texture() { if (texture) SDL_DestroyTexture(texture); }
};

class Font {
public:
    TTF_Font* font = nullptr;
    int size;
    Font(const std::string& path, int size) : size(size) {
        font = TTF_OpenFont(path.c_str(), size);
    }
    
    ~Font() { if (font) TTF_CloseFont(font); }
};

class Sound {
public:
    Mix_Chunk* chunk = nullptr;
    Sound(const std::string& path) {
        chunk = Mix_LoadWAV(path.c_str());
    }
    
    ~Sound() { if (chunk) Mix_FreeChunk(chunk); }
};

class Music {
public:
    Mix_Music* music = nullptr;
    Music(const std::string& path) {
        music = Mix_LoadMUS(path.c_str());
    }
    
    ~Music() { if (music) Mix_FreeMusic(music); }
};

class Input {
private:
    std::unordered_map<int, bool> currentKeys;
    std::unordered_map<int, bool> previousKeys;
    std::unordered_map<int, bool> currentMouse;
    std::unordered_map<int, bool> previousMouse;
    Vec2 mousePos{0, 0};
    Vec2 mouseWheel{0, 0};
    
public:
    void update() {
        previousKeys = currentKeys;
        previousMouse = currentMouse;
        mouseWheel = Vec2(0, 0);
    }
    
    void setKey(int key, bool pressed) { currentKeys[key] = pressed; }
    void setMouse(int button, bool pressed) { currentMouse[button] = pressed; }
    void setMousePos(int x, int y) { mousePos = Vec2(x, y); }
    void setMouseWheel(int x, int y) { mouseWheel = Vec2(x, y); }
    bool isDown(int key) const { 
        auto it = currentKeys.find(key);
        return it != currentKeys.end() && it->second;
    }
    
    bool isPressed(int key) const {
        auto curr = currentKeys.find(key);
        auto prev = previousKeys.find(key);
        bool currPressed = curr != currentKeys.end() && curr->second;
        bool prevPressed = prev != previousKeys.end() && prev->second;
        return currPressed && !prevPressed;
    }
    
    bool isReleased(int key) const {
        auto curr = currentKeys.find(key);
        auto prev = previousKeys.find(key);
        bool currPressed = curr != currentKeys.end() && curr->second;
        bool prevPressed = prev != previousKeys.end() && prev->second;
        return !currPressed && prevPressed;
    }
    
    bool isMouseDown(int button) const {
        auto it = currentMouse.find(button);
        return it != currentMouse.end() && it->second;
    }
    
    bool isMousePressed(int button) const {
        auto curr = currentMouse.find(button);
        auto prev = previousMouse.find(button);
        bool currPressed = curr != currentMouse.end() && curr->second;
        bool prevPressed = prev != previousMouse.end() && prev->second;
        return currPressed && !prevPressed;
    }
    
    bool isMouseReleased(int button) const {
        auto curr = currentMouse.find(button);
        auto prev = previousMouse.find(button);
        bool currPressed = curr != currentMouse.end() && curr->second;
        bool prevPressed = prev != previousMouse.end() && prev->second;
        return !currPressed && prevPressed;
    }
    
    Vec2 getMousePosition() const { return mousePos; }
    Vec2 getMouseWheel() const { return mouseWheel; }
};

class Camera {
public:
    Vec2 position{0, 0};
    float rotation = 0.0f;
    Vec2 scale{1, 1};
    
    void translate(const Vec2& offset) { position = position + offset; }
    void rotate(float angle) { rotation += angle; }
    void zoom(float factor) { scale = scale * factor; }
    void lookAt(const Vec2& target) { position = target; }
    
    Vec2 worldToScreen(const Vec2& worldPos, int screenWidth, int screenHeight) const {
        Vec2 translated = worldPos - position;
        float cos_r = cos(-rotation);
        float sin_r = sin(-rotation);
        Vec2 rotated(translated.x * cos_r - translated.y * sin_r,
                     translated.x * sin_r + translated.y * cos_r);
        Vec2 scaled(rotated.x * scale.x, rotated.y * scale.y);
        return Vec2(scaled.x + screenWidth/2, scaled.y + screenHeight/2);
    }
    
    Vec2 screenToWorld(const Vec2& screenPos, int screenWidth, int screenHeight) const {
        Vec2 centered(screenPos.x - screenWidth/2, screenPos.y - screenHeight/2);
        Vec2 unscaled(centered.x / scale.x, centered.y / scale.y);
        float cos_r = cos(rotation);
        float sin_r = sin(rotation);
        Vec2 rotated(unscaled.x * cos_r - unscaled.y * sin_r,
                     unscaled.x * sin_r + unscaled.y * cos_r);
        return rotated + position;
    }
};

class Graphics {
private:
    SDL_Renderer* renderer;
    Camera camera;
    Color currentColor{255, 255, 255, 255};
    std::shared_ptr<Font> currentFont;
    float lineWidth = 1.0f;
    
public:
    Graphics(SDL_Renderer* r) : renderer(r) {}
    
    void setColor(const Color& color) {
        currentColor = color;
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    }
    
    void setFont(std::shared_ptr<Font> font) { currentFont = font; }
    void setLineWidth(float width) { lineWidth = width; }
    
    void clear(const Color& color = Color(0, 0, 0, 255)) {
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderClear(renderer);
    }
    
    void present() { SDL_RenderPresent(renderer); }
    
    void drawPoint(const Vec2& pos) {
        SDL_RenderDrawPoint(renderer, (int)pos.x, (int)pos.y);
    }
    
    void drawLine(const Vec2& start, const Vec2& end) {
        if (lineWidth <= 1.0f) {
            SDL_RenderDrawLine(renderer, (int)start.x, (int)start.y, (int)end.x, (int)end.y);
        } else {
            Vec2 dir = (end - start).normalize();
            Vec2 perp(-dir.y, dir.x);
            Vec2 offset = perp * (lineWidth / 2.0f);
            
            SDL_Point points[4] = {
                {(int)(start.x + offset.x), (int)(start.y + offset.y)},
                {(int)(start.x - offset.x), (int)(start.y - offset.y)},
                {(int)(end.x - offset.x), (int)(end.y - offset.y)},
                {(int)(end.x + offset.x), (int)(end.y + offset.y)}
            };
            
            SDL_RenderDrawLines(renderer, points, 5);
        }
    }
    
    void drawRect(const Vec2& pos, const Vec2& size, bool filled = false) {
        SDL_Rect rect = {(int)pos.x, (int)pos.y, (int)size.x, (int)size.y};
        if (filled) {
            SDL_RenderFillRect(renderer, &rect);
        } else {
            SDL_RenderDrawRect(renderer, &rect);
        }
    }
    
    void drawCircle(const Vec2& center, float radius, bool filled = false) {
        int segments = std::max(8, (int)(radius * 0.5f));
        std::vector<SDL_Point> points;
        
        for (int i = 0; i <= segments; i++) {
            float angle = 2.0f * M_PI * i / segments;
            int x = (int)(center.x + radius * cos(angle));
            int y = (int)(center.y + radius * sin(angle));
            points.push_back({x, y});
        }
        
        if (filled) {
            for (int i = 1; i < segments; i++) {
                SDL_Point triangle[3] = {
                    {(int)center.x, (int)center.y},
                    points[i-1],
                    points[i]
                };
                SDL_RenderDrawLines(renderer, triangle, 4);
            }
        } else {
            SDL_RenderDrawLines(renderer, points.data(), points.size());
        }
    }
    
    void drawEllipse(const Vec2& center, const Vec2& radii, bool filled = false) {
        int segments = std::max(16, (int)((radii.x + radii.y) * 0.25f));
        std::vector<SDL_Point> points;
        
        for (int i = 0; i <= segments; i++) {
            float angle = 2.0f * M_PI * i / segments;
            int x = (int)(center.x + radii.x * cos(angle));
            int y = (int)(center.y + radii.y * sin(angle));
            points.push_back({x, y});
        }
        
        if (filled) {
            for (int i = 1; i < segments; i++) {
                SDL_Point triangle[3] = {
                    {(int)center.x, (int)center.y},
                    points[i-1],
                    points[i]
                };
                SDL_RenderDrawLines(renderer, triangle, 4);
            }
        } else {
            SDL_RenderDrawLines(renderer, points.data(), points.size());
        }
    }
    
    void drawTexture(std::shared_ptr<Texture> texture, const Transform& transform, const Color& tint = Color(255,255,255,255)) {
        if (!texture || !texture->texture) return;
        
        SDL_SetTextureColorMod(texture->texture, tint.r, tint.g, tint.b);
        SDL_SetTextureAlphaMod(texture->texture, tint.a);
        
        SDL_Rect dst = {
            (int)(transform.position.x - transform.origin.x * transform.scale.x),
            (int)(transform.position.y - transform.origin.y * transform.scale.y),
            (int)(texture->width * transform.scale.x),
            (int)(texture->height * transform.scale.y)
        };
        
        if (transform.rotation == 0.0f) {
            SDL_RenderCopy(renderer, texture->texture, nullptr, &dst);
        } else {
            SDL_Point center = transform.getSDLOrigin();
            SDL_RenderCopyEx(renderer, texture->texture, nullptr, &dst, 
                           transform.getRotationDegrees(), &center, SDL_FLIP_NONE);
        }
    }
    
    void drawText(const std::string& text, const Vec2& pos, const Color& color = Color(255,255,255,255)) {
        if (!currentFont || !currentFont->font) return;
        
        SDL_Color sdlColor = {color.r, color.g, color.b, color.a};
        SDL_Surface* surface = TTF_RenderText_Solid(currentFont->font, text.c_str(), sdlColor);
        if (!surface) return;
        
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (texture) {
            SDL_Rect dst = {(int)pos.x, (int)pos.y, surface->w, surface->h};
            SDL_RenderCopy(renderer, texture, nullptr, &dst);
            SDL_DestroyTexture(texture);
        }
        SDL_FreeSurface(surface);
    }
    
    void drawPolygon(const std::vector<Vec2>& vertices, bool filled = false) {
        if (vertices.size() < 3) return;
        
        std::vector<SDL_Point> points;
        for (const auto& v : vertices) {
            points.push_back({(int)v.x, (int)v.y});
        }
        points.push_back(points[0]);
        
        if (filled) {
            for (size_t i = 1; i < vertices.size() - 1; i++) {
                SDL_Point triangle[4] = {
                    {(int)vertices[0].x, (int)vertices[0].y},
                    {(int)vertices[i].x, (int)vertices[i].y},
                    {(int)vertices[i+1].x, (int)vertices[i+1].y},
                    {(int)vertices[0].x, (int)vertices[0].y}
                };
                SDL_RenderDrawLines(renderer, triangle, 4);
            }
        } else {
            SDL_RenderDrawLines(renderer, points.data(), points.size());
        }
    }
    
    Camera& getCamera() { return camera; }
    void setCamera(const Camera& cam) { camera = cam; }
    
    void pushMatrix() {}
    void popMatrix() {}
    void translate(const Vec2& offset) { camera.translate(offset * -1); }
    void rotate(float angle) { camera.rotate(angle * -1); }
    void scale(const Vec2& scale) { camera.scale = camera.scale * Vec2(1.0f/scale.x, 1.0f/scale.y); }
};

class Timer {
private:
    std::chrono::high_resolution_clock::time_point startTime;
    std::chrono::high_resolution_clock::time_point lastFrame;
    double deltaTime = 0.0;
    double totalTime = 0.0;
    int frameCount = 0;
    double fps = 0.0;
    double fpsUpdateTime = 0.0;
    
public:
    Timer() : startTime(std::chrono::high_resolution_clock::now()), lastFrame(startTime) {}
    
    void update() {
        auto currentTime = std::chrono::high_resolution_clock::now();
        deltaTime = std::chrono::duration<double>(currentTime - lastFrame).count();
        totalTime = std::chrono::duration<double>(currentTime - startTime).count();
        lastFrame = currentTime;
        frameCount++;
        
        fpsUpdateTime += deltaTime;
        if (fpsUpdateTime >= 1.0) {
            fps = frameCount / fpsUpdateTime;
            frameCount = 0;
            fpsUpdateTime = 0.0;
        }
    }
    
    double getDelta() const { return deltaTime; }
    double getTime() const { return totalTime; }
    double getFPS() const { return fps; }
    
    void sleep(double seconds) {
        auto duration = std::chrono::duration<double>(seconds);
        std::this_thread::sleep_for(duration);
    }
};

class Random {
private:
    std::mt19937 generator;
    
public:
    Random() : generator(std::random_device{}()) {}
    Random(uint32_t seed) : generator(seed) {}
    
    void setSeed(uint32_t seed) { generator.seed(seed); }
    
    int randomInt(int min, int max) {
        std::uniform_int_distribution<int> dist(min, max);
        return dist(generator);
    }
    
    float randomFloat(float min = 0.0f, float max = 1.0f) {
        std::uniform_real_distribution<float> dist(min, max);
        return dist(generator);
    }
    
    bool randomBool() {
        return randomFloat() < 0.5f;
    }
    
    Vec2 randomVec2(const Vec2& min = Vec2(0,0), const Vec2& max = Vec2(1,1)) {
        return Vec2(randomFloat(min.x, max.x), randomFloat(min.y, max.y));
    }
};

class Physics {
public:
    struct Body {
        Vec2 position{0, 0};
        Vec2 velocity{0, 0};
        Vec2 acceleration{0, 0};
        float mass = 1.0f;
        float friction = 0.0f;
        float restitution = 1.0f;
        bool kinematic = false;
    };
    
    struct AABB {
        Vec2 min, max;
        
        bool intersects(const AABB& other) const {
            return max.x >= other.min.x && min.x <= other.max.x &&
                   max.y >= other.min.y && min.y <= other.max.y;
        }
        
        bool contains(const Vec2& point) const {
            return point.x >= min.x && point.x <= max.x &&
                   point.y >= min.y && point.y <= max.y;
        }
    };
    
    static bool circleCircleCollision(const Vec2& pos1, float radius1, const Vec2& pos2, float radius2) {
        float dx = pos2.x - pos1.x;
        float dy = pos2.y - pos1.y;
        float distance = sqrt(dx*dx + dy*dy);
        return distance < (radius1 + radius2);
    }
    
    static bool pointInCircle(const Vec2& point, const Vec2& center, float radius) {
        float dx = point.x - center.x;
        float dy = point.y - center.y;
        return (dx*dx + dy*dy) <= (radius*radius);
    }
    
    static void updateBody(Body& body, float dt) {
        if (body.kinematic) return;
        
        body.velocity = body.velocity + body.acceleration * dt;
        body.velocity = body.velocity * (1.0f - body.friction * dt);
        body.position = body.position + body.velocity * dt;
        body.acceleration = Vec2(0, 0);
    }
    
    static void applyForce(Body& body, const Vec2& force) {
        body.acceleration = body.acceleration + force * (1.0f / body.mass);
    }
    
    static void resolveCollision(Body& body1, Body& body2, const Vec2& normal) {
        Vec2 relativeVelocity = body2.velocity - body1.velocity;
        float velAlongNormal = relativeVelocity.x * normal.x + relativeVelocity.y * normal.y;
        
        if (velAlongNormal > 0) return;
        
        float e = std::min(body1.restitution, body2.restitution);
        float j = -(1 + e) * velAlongNormal;
        j /= (1.0f / body1.mass) + (1.0f / body2.mass);
        
        Vec2 impulse = normal * j;
        body1.velocity = body1.velocity - impulse * (1.0f / body1.mass);
        body2.velocity = body2.velocity + impulse * (1.0f / body2.mass);
    }
};

class Audio {
private:
    std::unordered_map<std::string, std::shared_ptr<Sound>> sounds;
    std::unordered_map<std::string, std::shared_ptr<Music>> musics;
    
public:
    Audio() {
        Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    }
    
    ~Audio() {
        Mix_CloseAudio();
    }
    
    std::shared_ptr<Sound> loadSound(const std::string& path) {
        auto sound = std::make_shared<Sound>(path);
        if (sound->chunk) {
            sounds[path] = sound;
            return sound;
        }
        return nullptr;
    }
    
    std::shared_ptr<Music> loadMusic(const std::string& path) {
        auto music = std::make_shared<Music>(path);
        if (music->music) {
            musics[path] = music;
            return music;
        }
        return nullptr;
    }
    
    void playSound(std::shared_ptr<Sound> sound, int volume = 128, int channel = -1) {
        if (sound && sound->chunk) {
            Mix_VolumeChunk(sound->chunk, volume);
            Mix_PlayChannel(channel, sound->chunk, 0);
        }
    }
    
    void playMusic(std::shared_ptr<Music> music, int loops = -1) {
        if (music && music->music) {
            Mix_PlayMusic(music->music, loops);
        }
    }
    
    void pauseMusic() { Mix_PauseMusic(); }
    void resumeMusic() { Mix_ResumeMusic(); }
    void stopMusic() { Mix_HaltMusic(); }
    void setMusicVolume(int volume) { Mix_VolumeMusic(volume); }
    
    bool isMusicPlaying() const { return Mix_PlayingMusic(); }
    bool isMusicPaused() const { return Mix_PausedMusic(); }
};

class TensaiEngine : public Napi::ObjectWrap<TensaiEngine> {
private:
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    std::unique_ptr<Graphics> graphics;
    std::unique_ptr<Input> input;
    std::unique_ptr<Timer> timer;
    std::unique_ptr<Random> random;
    std::unique_ptr<Audio> audio;
    
    Napi::FunctionReference loadCallback;
    Napi::FunctionReference updateCallback;
    Napi::FunctionReference drawCallback;
    
    bool running = false;
    int windowWidth, windowHeight;
    std::string windowTitle;
    bool fullscreen, vsync;
    
    std::unordered_map<std::string, std::shared_ptr<Texture>> textures;
    std::unordered_map<std::string, std::shared_ptr<Font>> fonts;
    
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports) {
        Napi::Function func = DefineClass(env, "TensaiEngine", {
            InstanceMethod("run", &TensaiEngine::Run),
            InstanceAccessor("load", nullptr, &TensaiEngine::SetLoad),
            InstanceAccessor("update", nullptr, &TensaiEngine::SetUpdate),
            InstanceAccessor("draw", nullptr, &TensaiEngine::SetDraw),
            InstanceMethod("loadTexture", &TensaiEngine::LoadTexture),
            InstanceMethod("loadFont", &TensaiEngine::LoadFont),
            InstanceMethod("loadSound", &TensaiEngine::LoadSound),
            InstanceMethod("loadMusic", &TensaiEngine::LoadMusic),
            InstanceMethod("getWidth", &TensaiEngine::GetWidth),
            InstanceMethod("getHeight", &TensaiEngine::GetHeight),
            InstanceMethod("setTitle", &TensaiEngine::SetTitle),
            InstanceMethod("setFullscreen", &TensaiEngine::SetFullscreen),
            InstanceMethod("quit", &TensaiEngine::Quit),
            InstanceMethod("isKeyDown", &TensaiEngine::IsKeyDown),
            InstanceMethod("isKeyPressed", &TensaiEngine::IsKeyPressed),
            InstanceMethod("isKeyReleased", &TensaiEngine::IsKeyReleased),
            InstanceMethod("isMouseDown", &TensaiEngine::IsMouseDown),
            InstanceMethod("isMousePressed", &TensaiEngine::IsMousePressed),
            InstanceMethod("isMouseReleased", &TensaiEngine::IsMouseReleased),
            InstanceMethod("getMouseX", &TensaiEngine::GetMouseX),
            InstanceMethod("getMouseY", &TensaiEngine::GetMouseY),
            InstanceMethod("getMousePosition", &TensaiEngine::GetMousePosition),
            InstanceMethod("getDelta", &TensaiEngine::GetDelta),
            InstanceMethod("getTime", &TensaiEngine::GetTime),
            InstanceMethod("getFPS", &TensaiEngine::GetFPS),
            InstanceMethod("clear", &TensaiEngine::Clear),
            InstanceMethod("setColor", &TensaiEngine::SetColor),
            InstanceMethod("drawPoint", &TensaiEngine::DrawPoint),
            InstanceMethod("drawLine", &TensaiEngine::DrawLine),
            InstanceMethod("drawRect", &TensaiEngine::DrawRect),
            InstanceMethod("drawCircle", &TensaiEngine::DrawCircle),
            InstanceMethod("drawTexture", &TensaiEngine::DrawTexture),
            InstanceMethod("drawText", &TensaiEngine::DrawText),
            InstanceMethod("setFont", &TensaiEngine::SetFont),
            InstanceMethod("playSound", &TensaiEngine::PlaySound),
            InstanceMethod("playMusic", &TensaiEngine::PlayMusic),
            InstanceMethod("stopMusic", &TensaiEngine::StopMusic),
            InstanceMethod("setMusicVolume", &TensaiEngine::SetMusicVolume),
            InstanceMethod("randomInt", &TensaiEngine::RandomInt),
            InstanceMethod("randomFloat", &TensaiEngine::RandomFloat),
            InstanceMethod("randomBool", &TensaiEngine::RandomBool),
        });
        
        Napi::FunctionReference* constructor = new Napi::FunctionReference();
        *constructor = Napi::Persistent(func);
        env.SetInstanceData(constructor);
        
        exports.Set("TensaiEngine", func);
        return exports;
    }
    
    TensaiEngine(const Napi::CallbackInfo& info) : Napi::ObjectWrap<TensaiEngine>(info) {
        Napi::Env env = info.Env();
        
        if (info.Length() < 5) {
            Napi::TypeError::New(env, "Expected 5 arguments: title, width, height, fullscreen, vsync")
                .ThrowAsJavaScriptException();
            return;
        }
        
        windowTitle = info[0].As<Napi::String>().Utf8Value();
        windowWidth = info[1].As<Napi::Number>().Int32Value();
        windowHeight = info[2].As<Napi::Number>().Int32Value();
        fullscreen = info[3].As<Napi::Boolean>().Value();
        vsync = info[4].As<Napi::Boolean>().Value();
        
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS) != 0) {
            Napi::Error::New(env, "Failed to initialize SDL").ThrowAsJavaScriptException();
            return;
        }
        
        IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
        TTF_Init();
        
        Uint32 windowFlags = SDL_WINDOW_SHOWN;
        if (fullscreen) windowFlags |= SDL_WINDOW_FULLSCREEN;
        
        window = SDL_CreateWindow(
            windowTitle.c_str(),
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            windowWidth,
            windowHeight,
            windowFlags
        );
        
        if (!window) {
            Napi::Error::New(env, "Failed to create window").ThrowAsJavaScriptException();
            return;
        }
        
        Uint32 rendererFlags = SDL_RENDERER_ACCELERATED;
        if (vsync) rendererFlags |= SDL_RENDERER_PRESENTVSYNC;
        
        renderer = SDL_CreateRenderer(window, -1, rendererFlags);
        if (!renderer) {
            Napi::Error::New(env, "Failed to create renderer").ThrowAsJavaScriptException();
            return;
        }
        
        graphics = std::make_unique<Graphics>(renderer);
        input = std::make_unique<Input>();
        timer = std::make_unique<Timer>();
        random = std::make_unique<Random>();
        audio = std::make_unique<Audio>();
    }
    
    ~TensaiEngine() {
        if (renderer) SDL_DestroyRenderer(renderer);
        if (window) SDL_DestroyWindow(window);
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
    }
    
    void SetLoad(const Napi::CallbackInfo& info, const Napi::Value& value) {
        loadCallback = Napi::Persistent(value.As<Napi::Function>());
    }
    
    void SetUpdate(const Napi::CallbackInfo& info, const Napi::Value& value) {
        updateCallback = Napi::Persistent(value.As<Napi::Function>());
    }
    
    void SetDraw(const Napi::CallbackInfo& info, const Napi::Value& value) {
        drawCallback = Napi::Persistent(value.As<Napi::Function>());
    }
    
    Napi::Value Run(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (loadCallback) {
            loadCallback.Call({});
        }
        
        running = true;
        SDL_Event event;
        
        while (running) {
            timer->update();
            input->update();
            
            while (SDL_PollEvent(&event)) {
                switch (event.type) {
                    case SDL_QUIT:
                        running = false;
                        break;
                    case SDL_KEYDOWN:
                        input->setKey(event.key.keysym.sym, true);
                        break;
                    case SDL_KEYUP:
                        input->setKey(event.key.keysym.sym, false);
                        break;
                    case SDL_MOUSEBUTTONDOWN:
                        input->setMouse(event.button.button, true);
                        break;
                    case SDL_MOUSEBUTTONUP:
                        input->setMouse(event.button.button, false);
                        break;
                    case SDL_MOUSEMOTION:
                        input->setMousePos(event.motion.x, event.motion.y);
                        break;
                    case SDL_MOUSEWHEEL:
                        input->setMouseWheel(event.wheel.x, event.wheel.y);
                        break;
                }
            }
            
            if (updateCallback) {
                updateCallback.Call({Napi::Number::New(env, timer->getDelta())});
            }
            
            if (drawCallback) {
                drawCallback.Call({});
            }
            
            graphics->present();
        }
        
        return env.Undefined();
    }
    
    Napi::Value LoadTexture(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (info.Length() < 1) {
            Napi::TypeError::New(env, "Expected path argument").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        
        std::string path = info[0].As<Napi::String>().Utf8Value();
        
        SDL_Surface* surface = IMG_Load(path.c_str());
        if (!surface) {
            return env.Undefined();
        }
        
        auto texture = std::make_shared<Texture>();
        texture->texture = SDL_CreateTextureFromSurface(renderer, surface);
        texture->width = surface->w;
        texture->height = surface->h;
        
        SDL_FreeSurface(surface);
        
        if (texture->texture) {
            textures[path] = texture;
            return Napi::String::New(env, path);
        }
        
        return env.Undefined();
    }
    
    Napi::Value LoadFont(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (info.Length() < 2) {
            Napi::TypeError::New(env, "Expected path and size arguments").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        
        std::string path = info[0].As<Napi::String>().Utf8Value();
        int size = info[1].As<Napi::Number>().Int32Value();
        
        auto font = std::make_shared<Font>(path, size);
        if (font->font) {
            std::string key = path + "_" + std::to_string(size);
            fonts[key] = font;
            return Napi::String::New(env, key);
        }
        
        return env.Undefined();
    }
    
    Napi::Value LoadSound(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (info.Length() < 1) {
            Napi::TypeError::New(env, "Expected path argument").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        
        std::string path = info[0].As<Napi::String>().Utf8Value();
        auto sound = audio->loadSound(path);
        
        return sound ? Napi::String::New(env, path) : env.Undefined();
    }
    
    Napi::Value LoadMusic(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (info.Length() < 1) {
            Napi::TypeError::New(env, "Expected path argument").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        
        std::string path = info[0].As<Napi::String>().Utf8Value();
        auto music = audio->loadMusic(path);
        
        return music ? Napi::String::New(env, path) : env.Undefined();
    }
    
    Napi::Value GetWidth(const Napi::CallbackInfo& info) {
        return Napi::Number::New(info.Env(), windowWidth);
    }
    
    Napi::Value GetHeight(const Napi::CallbackInfo& info) {
        return Napi::Number::New(info.Env(), windowHeight);
    }
    
    Napi::Value SetTitle(const Napi::CallbackInfo& info) {
        if (info.Length() >= 1) {
            windowTitle = info[0].As<Napi::String>().Utf8Value();
            SDL_SetWindowTitle(window, windowTitle.c_str());
        }
        return info.Env().Undefined();
    }
    
    Napi::Value SetFullscreen(const Napi::CallbackInfo& info) {
        if (info.Length() >= 1) {
            bool fs = info[0].As<Napi::Boolean>().Value();
            SDL_SetWindowFullscreen(window, fs ? SDL_WINDOW_FULLSCREEN : 0);
            fullscreen = fs;
        }
        return info.Env().Undefined();
    }
    
    Napi::Value Quit(const Napi::CallbackInfo& info) {
        running = false;
        return info.Env().Undefined();
    }
    
    Napi::Value IsKeyDown(const Napi::CallbackInfo& info) {
        if (info.Length() >= 1) {
            int key = info[0].As<Napi::Number>().Int32Value();
            return Napi::Boolean::New(info.Env(), input->isDown(key));
        }
        return Napi::Boolean::New(info.Env(), false);
    }
    
    Napi::Value IsKeyPressed(const Napi::CallbackInfo& info) {
        if (info.Length() >= 1) {
            int key = info[0].As<Napi::Number>().Int32Value();
            return Napi::Boolean::New(info.Env(), input->isPressed(key));
        }
        return Napi::Boolean::New(info.Env(), false);
    }
    
    Napi::Value IsKeyReleased(const Napi::CallbackInfo& info) {
        if (info.Length() >= 1) {
            int key = info[0].As<Napi::Number>().Int32Value();
            return Napi::Boolean::New(info.Env(), input->isReleased(key));
        }
        return Napi::Boolean::New(info.Env(), false);
    }
    
    Napi::Value IsMouseDown(const Napi::CallbackInfo& info) {
        if (info.Length() >= 1) {
            int button = info[0].As<Napi::Number>().Int32Value();
            return Napi::Boolean::New(info.Env(), input->isMouseDown(button));
        }
        return Napi::Boolean::New(info.Env(), false);
    }
    
    Napi::Value IsMousePressed(const Napi::CallbackInfo& info) {
        if (info.Length() >= 1) {
            int button = info[0].As<Napi::Number>().Int32Value();
            return Napi::Boolean::New(info.Env(), input->isMousePressed(button));
        }
        return Napi::Boolean::New(info.Env(), false);
    }
    
    Napi::Value IsMouseReleased(const Napi::CallbackInfo& info) {
        if (info.Length() >= 1) {
            int button = info[0].As<Napi::Number>().Int32Value();
            return Napi::Boolean::New(info.Env(), input->isMouseReleased(button));
        }
        return Napi::Boolean::New(info.Env(), false);
    }
    
    Napi::Value GetMouseX(const Napi::CallbackInfo& info) {
        return Napi::Number::New(info.Env(), input->getMousePosition().x);
    }
    
    Napi::Value GetMouseY(const Napi::CallbackInfo& info) {
        return Napi::Number::New(info.Env(), input->getMousePosition().y);
    }
    
    Napi::Value GetMousePosition(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        Vec2 pos = input->getMousePosition();
        Napi::Object obj = Napi::Object::New(env);
        obj.Set("x", pos.x);
        obj.Set("y", pos.y);
        return obj;
    }
    
    Napi::Value GetDelta(const Napi::CallbackInfo& info) {
        return Napi::Number::New(info.Env(), timer->getDelta());
    }
    
    Napi::Value GetTime(const Napi::CallbackInfo& info) {
        return Napi::Number::New(info.Env(), timer->getTime());
    }
    
    Napi::Value GetFPS(const Napi::CallbackInfo& info) {
        return Napi::Number::New(info.Env(), timer->getFPS());
    }
    
    Napi::Value Clear(const Napi::CallbackInfo& info) {
        if (info.Length() >= 4) {
            Color color(
                info[0].As<Napi::Number>().Uint32Value(),
                info[1].As<Napi::Number>().Uint32Value(),
                info[2].As<Napi::Number>().Uint32Value(),
                info[3].As<Napi::Number>().Uint32Value()
            );
            graphics->clear(color);
        } else if (info.Length() >= 3) {
            Color color(
                info[0].As<Napi::Number>().Uint32Value(),
                info[1].As<Napi::Number>().Uint32Value(),
                info[2].As<Napi::Number>().Uint32Value()
            );
            graphics->clear(color);
        } else {
            graphics->clear();
        }
        return info.Env().Undefined();
    }
    
    Napi::Value SetColor(const Napi::CallbackInfo& info) {
        if (info.Length() >= 4) {
            Color color(
                info[0].As<Napi::Number>().Uint32Value(),
                info[1].As<Napi::Number>().Uint32Value(),
                info[2].As<Napi::Number>().Uint32Value(),
                info[3].As<Napi::Number>().Uint32Value()
            );
            graphics->setColor(color);
        } else if (info.Length() >= 3) {
            Color color(
                info[0].As<Napi::Number>().Uint32Value(),
                info[1].As<Napi::Number>().Uint32Value(),
                info[2].As<Napi::Number>().Uint32Value()
            );
            graphics->setColor(color);
        }
        return info.Env().Undefined();
    }
    
    Napi::Value DrawPoint(const Napi::CallbackInfo& info) {
        if (info.Length() >= 2) {
            Vec2 pos(
                info[0].As<Napi::Number>().FloatValue(),
                info[1].As<Napi::Number>().FloatValue()
            );
            graphics->drawPoint(pos);
        }
        return info.Env().Undefined();
    }
    
    Napi::Value DrawLine(const Napi::CallbackInfo& info) {
        if (info.Length() >= 4) {
            Vec2 start(
                info[0].As<Napi::Number>().FloatValue(),
                info[1].As<Napi::Number>().FloatValue()
            );
            Vec2 end(
                info[2].As<Napi::Number>().FloatValue(),
                info[3].As<Napi::Number>().FloatValue()
            );
            graphics->drawLine(start, end);
        }
        return info.Env().Undefined();
    }
    
    Napi::Value DrawRect(const Napi::CallbackInfo& info) {
        if (info.Length() >= 4) {
            Vec2 pos(
                info[0].As<Napi::Number>().FloatValue(),
                info[1].As<Napi::Number>().FloatValue()
            );
            Vec2 size(
                info[2].As<Napi::Number>().FloatValue(),
                info[3].As<Napi::Number>().FloatValue()
            );
            bool filled = info.Length() >= 5 ? info[4].As<Napi::Boolean>().Value() : false;
            graphics->drawRect(pos, size, filled);
        }
        return info.Env().Undefined();
    }
    
    Napi::Value DrawCircle(const Napi::CallbackInfo& info) {
        if (info.Length() >= 3) {
            Vec2 center(
                info[0].As<Napi::Number>().FloatValue(),
                info[1].As<Napi::Number>().FloatValue()
            );
            float radius = info[2].As<Napi::Number>().FloatValue();
            bool filled = info.Length() >= 4 ? info[3].As<Napi::Boolean>().Value() : false;
            graphics->drawCircle(center, radius, filled);
        }
        return info.Env().Undefined();
    }
    
    Napi::Value DrawTexture(const Napi::CallbackInfo& info) {
        if (info.Length() >= 3) {
            std::string path = info[0].As<Napi::String>().Utf8Value();
            auto it = textures.find(path);
            if (it != textures.end()) {
                Transform transform;
                transform.position.x = info[1].As<Napi::Number>().FloatValue();
                transform.position.y = info[2].As<Napi::Number>().FloatValue();
                
                if (info.Length() >= 4) transform.rotation = info[3].As<Napi::Number>().FloatValue();
                if (info.Length() >= 6) {
                    transform.scale.x = info[4].As<Napi::Number>().FloatValue();
                    transform.scale.y = info[5].As<Napi::Number>().FloatValue();
                }
                if (info.Length() >= 8) {
                    transform.origin.x = info[6].As<Napi::Number>().FloatValue();
                    transform.origin.y = info[7].As<Napi::Number>().FloatValue();
                }
                
                Color tint(255, 255, 255, 255);
                if (info.Length() >= 12) {
                    tint.r = info[8].As<Napi::Number>().Uint32Value();
                    tint.g = info[9].As<Napi::Number>().Uint32Value();
                    tint.b = info[10].As<Napi::Number>().Uint32Value();
                    tint.a = info[11].As<Napi::Number>().Uint32Value();
                }
                
                graphics->drawTexture(it->second, transform, tint);
            }
        }
        return info.Env().Undefined();
    }
    
    Napi::Value DrawText(const Napi::CallbackInfo& info) {
        if (info.Length() >= 3) {
            std::string text = info[0].As<Napi::String>().Utf8Value();
            Vec2 pos(
                info[1].As<Napi::Number>().FloatValue(),
                info[2].As<Napi::Number>().FloatValue()
            );
            
            Color color(255, 255, 255, 255);
            if (info.Length() >= 7) {
                color.r = info[3].As<Napi::Number>().Uint32Value();
                color.g = info[4].As<Napi::Number>().Uint32Value();
                color.b = info[5].As<Napi::Number>().Uint32Value();
                color.a = info[6].As<Napi::Number>().Uint32Value();
            }
            
            graphics->drawText(text, pos, color);
        }
        return info.Env().Undefined();
    }
    
    Napi::Value SetFont(const Napi::CallbackInfo& info) {
        if (info.Length() >= 1) {
            std::string key = info[0].As<Napi::String>().Utf8Value();
            auto it = fonts.find(key);
            if (it != fonts.end()) {
                graphics->setFont(it->second);
            }
        }
        return info.Env().Undefined();
    }
    
    Napi::Value PlaySound(const Napi::CallbackInfo& info) {
        if (info.Length() >= 1) {
            std::string path = info[0].As<Napi::String>().Utf8Value();
            auto sound = audio->loadSound(path);
            if (sound) {
                int volume = info.Length() >= 2 ? info[1].As<Napi::Number>().Int32Value() : 128;
                audio->playSound(sound, volume);
            }
        }
        return info.Env().Undefined();
    }
    
    Napi::Value PlayMusic(const Napi::CallbackInfo& info) {
        if (info.Length() >= 1) {
            std::string path = info[0].As<Napi::String>().Utf8Value();
            auto music = audio->loadMusic(path);
            if (music) {
                int loops = info.Length() >= 2 ? info[1].As<Napi::Number>().Int32Value() : -1;
                audio->playMusic(music, loops);
            }
        }
        return info.Env().Undefined();
    }
    
    Napi::Value StopMusic(const Napi::CallbackInfo& info) {
        audio->stopMusic();
        return info.Env().Undefined();
    }
    
    Napi::Value SetMusicVolume(const Napi::CallbackInfo& info) {
        if (info.Length() >= 1) {
            int volume = info[0].As<Napi::Number>().Int32Value();
            audio->setMusicVolume(volume);
        }
        return info.Env().Undefined();
    }
    
    Napi::Value RandomInt(const Napi::CallbackInfo& info) {
        if (info.Length() >= 2) {
            int min = info[0].As<Napi::Number>().Int32Value();
            int max = info[1].As<Napi::Number>().Int32Value();
            return Napi::Number::New(info.Env(), random->randomInt(min, max));
        }
        return Napi::Number::New(info.Env(), random->randomInt(0, 1));
    }
    
    Napi::Value RandomFloat(const Napi::CallbackInfo& info) {
        if (info.Length() >= 2) {
            float min = info[0].As<Napi::Number>().FloatValue();
            float max = info[1].As<Napi::Number>().FloatValue();
            return Napi::Number::New(info.Env(), random->randomFloat(min, max));
        }
        return Napi::Number::New(info.Env(), random->randomFloat());
    }
    
    Napi::Value RandomBool(const Napi::CallbackInfo& info) {
        return Napi::Boolean::New(info.Env(), random->randomBool());
    }
};

Napi::Object CreateTensai(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 5) {
        Napi::TypeError::New(env, "Expected 5 arguments").ThrowAsJavaScriptException();
        return Napi::Object::New(env);
    }
    
    Napi::FunctionReference* constructor = env.GetInstanceData<Napi::FunctionReference>();
    return constructor->New({
        info[0], info[1], info[2], info[3], info[4]
    });
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    TensaiEngine::Init(env, exports);
    exports.Set("Tensai", Napi::Function::New(env, CreateTensai));
    
    Napi::Object keys = Napi::Object::New(env);
    keys.Set("A", Napi::Number::New(env, SDLK_a));
    keys.Set("B", Napi::Number::New(env, SDLK_b));
    keys.Set("C", Napi::Number::New(env, SDLK_c));
    keys.Set("D", Napi::Number::New(env, SDLK_d));
    keys.Set("E", Napi::Number::New(env, SDLK_e));
    keys.Set("F", Napi::Number::New(env, SDLK_f));
    keys.Set("G", Napi::Number::New(env, SDLK_g));
    keys.Set("H", Napi::Number::New(env, SDLK_h));
    keys.Set("I", Napi::Number::New(env, SDLK_i));
    keys.Set("J", Napi::Number::New(env, SDLK_j));
    keys.Set("K", Napi::Number::New(env, SDLK_k));
    keys.Set("L", Napi::Number::New(env, SDLK_l));
    keys.Set("M", Napi::Number::New(env, SDLK_m));
    keys.Set("N", Napi::Number::New(env, SDLK_n));
    keys.Set("O", Napi::Number::New(env, SDLK_o));
    keys.Set("P", Napi::Number::New(env, SDLK_p));
    keys.Set("Q", Napi::Number::New(env, SDLK_q));
    keys.Set("R", Napi::Number::New(env, SDLK_r));
    keys.Set("S", Napi::Number::New(env, SDLK_s));
    keys.Set("T", Napi::Number::New(env, SDLK_t));
    keys.Set("U", Napi::Number::New(env, SDLK_u));
    keys.Set("V", Napi::Number::New(env, SDLK_v));
    keys.Set("W", Napi::Number::New(env, SDLK_w));
    keys.Set("X", Napi::Number::New(env, SDLK_x));
    keys.Set("Y", Napi::Number::New(env, SDLK_y));
    keys.Set("Z", Napi::Number::New(env, SDLK_z));
    keys.Set("SPACE", Napi::Number::New(env, SDLK_SPACE));
    keys.Set("RETURN", Napi::Number::New(env, SDLK_RETURN));
    keys.Set("ESCAPE", Napi::Number::New(env, SDLK_ESCAPE));
    keys.Set("UP", Napi::Number::New(env, SDLK_UP));
    keys.Set("DOWN", Napi::Number::New(env, SDLK_DOWN));
    keys.Set("LEFT", Napi::Number::New(env, SDLK_LEFT));
    keys.Set("RIGHT", Napi::Number::New(env, SDLK_RIGHT));
    keys.Set("LSHIFT", Napi::Number::New(env, SDLK_LSHIFT));
    keys.Set("RSHIFT", Napi::Number::New(env, SDLK_RSHIFT));
    keys.Set("LCTRL", Napi::Number::New(env, SDLK_LCTRL));
    keys.Set("RCTRL", Napi::Number::New(env, SDLK_RCTRL));
    keys.Set("LALT", Napi::Number::New(env, SDLK_LALT));
    keys.Set("RALT", Napi::Number::New(env, SDLK_RALT));
    keys.Set("TAB", Napi::Number::New(env, SDLK_TAB));
    keys.Set("BACKSPACE", Napi::Number::New(env, SDLK_BACKSPACE));
    keys.Set("DELETE", Napi::Number::New(env, SDLK_DELETE));
    keys.Set("HOME", Napi::Number::New(env, SDLK_HOME));
    keys.Set("END", Napi::Number::New(env, SDLK_END));
    keys.Set("PAGEUP", Napi::Number::New(env, SDLK_PAGEUP));
    keys.Set("PAGEDOWN", Napi::Number::New(env, SDLK_PAGEDOWN));
    keys.Set("F1", Napi::Number::New(env, SDLK_F1));
    keys.Set("F2", Napi::Number::New(env, SDLK_F2));
    keys.Set("F3", Napi::Number::New(env, SDLK_F3));
    keys.Set("F4", Napi::Number::New(env, SDLK_F4));
    keys.Set("F5", Napi::Number::New(env, SDLK_F5));
    keys.Set("F6", Napi::Number::New(env, SDLK_F6));
    keys.Set("F7", Napi::Number::New(env, SDLK_F7));
    keys.Set("F8", Napi::Number::New(env, SDLK_F8));
    keys.Set("F9", Napi::Number::New(env, SDLK_F9));
    keys.Set("F10", Napi::Number::New(env, SDLK_F10));
    keys.Set("F11", Napi::Number::New(env, SDLK_F11));
    keys.Set("F12", Napi::Number::New(env, SDLK_F12));
    
    for (int i = 0; i <= 9; i++) {
        keys.Set(std::to_string(i), Napi::Number::New(env, SDLK_0 + i));
    }
    
    exports.Set("Keys", keys);
    
    Napi::Object mouse = Napi::Object::New(env);
    mouse.Set("LEFT", Napi::Number::New(env, SDL_BUTTON_LEFT));
    mouse.Set("MIDDLE", Napi::Number::New(env, SDL_BUTTON_MIDDLE));
    mouse.Set("RIGHT", Napi::Number::New(env, SDL_BUTTON_RIGHT));
    exports.Set("Mouse", mouse);
    
    return exports;
}

NODE_API_MODULE(tensai, Init)
