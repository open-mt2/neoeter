# EffectLib (`pkg_effectlib`)

[![EffectLib CI](https://github.com/open-mt2/pkg_effectlib/actions/workflows/ci.yml/badge.svg)](https://github.com/open-mt2/pkg_effectlib/actions)

`EffectLib` is the particle system, visual effects, mesh effect animations, and dynamic lighting subsystem of the OpenMT2 client (`CEffectManager`, `CEffectInstance`, `CEffectData`, `CParticleSystemData`, `CParticleSystemInstance`, `CEffectMesh`, `CLightData`).

## Architecture & Linux Porting Notes

- **64-Bit & Cross-Platform Math**:
  - Fixed pointer-to-integer offset math in `NEffectUpdateDecorator::CTimeEventDecorator` for 64-bit address spaces (`uintptr_t` arithmetic replacing legacy 32-bit `DWORD` casts).
  - Added portable 3D math implementations including `D3DXQuaternionRotationAxis` and `D3DXQuaternionIdentity` to `DirectXMathCompat.h`.
  - Fixed C++17 conformance issue in `Type.h` with dependent type iterators (`typename`).
  - Fixed multi-line comment trailing backslash issue in `Type.h` enum definitions.
  - Corrected functor call forwarding in `CParticleSystemInstance::ForEachParticleRendering`.
- **Particle System & Update Decorators**:
  - Implements modular particle update behaviors through decorator chains: air resistance (`CAirResistanceDecorator`), gravity (`CGravityDecorator`), rotation (`CRotationDecorator`), color, scale, and texture animation frame stepping.
  - Supports multiple emitter geometries (`POINT`, `ELLIPSE`, `SQUARE`, `SPHERE`) and emission velocities (`FREE`, `OUTER`, `INNER`).
- **Effect Scripts & Resource Management**:
  - Full support for parsing and loading `.mse` effect scripts and `.mss` sound tracks through the virtual filesystem (`CEterPackManager`).
  - Centralized effect registration and lifecycle management via `CEffectManager` singleton and high-performance object pooling (`CDynamicPool`).

## Directory Layout (Pitchfork Standard)

- `include/EffectLib/` — Public headers (`EffectData.h`, `EffectInstance.h`, `EffectManager.h`, `ParticleInstance.h`, `ParticleSystemData.h`, `EffectMesh.h`, `SimpleLightData.h`, `Type.h`, `StdAfx.h`, etc.)
- `include/Platform/` — Bundled cross-platform compatibility headers (`Platform.h`, `DirectXMathCompat.h`)
- `src/` — Implementation files (`EffectData.cpp`, `EffectInstance.cpp`, `EffectManager.cpp`, `ParticleSystemData.cpp`, `ParticleSystemInstance.cpp`, `EffectMesh.cpp`, `FrameController.cpp`, etc.)
- `tests/` — CTest unit & smoke test suite (`test_effectlib.cpp`)

## Building & Testing Standalone

```bash
cmake -B build -G Ninja -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```
