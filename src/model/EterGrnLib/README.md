# EterGrnLib (`pkg_etergrnlib`)

[![EterGrnLib CI](https://github.com/open-mt2/pkg_etergrnlib/actions/workflows/ci.yml/badge.svg)](https://github.com/open-mt2/pkg_etergrnlib/actions)

`EterGrnLib` is the 3D skeletal model, mesh, animation, and level-of-detail (LOD) subsystem of the OpenMT2 client (`CGraphicThing`, `CGraphicThingInstance`, `CGrannyLODController`, `CGrannyModel`, `CGrannyModelInstance`, `CGrannyMesh`, `CGrannyMaterial`, `CGrannyMotion`).

## Architecture & Clean-Room Granny Replacement (`GrannyCompat.h`)

The proprietary 32-bit Windows RAD Game Tools Granny 3D SDK (`granny2.dll` / `granny24.lib` / `granny29.lib`) has been completely replaced with a 64-bit native, open-source compatibility layer (`include/EterGrnLib/GrannyCompat.h`) that provides:
- **64-Bit Scene Graph Structures**: `granny_file_info`, `granny_model`, `granny_skeleton`, `granny_bone`, `granny_mesh`, `granny_material`, `granny_animation`, and `granny_track_group`.
- **World Pose 4×4 Matrix Hierarchy (`GrannyBuildWorldPose`)**: Concatenates local bone translation, quaternion orientation, and scale/shear transforms across bone parent hierarchies and computes composite skinning matrices (`InverseWorld4x4 * World4x4`).
- **Animation Control & Blending**: Stateful `granny_model_instance` and `granny_control` clock management, loop counts, speed ratios, and ease-in/ease-out curves.
- **CPU Linear Blend Skinning (`GrannyDeformVertices`)**: 4-weight bone vertex deformation transforming `PWNT3432` and `PNT332` vertices into hardware/dynamic vertex buffers.

## Directory Layout (Pitchfork Standard)

- `include/EterGrnLib/` — Public headers (`StdAfx.h`, `GrannyCompat.h`, `Thing.h`, `ThingInstance.h`, `LODController.h`, `Model.h`, `ModelInstance.h`, `Mesh.h`, `Material.h`, `Motion.h`, `Util.h`)
- `include/Platform/` — Bundled cross-platform compatibility headers (`Platform.h`, `DirectXMathCompat.h`)
- `src/` — Implementation files
- `tests/` — CTest unit & smoke test suite (`test_etergrnlib.cpp`)

## Building & Testing Standalone

```bash
cmake -B build -G Ninja -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```
