# Open Sosaria

`open-sosaria` is a PC engine reimplementation project focused on the original **Ultima (1981)** Apple II release, first published by **California Pacific Computer Company**.

## Project Goal

Recreate the original game behavior on modern PCs while preserving historical mechanics and data compatibility.

## Legal and Content Policy

- This project is released under the **MIT License**.
- This is an independent, non-commercial fan reimplementation.
- The repository does **not** include original game assets, ROMs, manuals, or disk images.
- You must provide your own legally obtained Apple II disk images.

## Required Game Data (Bring Your Own)

To run the future engine implementation, you must supply:

- `disk1.dsk`
- `disk2.dsk`

Expected location (planned convention):

```text
bin/disk1.dsk
bin/disk2.dsk
```

If files are missing, the runtime should fail with a clear error message.

## Historical Context

- **Game:** Ultima (later known as *Ultima I: The First Age of Darkness*)
- **Original release:** 1981
- **Original platform:** Apple II
- **Original U.S. publisher:** California Pacific Computer Company
- **Creator:** Richard Garriott

## Current Tech Baseline

- C99
- GLFW3 + OpenGL bootstrap window
- Cross-platform Makefile

## Build Requirements

**Linux:**
```bash
sudo apt install build-essential pkg-config libglfw3-dev libgl1-mesa-dev
```

**macOS:**
```bash
brew install pkg-config glfw
```

**Windows:**
- Use an MSYS2 MinGW-w64 shell (`UCRT64` recommended)
- Install `gcc`, `make`, `pkgconf`, and `glfw` from MSYS2
- Manual fallback: install GLFW to `C:/CLibs/include` and `C:/CLibs/libs`

See [BUILDING.md](BUILDING.md) for detailed build instructions.

## Build

```bash
make              # Release build
make debug        # Debug build with symbols
make clean        # Clean build artifacts
```

On some MSYS2 setups, use `mingw32-make` if `make` is not available in PATH.

## Run

```bash
./bin/open-sosaria
```

## Current Implementation

The engine currently includes:

- **Disk Verification:** Validates that `disk1.dsk` and `disk2.dsk` exist and contain the required Apple II Ultima files with correct sizes
- **Disk Loader Scene (Partial):** Core disk loading flow is working and displays status messages during boot (file verification and error messages)
- **Asset Extraction:** Reads and parses DOS 3.3 disk catalogs and file structures from the Apple II disks
- **Title Splash Screen:** Extracts and renders the title screen picture (`PIC.ULTIMATUM`) from the disk images
- **Character Generator:** Character creation and saving is working
- **Overworld Rendering:** Overworld map rendering is working with a temporary flying camera
- **Overworld Player Controller:** Player movement and collision in the overworld are working
- **Rendering Pipeline:** OpenGL-based 2D graphics rendering via GLFW

## Next Steps

Game logic, additional sprite rendering work, and Apple II game mechanics are planned for future development.

## Research Notes

Engine behavior findings and reinterpretation decisions are documented in [docs/engine-reinterpretation-notes.md](docs/engine-reinterpretation-notes.md).
