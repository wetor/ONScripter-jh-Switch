<!-- OPENSPEC:START -->
# OpenSpec Instructions

These instructions are for AI assistants working in this project.

Always open `@/openspec/AGENTS.md` when the request:
- Mentions planning or proposals (words like proposal, spec, change, plan)
- Introduces new capabilities, breaking changes, architecture shifts, or big performance/security work
- Sounds ambiguous and you need the authoritative spec before coding

Use `@/openspec/AGENTS.md` to learn:
- How to create and apply change proposals
- Spec format and conventions
- Project structure and guidelines

Keep this managed block so 'openspec update' can refresh the instructions.

<!-- OPENSPEC:END -->

# ONScripter-jh-Switch Development Guide

Nintendo Switch platform ONScripter/NScripter game emulator, ported from OnscripterYuri.

## Build & Test

```bash
# Clean build
make -f Makefile.switch clean
make -f Makefile.switch -j8

# Single test
cd tests
make run_input_tests          # Switch input handling
make run_path_tests           # Path resolution
make run_game_browser_tests   # Game browser logic
make run_screen_tests         # Screen scaling
make run_utils_tests          # Utility functions
make run_screen_edge_tests    # Edge cases

# All tests
cd tests && make test
```

Output: `onsyuri.nro` (Switch executable)

## Code Style

### Language & Compilation
- **Standard**: C++17 (`-std=gnu++17`)
- **No exceptions**: `-fno-exceptions` (error handling via return codes)
- **No RTTI**: `-fno-rtti`
- **Architecture**: ARMv8-A with NEON SIMD (`-march=armv8-a+crc+crypto`)
- **Platform**: Switch-only (`-D__SWITCH__ -DUSE_SDL2 -DUSE_GLES -DUSE_FILELOG`)

### Naming Conventions
- **Classes**: PascalCase (`GameBrowser`, `ONScripter`, `AnimationInfo`)
- **Functions**: camelCase (`loadFonts()`, `cleanup()`)
- **Member variables**: snake_case with underscore suffix (`window_`, `renderer_`, `selected_index_`)
- **Constants**: UPPER_SNAKE_CASE (`MAX_SPRITE_NUM`, `DEFAULT_VOLUME`)
- **Macros**: UPPER_SNAKE_CASE with underscores (`__ONSCRIPTER_H__`)
- **Namespaces**: lowercase (`utils`)

### File Organization
- Headers: Include guards with `__FILENAME_H__` format
- Include order: Platform headers → Project headers → STL
- SDL includes: Use `SDL2/SDL.h` format (Switch platform)
- No `using namespace std;` in headers

### Error Handling
- Return error codes, never exceptions
- Use `utils::printError()` for logging (writes to `stderr.txt` on Switch)
- Use `utils::printInfo()` for informational logging (writes to `stdout.txt`)
- Check return values from SDL functions (SDL_Init, TTF_Init, etc.)
- Use `false`/nullptr for initialization failure

### Memory Management
- No smart pointers (due to no RTTI/exceptions restriction)
- Manual RAII in destructors
- Resource cleanup in `cleanup()` methods
- Delete/copy constructors where appropriate

### Code Formatting
- 4-space indentation
- Opening brace on same line for functions/methods
- Space after keywords: `if (`, `while (`, `return ` (not single-line)
- Pointer asterisk with variable: `SDL_Window* window`
- Reference symbol with variable: `int& value`

### Comments & Documentation
- File headers: Copyright, license (GPL-2.0), author info
- Function comments: Not strictly required for simple getters/setters
- TODOs: Use `// TODO:` format
- Platform-specific code: Wrap in `#ifdef SWITCH` guards

## Architecture Guidelines

### Key Components
- **`onscripter_main.cpp`**: Entry point, initialization, cleanup
- **`ONScripter*.cpp`**: Core engine (event, command, effect, file, image, sound, text)
- **`GameBrowser.cpp`**: Game selection UI (Switch-specific)
- **`ONScripter_event.cpp`**: Input handling (button mapping here)
- **`gles_renderer.cpp`**: OpenGL ES rendering

### Platform-Specific Code
- Switch features: Wrap in `#ifdef SWITCH` ... `#endif`
- Don't modify `build_switch/` (these are generated artifacts)
- Use Switch SDK headers: `<switch.h>` in `onscripter_main.cpp`

### Resource Paths
- Game directory: `sdmc:/onsemu/`
- Logs: `sdmc:/onsemu/stdout.txt`, `sdmc:/onsemu/stderr.txt`
- Save files: `sdmc:/onsemu/<game>/save/`
- Embedded fonts: Via ROMFS (see `romfs/` directory)

### Input Mapping (Switch)
- A: Confirm/Forward
- B: Cancel/Menu
- X: Skip text
- Y: Auto mode
- L: History view
- R: Fast forward
- L3 (click): Toggle mouse mode
- Touch: UI interaction

### Graphics & Rendering
- OpenGL ES 2.0+ (`-DUSE_GLES`)
- Use `gles_renderer` for rendering operations
- Texture management via `AnimationInfo` class
- Dirty rectangle tracking via `DirtyRect` class

### Audio & Multimedia
- SDL2_mixer for audio playback
- Channels: ONS_MIX_CHANNELS (50) + 4 extra
- Video playback: Not supported on Switch (AVI files excluded)

### Optimization Notes
- ARM NEON SIMD: Use `simd/` utilities for vector operations
- -O2 optimization flag
- Function sections for linker optimization
- Profile before optimizing

## Testing Guidelines
- Test locally first before deploying to Switch
- Use mock SDL headers in tests (`mock_sdl.h`)
- Test framework: Simple assertion-based (`test_framework.h`)
- All tests must pass before commits

## Common Pitfalls
- Don't use exceptions/error handling patterns requiring standard library support
- Don't modify `build_switch/` or generated `.nro` files
- Remember: No regex, no filesystem (use SDL file operations)
- Button mapping logic lives in `ONScripter_event.cpp`
- Game browser logic is Switch-only (`#ifdef SWITCH`)
