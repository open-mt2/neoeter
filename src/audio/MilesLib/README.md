# MilesLib (`pkg_mileslib`)

Audio engine library for the Open-MT2 client (`x86_64` Linux & Windows).

## Overview

`MilesLib` provides the 2D, 3D positional, and streaming audio subsystem for the Metin2 client:

- **`CSoundManager`**: Master singleton managing 2D sound effects, 3D spatial/character/ambience audio, and BGM music streams with fade-in/fade-out transitions and volume controls.
- **`CSoundManager2D` / `CSoundInstance2D`**: 2D UI and sound effect playback pool (`INSTANCE_MAX_COUNT = 4`).
- **`CSoundManager3D` / `CSoundInstance3D`**: 3D positional sound and listener management (`INSTANCE_MAX_COUNT = 32`).
- **`CSoundManagerStream` / `CSoundInstanceStream`**: Background music streaming (`MUSIC_INSTANCE_MAX_NUM = 3`).
- **`CSoundData`**: Reference-counted sound buffer loader supporting `CEterPackManager` virtual file system callbacks.
- **`NSound` (`Type.h`)**: Motion/effect sound script loader, saver, and frame-based instance converter (`LoadSoundInformationPiece`, `SaveSoundInformationPiece`, `DataToInstance`).
- **`MSSCompat.h`**: Self-contained Linux/POSIX compatibility layer for the RAD Miles Sound System (`AIL_*`) API (Stage 1 baseline prior to Stage 2 `miles-rs` migration).

## Directory Structure (Pitchfork Layout)

```text
MilesLib/
├── CMakeLists.txt
├── README.md
├── include/
│   ├── MilesLib/         # Public MilesLib headers & MSSCompat.h
│   └── Platform/         # Cross-platform POSIX/Win32 compatibility headers
├── src/                  # Implementation files (.cpp)
└── tests/                # CTest smoke and unit tests
```

## Building & Testing

### Standalone Build

```bash
cmake -S . -B build
cmake --build build -j
ctest --test-dir build --output-on-failure
```

### Monorepo Build

Included automatically via `add_subdirectory(src/MilesLib)` from the root `CMakeLists.txt`.
