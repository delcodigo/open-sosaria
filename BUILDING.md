# Building Open Sosaria

## Requirements

- GCC compiler with C99 support
- GLFW3 development libraries
- OpenGL libraries
- Make

## Build Types

```bash
make              # Release build (optimized, stripped)
make debug        # Debug build (symbols, no optimization, no stripping)
make clean        # Remove build artifacts
```

## Platform-Specific Setup

### Linux (Ubuntu/Debian)

```bash
sudo apt install libglfw3-dev libgl1-mesa-dev build-essential
```

The Makefile expects GLFW headers in `/usr/local/include` and libraries in `/usr/local/lib`. If using system packages, they're typically in `/usr/include` and `/usr/lib`, which the linker will find automatically.

### macOS

```bash
brew install glfw
```

GLFW will be installed to `/usr/local/` by default.

### Windows (MinGW/MSYS2)

The Makefile expects GLFW in `C:/CLibs/include` and `C:/CLibs/libs`.

1. Download GLFW from [glfw.org](https://www.glfw.org/download.html)
2. Extract to `C:/CLibs/` or adjust the paths in the Makefile's `LDFlags`
3. Or use MSYS2 package manager:
   ```bash
   pacman -S mingw-w64-x86_64-glfw
   ```
   Then update the Makefile paths accordingly.

## Build Details

### Compiler Flags

**Release build:**
- `-std=c99` - C99 standard
- `-s` - Strip symbols (smaller binary)
- `-Wall -Wextra -Werror` - Strict warnings
- Platform-specific defines: `-DCR_UNIX`, `-DCR_WINDOWS`, `-D_POSIX_C_SOURCE=200809L`

**Debug build:**
- `-g` - Debug symbols
- `-DDEBUG` - Debug macro
- No stripping or `-mwindows` flag

### Special Compilation Rules

The `glad.c` dependency file is compiled without `-Werror` and `-pedantic-errors` to avoid warnings in third-party code.

## Verifying the Build

Check dynamic library dependencies:

```bash
# Linux
ldd bin/open-sosaria

# macOS  
otool -L bin/open-sosaria

# Windows (MSYS2)
ldd bin/open-sosaria.exe
```

## Makefile Variables

- `CC` - Compiler (default: `gcc`)
- `CFLAGS` - Base compiler flags (default: `-std=c99`)
- `LDFlags` - Linker flags (platform-specific, auto-detected)
- `OSBuildFlags` - OS-specific build flags (e.g., `-s` for stripping)
- `OSDebugFlags` - Debug-specific flags (e.g., `-DDEBUG`)

## Project Structure

```
open-sosaria/
├── src/
│   ├── main.c              # Main entry point
│   └── dependencies/
│       ├── glad.c          # OpenGL loader
│       └── glad.h          # OpenGL headers
├── build/                  # Object files (*.o)
├── bin/
│   └── open-sosaria        # Final executable
└── Makefile
```

## Troubleshooting

**"cannot find -lglfw" error:**
- Ensure GLFW development libraries are installed
- On Linux, verify `/usr/local/lib` or `/usr/lib` contains `libglfw.so`
- On macOS, check `/usr/local/lib` for `libglfw.dylib`
- On Windows, verify paths in `LDFlags` match your GLFW installation

**Compilation warnings in glad.c:**
- These are suppressed by design; glad.c uses a special compilation rule
