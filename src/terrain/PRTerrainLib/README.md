# PRTerrainLib (`pkg_prterrainlib`)

[![PRTerrainLib CI](https://github.com/open-mt2/pkg_prterrainlib/actions/workflows/ci.yml/badge.svg)](https://github.com/open-mt2/pkg_prterrainlib/actions)

`PRTerrainLib` is the outdoor terrain, heightfield, texture splatting, and terrain attribute subsystem of the OpenMT2 client (`CTerrainImpl`, `CTextureSet`, `TTerrainSplatPatch`).

## Architecture & Linux Porting Notes

- **64-Bit & Cross-Platform Math**: Replaced 32-bit x86 inline assembly macros (`__asm fld / fistp`) in `TerrainType.h` with portable, IEEE-754 compliant fixed-point and floating-point conversions (`PR_FLOAT_TO_INT`, `PR_FLOAT_TO_FIXED`, `PR_FLOAT_ADD_TO_INT`).
- **Terrain Asset Loaders**: Full support for loading 16-bit raw heightmaps (`LoadHeightMap`), packed attribute maps (`LoadAttrMap`), splat tilemaps (`RAW_LoadTileMap`), and multi-layer water maps (`LoadWaterMap`) through the VFS (`CEterPackManager`).
- **Texture Splatting & Sets**: `CTextureSet` manages terrain texture coordinate transforms (`m_matTransform`), scale/offsets, heightfield blending boundaries (`Begin`/`End`), and serializing/deserializing texture configuration scripts (`Load`/`Save`).
- **Shadow Map Handling**: 16-bit 5-6-5 RGB shadow color sampling (`GetShadowMapColor`) with boundary validation across world coordinates.

## Directory Layout (Pitchfork Standard)

- `include/PRTerrainLib/` — Public headers (`StdAfx.h`, `Terrain.h`, `TerrainType.h`, `TextureSet.h`)
- `include/Platform/` — Bundled cross-platform compatibility headers (`Platform.h`, `DirectXMathCompat.h`)
- `src/` — Implementation files (`StdAfx.cpp`, `Terrain.cpp`, `TextureSet.cpp`)
- `tests/` — CTest unit & smoke test suite (`test_prterrainlib.cpp`)

## Building & Testing Standalone

```bash
cmake -B build -G Ninja -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```
