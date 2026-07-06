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

To run the engine, you must supply:

- `disk1.dsk`
- `disk2.dsk`

Expected location (planned convention):

```text
bin/disk1.dsk
bin/disk2.dsk
```

If files are missing, the runtime will fail with a clear error message.

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


## Current Implementation (v0.5.0)

**The game is now fully beatable end-to-end**, from character creation through the final confrontation with Mondain.

The engine now includes:

- **Mondain Final Battle:** The final boss encounter is complete. Players can travel through the Time Machine, confront Mondain, destroy the Gem of Immortality, and defeat him to win the game.
- **Full Towns and Castles:** All towns and castles are fully implemented with feature parity to the original Apple II release.
- **Overworld (Complete):** Overworld map, player movement, collision, and the Time Machine are all implemented.
- **Dungeons (Feature Parity Achieved):** Full dungeon support is now complete. Players can enter dungeons, delve up to 10 levels deep, fight enemies, and complete quests by killing the required monster.
- **Space Section:** Players can now launch into space, board and switch vessels at the space station, engage enemy crafts in first-person combat, and progress toward becoming a space ace.
- **Disk Verification:** Validates that `program.dsk` and `player.dsk` exist and contain the required Apple II Ultima files with correct sizes.
- **Disk Loader Scene:** Core disk loading flow is working and displays status messages during boot (file verification and error messages).
- **Asset Extraction:** Reads and parses DOS 3.3 disk catalogs and file structures from the Apple II disks.
- **Title Splash Screen:** Extracts and renders the title screen picture (`PIC.ULTIMATUM`) from the disk images.
- **Character Generator:** Character creation and saving is working.
- **Rendering Pipeline:** OpenGL-based 2D graphics rendering via GLFW.

## Next Steps

- Continue refining game logic, sprite rendering, and Apple II game mechanics.
- Polish and bugfixing pass now that the full game loop is playable.

## Research Notes

Engine behavior findings and reinterpretation decisions are documented in [docs/engine-reinterpretation-notes.md](docs/engine-reinterpretation-notes.md).