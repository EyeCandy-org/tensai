{
  "targets": [
    {
      "target_name": "tensai",
      "sources": [
        "src/tensai.cpp",
        "src/core/vec2.cpp",
        "src/core/color.cpp",
        "src/core/transform.cpp",
        "src/resources/texture.cpp",
        "src/resources/font.cpp",
        "src/resources/sound.cpp",
        "src/resources/music.cpp",
        "src/modules/input.cpp",
        "src/modules/camera.cpp",
        "src/modules/graphics.cpp",
        "src/modules/timer.cpp",
        "src/modules/random.cpp",
        "src/modules/physics.cpp",
        "src/modules/audio.cpp",
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")"
      ],
      "dependencies": [
        "<!(node -p \"require('node-addon-api').gyp\")"
      ],
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "cflags_cc": [ "-std=c++17" ],
      "defines": [ "NAPI_DISABLE_CPP_EXCEPTIONS" ],
      "conditions": [
        [
          "OS=='win'",
          {
            "libraries": [
              "-lSDL2",
              "-lSDL2_image", 
              "-lSDL2_ttf",
              "-lSDL2_mixer"
            ],
            "include_dirs": [
              "C:/SDL2/include",
              "C:/SDL2_image/include",
              "C:/SDL2_ttf/include", 
              "C:/SDL2_mixer/include"
            ],
            "library_dirs": [
              "C:/SDL2/lib/x64",
              "C:/SDL2_image/lib/x64",
              "C:/SDL2_ttf/lib/x64",
              "C:/SDL2_mixer/lib/x64"
            ]
          }
        ],
        [
          "OS=='mac'",
          {
            "libraries": [
              "-lSDL2",
              "-lSDL2_image",
              "-lSDL2_ttf", 
              "-lSDL2_mixer"
            ],
            "include_dirs": [
              "/usr/local/include/SDL2",
              "/opt/homebrew/include/SDL2"
            ],
            "library_dirs": [
              "/usr/local/lib",
              "/opt/homebrew/lib"
            ],
            "xcode_settings": {
              "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
              "CLANG_CXX_LIBRARY": "libc++",
              "MACOSX_DEPLOYMENT_TARGET": "10.15"
            }
          }
        ],
        [
          "OS=='linux'",
          {
            "libraries": [
              "-lSDL2",
              "-lSDL2_image",
              "-lSDL2_ttf",
              "-lSDL2_mixer"
            ],
            "cflags": [
              "<!@(pkg-config --cflags sdl2)",
              "-std=c++17"
            ],
            "ldflags": [
              "<!@(pkg-config --libs sdl2 SDL2_image SDL2_ttf SDL2_mixer)"
            ]
          }
        ]
      ]
    }
  ]
}
