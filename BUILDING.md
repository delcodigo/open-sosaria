# Building Open Sosaria

## Requirements

- GCC or Clang with C99 support
- GLFW3 development libraries
- OpenGL libraries
- GNU Make
- `pkg-config` recommended

## Supported Toolchains

The Makefile is written for GNU Make plus a POSIX shell.

- Linux: system GCC or Clang
- macOS: Apple Clang with Homebrew packages
- Windows: MSYS2 MinGW-w64 shell (`UCRT64` or `MINGW64`)

On Windows, run `make` from an MSYS2 shell. The recipes use POSIX commands such as `mkdir -p` and `rm -rf`.

## Build Types

```bash
make              # Release build (stripped, GUI subsystem on Windows)
make debug        # Debug build (symbols, console subsystem on Windows)
make clean        # Remove build artifacts
```

On some MSYS2 setups, `make` may be exposed as `mingw32-make`. If `make` is not found, use `mingw32-make` for the same targets.

## Platform-Specific Setup

### Linux (Ubuntu/Debian)

```bash
sudo apt install build-essential pkg-config libglfw3-dev libgl1-mesa-dev
```

The Makefile prefers `pkg-config` to discover GLFW. If `pkg-config` is unavailable, it falls back to `-lglfw` plus the required OpenGL/X11 libraries.

### macOS

```bash
brew install pkg-config glfw
```

The Makefile prefers `pkg-config`. If it is unavailable, it falls back to `brew --prefix` and links the required macOS frameworks.

### Windows (MSYS2)

Recommended setup:

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-make mingw-w64-ucrt-x86_64-pkgconf mingw-w64-ucrt-x86_64-glfw
```

Launch the matching MSYS2 shell before building. If `pkg-config` cannot locate GLFW, the Makefile falls back to `C:/CLibs/include` and `C:/CLibs/libs` for a manual install.

If `make` is unavailable in the shell, run `mingw32-make` instead.

## Build Details

### Compiler and Linker Flags

**Release build:**
- `-std=c99` - C99 standard
- `-Wall -Wextra -Werror` - Strict warnings
- `-MMD -MP` - Generate header dependency files
- `-s` - Strip symbols from the final binary
- Windows only: `-mwindows` for the GUI subsystem

**Debug build:**
- `-g` - Debug symbols
- `-DDEBUG` - Debug macro
- No `-s` or `-mwindows`

**Standard variable layout:**
- `CPPFLAGS` - Include paths and preprocessor defines
- `CFLAGS` - Warning and compile flags
- `LDFLAGS` - Linker search paths and options
- `LDLIBS` - Linked libraries

### Special Compilation Rules

The third-party `glad.c` source is compiled without warning promotion so external code does not fail the build.

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

## Project Structure

```text
open-sosaria/
├── src/
├── build/                  # Object files and generated .d files
├── bin/
│   └── open-sosaria(.exe)  # Final executable
└── Makefile
```

## Troubleshooting

**Missing GLFW flags or "cannot find -lglfw":**
- Ensure GLFW development libraries are installed
- Verify `pkg-config --cflags --libs glfw3` succeeds
- On Windows, use the MSYS2 shell that matches the installed package set (`UCRT64` vs `MINGW64`)
- If using a manual Windows install, verify `C:/CLibs/include` and `C:/CLibs/libs`

**Compilation warnings in glad.c:**
- These are suppressed by design; `glad.c` uses a dedicated compile rule
