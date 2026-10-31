# SpeedTreeLib (`pkg_speedtreelib`)

[![SpeedTreeLib CI](https://github.com/open-mt2/pkg_speedtreelib/actions/workflows/ci.yml/badge.svg)](https://github.com/open-mt2/pkg_speedtreelib/actions)

`SpeedTreeLib` is the outdoor vegetation, tree instancing, boundary management, and foliage subsystem of the OpenMT2 client (`CSpeedTreeForest`, `CSpeedTreeDirectX`, `CSpeedTreeWrapper`, `CSpeedTreeMaterial`, `CBoundaryShapeManager`).

## Architecture & Clean-Room SpeedTree Replacement (`SpeedTreeRTCompat.h`)

The proprietary 32-bit Windows SpeedTreeRT SDK (`SpeedTreeRT.dll` / `SpeedTreeRT.lib`) has been completely replaced with a 64-bit native compatibility layer (`include/SpeedTreeLib/SpeedTreeRTCompat.h`) that provides:
- **64-Bit Clean-Room Engine Structures**: `CSpeedTreeRT`, `SGeometry` (`SIndexed`, `SLeaf`, `SBillboard`), and `STextures`.
- **SPT Asset Parsing**: Extracts branch/bark textures, composite foliage maps, self-shadow maps, leaves, and fronds from Metin2 SpeedTree CAD (`.spt`) assets.
- **Tree Instancing & Bounding Management**: Efficient tree instancing (`CSpeedTreeForest::CreateInstance`, `CSpeedTreeWrapper::MakeInstance`), bounding box calculation, spherical culling bounding spheres, and trunk collision cylinder definitions.
- **Foliage Lighting & Wind Control**: Dynamic/static lighting parameter dispatch and compound rotation wind matrix computation.

## Baked Geometry Cache (`TreeCacheFormat.h`)

`include/SpeedTreeLib/TreeCacheFormat.h` defines the on-disk `.tree` format shared with the
offline baker (`tools/speedtree_baker`). The client hashes each `.spt` with FNV-1a and loads
`treecache/<hash>.tree` through `CSpeedTreeRT::LoadBakedGeometry`; missing entries fall back to
the procedural generator. Regenerate the cache with `python3 tools/speedtree_baker/bake.py`.

## Directory Layout (Pitchfork Standard)

- `include/SpeedTreeLib/` — Public headers (`StdAfx.h`, `SpeedTreeRTCompat.h`, `TreeCacheFormat.h`, `SpeedTreeForest.h`, `CSpeedTreeDirectX.h`, `SpeedTreeWrapper.h`, `SpeedTreeMaterial.h`, `SpeedTreeConfig.h`, `BoundaryShapeManager.h`, `Constants.h`, `VertexShaders.h`)
- `include/speedtree/` — Forwarding compatibility header (`SpeedTreeRT.h`)
- `include/Platform/` — Bundled cross-platform compatibility headers (`Platform.h`, `DirectXMathCompat.h`)
- `src/` — Implementation files (`SpeedTreeRTCompat.cpp`, `SpeedTreeForest.cpp`, `CSpeedTreeDirectX.cpp`, `SpeedTreeWrapper.cpp`, `BoundaryShapeManager.cpp`)
- `tests/` — CTest unit & smoke test suite (`test_speedtreelib.cpp`)

## Building & Testing Standalone

```bash
cmake -B build -G Ninja -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```
