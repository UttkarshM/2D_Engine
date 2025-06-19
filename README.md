# 2D Game Engine

A custom 2D game engine built in C++ with OpenGL rendering, featuring hot-reloadable game logic, tile-based level editing, physics simulation, and audio support.

## 🎬 Demo

![Demo](./demo.mp4)

*Watch the engine in action: platformer physics, tile-based level editing, and real-time gameplay.*

## 🎮 Features

### Core Engine
- **Custom 2D Renderer**: OpenGL-based sprite rendering with animation support
- **Hot Reload System**: Dynamic library loading for instant game logic updates during development
- **Memory Management**: Custom bump allocator for efficient memory usage
- **Cross-Platform**: Supports both Windows and Linux
- **Audio System**: WAV file playback with sound effects
- **Input Handling**: Keyboard and mouse input with customizable key mappings

### Game Features
- **Platformer Physics**: Character movement with jumping, gravity, and collision detection
- **Tile-Based Level Editor**: In-game level editing with mouse controls
- **Moving Platforms**: Dynamic solid objects with keyframe-based movement
- **Collision System**: Tile-based and sprite-based collision detection
- **Animation System**: Sprite-based character animations (idle, run, jump)
- **Level Persistence**: Save and load tile-based levels

### Technical Highlights
- **Entity Component System**: Modular game object architecture
- **Fixed Timestep Simulation**: Consistent physics updates with interpolated rendering
- **Automatic Bitmasking**: Smart tile rendering based on neighboring tiles
- **Real-time Debugging**: Console logging with color-coded messages

## 🚀 Getting Started

### Prerequisites
- **Clang++** compiler
- **OpenGL** development libraries
- **Git** (for version control)
- **Windows**: Visual Studio Build Tools or MinGW
- **Linux**: Standard development packages

### Building the Project

1. **Clone the repository**:
   ```bash
   git clone <repository-url>
   cd god
   ```

2. **Build and run**:
   ```bash
   ./build.sh
   ```
   
   The build script will:
   - Compile the game logic as a dynamic library
   - Build the main engine executable
   - Automatically clean up build artifacts (.ilk, .lib, .pdb, .exp files)
   - Only keep the essential `engine.exe` file

3. **Run the game**:
   ```bash
   ./engine.exe
   ```

## 🎮 Controls

### Player Movement
- **WASD** or **Arrow Keys**: Move left/right, jump
- **Space**: Jump

### Level Editor
- **Left Mouse**: Place tiles
- **Right Mouse**: Remove tiles
- **Middle Mouse**: Load saved level
- **Semicolon (;)**: Save current level

## 📁 Project Structure

```
god/
├── src/
│   ├── main.cpp              # Engine entry point and main loop
│   ├── libs/
│   │   ├── game.cpp          # Game logic (hot-reloadable)
│   │   ├── game.h            # Game interface
│   │   ├── game_libs.h       # Game-specific utilities
│   │   ├── core.h            # Core engine definitions
│   │   ├── engine_libs.h     # Engine utilities and math
│   │   ├── platform.h        # Platform abstraction
│   │   ├── gl_renderer.cpp   # OpenGL rendering
│   │   ├── input.h           # Input handling
│   │   ├── sound.h           # Audio system
│   │   └── sprites.h         # Sprite management
│   └── tiles/
│       └── test2.tile        # Level data
├── assets/
│   ├── shaders/              # GLSL shader files
│   ├── sounds/               # Audio files (.wav)
│   └── texture/              # Sprite textures
├── third_party/              # External dependencies
├── build.sh                  # Build script
└── README.md
```

## 🎨 Art Assets

### Creating Pixel Art
For creating 2D pixel art assets, we recommend:
- **Web-based**: [Piskel](https://www.piskelapp.com/)
- **Desktop**: Aseprite, GraphicsGale, or similar pixel art tools

### Color Palette
The project uses a cohesive color palette:
- `#002447` - Dark blue
- `#ce4a32` - Red
- `#e8b87e` - Light brown
- `#f39423` - Orange

## ⚙️ Configuration

### World Settings
- **World Size**: 320x180 pixels
- **Tile Size**: 8x8 pixels
- **Update Rate**: 60 FPS fixed timestep
- **Audio**: 44.1kHz, 2 channels

### Physics Constants
```cpp
static const float runSpeed = 2.0f;
static const float gravity = 13.0f;
static const float jumpSpeed = -3.0f;
```

## 🔧 Development

### Hot Reloading
The engine supports hot reloading of game logic:
1. Modify `src/libs/game.cpp`
2. Run `./build.sh`
3. The game will automatically reload the new logic without restarting

### Adding New Features
1. **Sprites**: Add new sprite definitions in `sprites.h`
2. **Sounds**: Place `.wav` files in `assets/sounds/`
3. **Game Objects**: Extend the GameState structure in `game_libs.h`
4. **Input**: Add new key mappings in the `game_keyMappings()` function

### Debug Logging
The engine provides colored console output:
```cpp
EN_TRACE("Debug information");    // Green
EN_WARN("Warning message");       // Yellow
EN_ERROR("Error message");        // Red
```

## 🐛 Troubleshooting

### Build Issues
- Ensure Clang++ is installed and in your PATH
- Check that OpenGL development libraries are available
- Verify all source files are present and readable


### Runtime Issues
- Check `build_errors.txt` for compilation errors
- Ensure asset files are in the correct directories
- Verify audio files are in WAV format (44.1kHz, 16-bit, stereo)

## 📄 License

This project is a custom game engine for educational and development purposes.

**Engine Version**: Custom 2D Engine v1.0  
**Last Updated**: June 2025
