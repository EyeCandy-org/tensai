# Tensai Engine

![GitHub last commit](https://img.shields.io/github/last-commit/neoapps-dev/tensai)
![GitHub issues](https://img.shields.io/github/issues/neoapps-dev/tensai)
![License](https://img.shields.io/badge/License-LGPL--3.0-blue.svg)

<img width="1200" height="480" alt="Tensai <3" src="https://github.com/user-attachments/assets/fdbf081f-cd08-4baa-94d8-337e8ffeecd2" />


Tensai is a powerful and versatile C++ library with seamless Node.js bindings, meticulously crafted to empower developers in creating engaging games and rich multimedia applications. It provides a robust foundation with essential modules for graphics, audio, input, physics, and more.

## Features

-   **Graphics:** Advanced rendering capabilities for stunning visual experiences.
-   **Audio:** Comprehensive sound and music management for immersive auditory environments.
-   **Input:** Flexible handling of user input across various devices.
-   **Physics:** Integrated physics engine for realistic object interactions and simulations.
-   **Camera:** Intuitive camera controls for dynamic scene perspectives.
-   **Timer:** High-precision timing utilities for game loops and animations.
-   **Random:** Robust random number generation for unpredictable game elements.
-   **Resources:** Efficient management of game assets including fonts, music, sounds, and textures.

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
