<div align="center">
  <img src="https://github.com/user-attachments/assets/fdbf081f-cd08-4baa-94d8-337e8ffeecd2" alt="Tensai <3" width="100%" />
</div>
---
<p align="center">
  <b>Tensai</b> is a powerful <b>C++ game engine</b> with seamless <b>Node.js bindings</b>, 
  designed to help developers build <i>engaging games</i> and <i>rich multimedia apps</i>.
</p>

## Features

| Module    | What it does |
|-----------|--------------|
| **Graphics** | Advanced rendering for stunning visuals |
| **Audio**    | Full sound & music management |
| **Input**    | Flexible handling for keyboards, mice, controllers |
| **Physics**  | Built-in physics for realistic interactions |
| **Camera**   | Intuitive, dynamic scene controls |
| **Timer**    | High-precision loops & animation timing |
| **Random**   | Robust RNG for unpredictability |
| **Resources**| Efficient asset management (fonts, music, textures) |


## Installation

To get started with the Tensai Engine, follow these simple steps:

1.  **Clone the repository:**

    ```bash
    git clone https://github.com/neoapps-dev/tensai.git
    cd tensai
    ```

2.  **Install Node.js dependencies:**

    ```bash
    npm install
    ```

    This command will automatically compile the native C++ addon using `node-gyp`.

## Usage

Here's a minimal example demonstrating how to set up and run a basic Tensai application:

```javascript
const { Tensai } = require('./index.js');

// Initialize the Tensai engine with a window title, width, height, and fullscreen/vsync options
const tensai = Tensai("My Tensai App", 800, 600, false, true);

// Define the load function (optional, for asset loading)
tensai.load = () => {
  console.log("App assets loaded!");
  // Load your textures, sounds, fonts here
};

// Define the update function (called every frame for game logic)
tensai.update = (dt) => {
  // dt is the delta time (time elapsed since last frame) in seconds
  // console.log(`Updating with delta time: ${dt.toFixed(4)}s`);
  // Update game state, handle input, etc.
};

// Define the draw function (called every frame for rendering)
tensai.draw = () => {
  // Clear the screen with a background color (e.g., dark blue)
  tensai.clear(20, 25, 40);

  // Draw your game objects here
  tensai.setColor(255, 255, 255);
  tensai.drawText("Hello, Tensai!", 10, 10);
};

// Start the Tensai engine game loop
tensai.run();

console.log("Tensai application started. Check the window!");
```

This example initializes a window, sets up basic `load`, `update`, and `draw` functions, and starts the engine loop. For a more comprehensive demonstration of the engine's capabilities, please refer to the examples section.

## Examples

The `test/test.js` file serves as a runnable example showcasing various features of the Tensai Engine, including graphics rendering, input handling, sound, music, and particle effects.

To run this example, navigate to the project root and execute:

```bash
node test/test.js
```

**Note:** This example may require the assets located in the `assets/` directory to run correctly.

## Building

The C++ addon is automatically built when you run `npm install`. This process is orchestrated by `node-gyp` using the `binding.gyp` configuration.

For manual rebuilding, you can use:

```bash
node-gyp rebuild
```

**Prerequisites:** Ensure you have the necessary build tools installed for your operating system:
-   **Windows:** Visual Studio (with C++ build tools)
-   **macOS:** Xcode Command Line Tools
-   **GNU/Linux:** `build-essential` package (or equivalent)

## License

This project is proudly distributed under the [LGPL-3.0 License](LICENSE).
