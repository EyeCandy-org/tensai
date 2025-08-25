#include "core/color.h"
#include "core/transform.h"
#include "core/vec2.h"
#include "modules/audio.h"
#include "modules/camera.h"
#include "modules/graphics.h"
#include "modules/input.h"
#include "modules/physics.h"
#include "modules/random.h"
#include "modules/timer.h"
#include "resources/font.h"
#include "resources/music.h"
#include "resources/sound.h"
#include "resources/texture.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <memory>
#include <napi.h>
#include <random>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
class TensaiEngine : public Napi::ObjectWrap<TensaiEngine> {
private:
  SDL_Window *window = nullptr;
  SDL_Renderer *renderer = nullptr;
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
    Napi::Function func = DefineClass(
        env, "TensaiEngine",
        {
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
    Napi::FunctionReference *constructor = new Napi::FunctionReference();
    *constructor = Napi::Persistent(func);
    env.SetInstanceData(constructor);
    exports.Set("TensaiEngine", func);
    return exports;
  }

  TensaiEngine(const Napi::CallbackInfo &info)
      : Napi::ObjectWrap<TensaiEngine>(info) {
    Napi::Env env = info.Env();
    if (info.Length() < 5) {
      Napi::TypeError::New(
          env, "Expected 5 arguments: title, width, height, fullscreen, vsync")
          .ThrowAsJavaScriptException();
      return;
    }

    windowTitle = info[0].As<Napi::String>().Utf8Value();
    windowWidth = info[1].As<Napi::Number>().Int32Value();
    windowHeight = info[2].As<Napi::Number>().Int32Value();
    fullscreen = info[3].As<Napi::Boolean>().Value();
    vsync = info[4].As<Napi::Boolean>().Value();
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS) != 0) {
      Napi::Error::New(env, "Failed to initialize SDL")
          .ThrowAsJavaScriptException();
      return;
    }

    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
    TTF_Init();
    Uint32 windowFlags = SDL_WINDOW_SHOWN;
    if (fullscreen)
      windowFlags |= SDL_WINDOW_FULLSCREEN;
    window = SDL_CreateWindow(windowTitle.c_str(), SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight,
                              windowFlags);

    if (!window) {
      Napi::Error::New(env, "Failed to create window")
          .ThrowAsJavaScriptException();
      return;
    }

    Uint32 rendererFlags = SDL_RENDERER_ACCELERATED;
    if (vsync)
      rendererFlags |= SDL_RENDERER_PRESENTVSYNC;
    renderer = SDL_CreateRenderer(window, -1, rendererFlags);
    if (!renderer) {
      Napi::Error::New(env, "Failed to create renderer")
          .ThrowAsJavaScriptException();
      return;
    }

    graphics = std::make_unique<Graphics>(renderer);
    input = std::make_unique<Input>();
    timer = std::make_unique<Timer>();
    random = std::make_unique<Random>();
    audio = std::make_unique<Audio>();
  }

  ~TensaiEngine() {
    if (renderer)
      SDL_DestroyRenderer(renderer);
    if (window)
      SDL_DestroyWindow(window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
  }

  void SetLoad(const Napi::CallbackInfo &info, const Napi::Value &value) {
    loadCallback = Napi::Persistent(value.As<Napi::Function>());
  }

  void SetUpdate(const Napi::CallbackInfo &info, const Napi::Value &value) {
    updateCallback = Napi::Persistent(value.As<Napi::Function>());
  }

  void SetDraw(const Napi::CallbackInfo &info, const Napi::Value &value) {
    drawCallback = Napi::Persistent(value.As<Napi::Function>());
  }

  Napi::Value Run(const Napi::CallbackInfo &info) {
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

  Napi::Value LoadTexture(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    if (info.Length() < 1) {
      Napi::TypeError::New(env, "Expected path argument")
          .ThrowAsJavaScriptException();
      return env.Undefined();
    }

    std::string path = info[0].As<Napi::String>().Utf8Value();
    SDL_Surface *surface = IMG_Load(path.c_str());
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

  Napi::Value LoadFont(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    if (info.Length() < 2) {
      Napi::TypeError::New(env, "Expected path and size arguments")
          .ThrowAsJavaScriptException();
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

  Napi::Value LoadSound(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    if (info.Length() < 1) {
      Napi::TypeError::New(env, "Expected path argument")
          .ThrowAsJavaScriptException();
      return env.Undefined();
    }

    std::string path = info[0].As<Napi::String>().Utf8Value();
    auto sound = audio->loadSound(path);
    return sound ? Napi::String::New(env, path) : env.Undefined();
  }

  Napi::Value LoadMusic(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    if (info.Length() < 1) {
      Napi::TypeError::New(env, "Expected path argument")
          .ThrowAsJavaScriptException();
      return env.Undefined();
    }

    std::string path = info[0].As<Napi::String>().Utf8Value();
    auto music = audio->loadMusic(path);
    return music ? Napi::String::New(env, path) : env.Undefined();
  }

  Napi::Value GetWidth(const Napi::CallbackInfo &info) {
    return Napi::Number::New(info.Env(), windowWidth);
  }

  Napi::Value GetHeight(const Napi::CallbackInfo &info) {
    return Napi::Number::New(info.Env(), windowHeight);
  }

  Napi::Value SetTitle(const Napi::CallbackInfo &info) {
    if (info.Length() >= 1) {
      windowTitle = info[0].As<Napi::String>().Utf8Value();
      SDL_SetWindowTitle(window, windowTitle.c_str());
    }
    return info.Env().Undefined();
  }

  Napi::Value SetFullscreen(const Napi::CallbackInfo &info) {
    if (info.Length() >= 1) {
      bool fs = info[0].As<Napi::Boolean>().Value();
      SDL_SetWindowFullscreen(window, fs ? SDL_WINDOW_FULLSCREEN : 0);
      fullscreen = fs;
    }
    return info.Env().Undefined();
  }

  Napi::Value Quit(const Napi::CallbackInfo &info) {
    running = false;
    return info.Env().Undefined();
  }

  Napi::Value IsKeyDown(const Napi::CallbackInfo &info) {
    if (info.Length() >= 1) {
      int key = info[0].As<Napi::Number>().Int32Value();
      return Napi::Boolean::New(info.Env(), input->isDown(key));
    }
    return Napi::Boolean::New(info.Env(), false);
  }

  Napi::Value IsKeyPressed(const Napi::CallbackInfo &info) {
    if (info.Length() >= 1) {
      int key = info[0].As<Napi::Number>().Int32Value();
      return Napi::Boolean::New(info.Env(), input->isPressed(key));
    }
    return Napi::Boolean::New(info.Env(), false);
  }

  Napi::Value IsKeyReleased(const Napi::CallbackInfo &info) {
    if (info.Length() >= 1) {
      int key = info[0].As<Napi::Number>().Int32Value();
      return Napi::Boolean::New(info.Env(), input->isReleased(key));
    }
    return Napi::Boolean::New(info.Env(), false);
  }

  Napi::Value IsMouseDown(const Napi::CallbackInfo &info) {
    if (info.Length() >= 1) {
      int button = info[0].As<Napi::Number>().Int32Value();
      return Napi::Boolean::New(info.Env(), input->isMouseDown(button));
    }
    return Napi::Boolean::New(info.Env(), false);
  }

  Napi::Value IsMousePressed(const Napi::CallbackInfo &info) {
    if (info.Length() >= 1) {
      int button = info[0].As<Napi::Number>().Int32Value();
      return Napi::Boolean::New(info.Env(), input->isMousePressed(button));
    }
    return Napi::Boolean::New(info.Env(), false);
  }

  Napi::Value IsMouseReleased(const Napi::CallbackInfo &info) {
    if (info.Length() >= 1) {
      int button = info[0].As<Napi::Number>().Int32Value();
      return Napi::Boolean::New(info.Env(), input->isMouseReleased(button));
    }
    return Napi::Boolean::New(info.Env(), false);
  }

  Napi::Value GetMouseX(const Napi::CallbackInfo &info) {
    return Napi::Number::New(info.Env(), input->getMousePosition().x);
  }

  Napi::Value GetMouseY(const Napi::CallbackInfo &info) {
    return Napi::Number::New(info.Env(), input->getMousePosition().y);
  }

  Napi::Value GetMousePosition(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    Vec2 pos = input->getMousePosition();
    Napi::Object obj = Napi::Object::New(env);
    obj.Set("x", pos.x);
    obj.Set("y", pos.y);
    return obj;
  }

  Napi::Value GetDelta(const Napi::CallbackInfo &info) {
    return Napi::Number::New(info.Env(), timer->getDelta());
  }

  Napi::Value GetTime(const Napi::CallbackInfo &info) {
    return Napi::Number::New(info.Env(), timer->getTime());
  }

  Napi::Value GetFPS(const Napi::CallbackInfo &info) {
    return Napi::Number::New(info.Env(), timer->getFPS());
  }

  Napi::Value Clear(const Napi::CallbackInfo &info) {
    if (info.Length() >= 4) {
      Color color(info[0].As<Napi::Number>().Uint32Value(),
                  info[1].As<Napi::Number>().Uint32Value(),
                  info[2].As<Napi::Number>().Uint32Value(),
                  info[3].As<Napi::Number>().Uint32Value());
      graphics->clear(color);
    } else if (info.Length() >= 3) {
      Color color(info[0].As<Napi::Number>().Uint32Value(),
                  info[1].As<Napi::Number>().Uint32Value(),
                  info[2].As<Napi::Number>().Uint32Value());
      graphics->clear(color);
    } else {
      graphics->clear();
    }
    return info.Env().Undefined();
  }

  Napi::Value SetColor(const Napi::CallbackInfo &info) {
    if (info.Length() >= 4) {
      Color color(info[0].As<Napi::Number>().Uint32Value(),
                  info[1].As<Napi::Number>().Uint32Value(),
                  info[2].As<Napi::Number>().Uint32Value(),
                  info[3].As<Napi::Number>().Uint32Value());
      graphics->setColor(color);
    } else if (info.Length() >= 3) {
      Color color(info[0].As<Napi::Number>().Uint32Value(),
                  info[1].As<Napi::Number>().Uint32Value(),
                  info[2].As<Napi::Number>().Uint32Value());
      graphics->setColor(color);
    }
    return info.Env().Undefined();
  }

  Napi::Value DrawPoint(const Napi::CallbackInfo &info) {
    if (info.Length() >= 2) {
      Vec2 pos(info[0].As<Napi::Number>().FloatValue(),
               info[1].As<Napi::Number>().FloatValue());
      graphics->drawPoint(pos);
    }
    return info.Env().Undefined();
  }

  Napi::Value DrawLine(const Napi::CallbackInfo &info) {
    if (info.Length() >= 4) {
      Vec2 start(info[0].As<Napi::Number>().FloatValue(),
                 info[1].As<Napi::Number>().FloatValue());
      Vec2 end(info[2].As<Napi::Number>().FloatValue(),
               info[3].As<Napi::Number>().FloatValue());
      graphics->drawLine(start, end);
    }
    return info.Env().Undefined();
  }

  Napi::Value DrawRect(const Napi::CallbackInfo &info) {
    if (info.Length() >= 4) {
      Vec2 pos(info[0].As<Napi::Number>().FloatValue(),
               info[1].As<Napi::Number>().FloatValue());
      Vec2 size(info[2].As<Napi::Number>().FloatValue(),
                info[3].As<Napi::Number>().FloatValue());
      bool filled =
          info.Length() >= 5 ? info[4].As<Napi::Boolean>().Value() : false;
      graphics->drawRect(pos, size, filled);
    }
    return info.Env().Undefined();
  }

  Napi::Value DrawCircle(const Napi::CallbackInfo &info) {
    if (info.Length() >= 3) {
      Vec2 center(info[0].As<Napi::Number>().FloatValue(),
                  info[1].As<Napi::Number>().FloatValue());
      float radius = info[2].As<Napi::Number>().FloatValue();
      bool filled =
          info.Length() >= 4 ? info[3].As<Napi::Boolean>().Value() : false;
      graphics->drawCircle(center, radius, filled);
    }
    return info.Env().Undefined();
  }

  Napi::Value DrawTexture(const Napi::CallbackInfo &info) {
    if (info.Length() >= 3) {
      std::string path = info[0].As<Napi::String>().Utf8Value();
      auto it = textures.find(path);
      if (it != textures.end()) {
        Transform transform;
        transform.position.x = info[1].As<Napi::Number>().FloatValue();
        transform.position.y = info[2].As<Napi::Number>().FloatValue();
        if (info.Length() >= 4) {
          if (info.Length() >= 13 && info[12].IsString() &&
              info[12].As<Napi::String>().Utf8Value() == "degrees") {
            transform.setRotationDegrees(
                info[3].As<Napi::Number>().FloatValue());
          } else {
            transform.setRotation(info[3].As<Napi::Number>().FloatValue());
          }
        }
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

  Napi::Value DrawText(const Napi::CallbackInfo &info) {
    if (info.Length() >= 3) {
      std::string text = info[0].As<Napi::String>().Utf8Value();
      Vec2 pos(info[1].As<Napi::Number>().FloatValue(),
               info[2].As<Napi::Number>().FloatValue());

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

  Napi::Value SetFont(const Napi::CallbackInfo &info) {
    if (info.Length() >= 1) {
      std::string key = info[0].As<Napi::String>().Utf8Value();
      auto it = fonts.find(key);
      if (it != fonts.end()) {
        graphics->setFont(it->second);
      }
    }
    return info.Env().Undefined();
  }

  Napi::Value PlaySound(const Napi::CallbackInfo &info) {
    if (info.Length() >= 1) {
      std::string path = info[0].As<Napi::String>().Utf8Value();
      auto sound = audio->loadSound(path);
      if (sound) {
        int volume =
            info.Length() >= 2 ? info[1].As<Napi::Number>().Int32Value() : 128;
        audio->playSound(sound, volume);
      }
    }
    return info.Env().Undefined();
  }

  Napi::Value PlayMusic(const Napi::CallbackInfo &info) {
    if (info.Length() >= 1) {
      std::string path = info[0].As<Napi::String>().Utf8Value();
      auto music = audio->loadMusic(path);
      if (music) {
        int loops =
            info.Length() >= 2 ? info[1].As<Napi::Number>().Int32Value() : -1;
        audio->playMusic(music, loops);
      }
    }
    return info.Env().Undefined();
  }

  Napi::Value StopMusic(const Napi::CallbackInfo &info) {
    audio->stopMusic();
    return info.Env().Undefined();
  }

  Napi::Value SetMusicVolume(const Napi::CallbackInfo &info) {
    if (info.Length() >= 1) {
      int volume = info[0].As<Napi::Number>().Int32Value();
      audio->setMusicVolume(volume);
    }
    return info.Env().Undefined();
  }

  Napi::Value RandomInt(const Napi::CallbackInfo &info) {
    if (info.Length() >= 2) {
      int min = info[0].As<Napi::Number>().Int32Value();
      int max = info[1].As<Napi::Number>().Int32Value();
      return Napi::Number::New(info.Env(), random->randomInt(min, max));
    }
    return Napi::Number::New(info.Env(), random->randomInt(0, 1));
  }

  Napi::Value RandomFloat(const Napi::CallbackInfo &info) {
    if (info.Length() >= 2) {
      float min = info[0].As<Napi::Number>().FloatValue();
      float max = info[1].As<Napi::Number>().FloatValue();
      return Napi::Number::New(info.Env(), random->randomFloat(min, max));
    }
    return Napi::Number::New(info.Env(), random->randomFloat());
  }

  Napi::Value RandomBool(const Napi::CallbackInfo &info) {
    return Napi::Boolean::New(info.Env(), random->randomBool());
  }
};

Napi::Object CreateTensai(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (info.Length() < 5) {
    Napi::TypeError::New(env, "Expected 5 arguments")
        .ThrowAsJavaScriptException();
    return Napi::Object::New(env);
  }

  Napi::FunctionReference *constructor =
      env.GetInstanceData<Napi::FunctionReference>();
  return constructor->New({info[0], info[1], info[2], info[3], info[4]});
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
