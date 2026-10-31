# SphereLib (`pkg_spherelib`)

Bounding sphere hierarchy, frustum culling, and ray/range intersection library for the Metin2 client engine.

## Overview

`SphereLib` provides spatial partitioning and culling primitives:
- **`Vector3d`**: 3D vector math extending `D3DXVECTOR3`.
- **`Sphere`**: Bounding sphere computation (Ritter's algorithm) and ray-sphere intersection.
- **`Frustum`**: 6-plane view frustum extraction and sphere-frustum culling (`VS_INSIDE`, `VS_PARTIAL`, `VS_OUTSIDE`).
- **`SpherePack` & `SpherePackFactory`**: Hierarchical bounding sphere tree supporting fast frustum culling, ray tracing, and range queries.

## Directory Layout (Pitchfork Standard)

```text
SphereLib/
├── CMakeLists.txt
├── include/
│   ├── SphereLib/
│   │   ├── frustum.h
│   │   ├── pool.h
│   │   ├── sphere.h
│   │   ├── spherepack.h
│   │   ├── StdAfx.h
│   │   └── vector.h
│   └── Platform/
│       ├── DirectXMathCompat.h
│       └── Platform.h
├── src/
│   ├── frustum.cpp
│   ├── sphere.cpp
│   ├── spherepack.cpp
│   └── StdAfx.cpp
└── tests/
    ├── CMakeLists.txt
    └── test_sphere.cpp
```

## Building & Testing Standalone

```bash
cmake -B build -G Ninja -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```
