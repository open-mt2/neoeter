# Architecture — Neoeter (engine)

## Layering

```
effect ── terrain ── model ── audio ── render ── image ── io ── platform
                                                                ▲
                                                    nothing below this line
```

The arrows are dependencies: a package may use anything to its right, never to
its left. The split is the one already encoded in the `pkg_*`
`target_link_libraries`; `D14` only changes where the code lives.

| Role | Packages | Role target | Why grouped |
|---|---|---|---|
| platform | `EterBase`, `EterLocale`, `SphereLib` | `Neoeter::Platform` | OS/alloc/compression, locale tables, math/geometry |
| io | `EterPack` | `Neoeter::IO` | `EIX`/`EPK`/zip archive and file provider layer |
| image | `EterImageLib` | `Neoeter::Image` | DXT codecs, texture/image conversion (no device) |
| render | `EterLib` | `Neoeter::Render` | device, fixed-function state, scene graph, `IRenderBackend` |
| audio | `MilesLib` | `Neoeter::Audio` | Miles façade over miniaudio |
| model | `EterGrnLib`, `SpeedTreeLib` | `Neoeter::Model` | `.gr2`, SpeedTree `.spt`/treecache |
| terrain | `PRTerrainLib` | `Neoeter::Terrain` | terrain heightfield/quadtree |
| effect | `EffectLib` | `Neoeter::Effect` | particle/effect runtime |

## Two invariants the move must not break

1. **Include roots.** Every package keeps its `include/<Package>/…` directory,
   so `#include <EterBase/Singleton.h>` still resolves. Only the CMake include
   directories move one level down; no include line changes.
2. **Build paths.** The root `CMakeLists.txt` passes an explicit binary
   directory: `add_subdirectory(src/render/EterLib build/src/EterLib)`. The
   binaries therefore stay at `build/src/EterLib/tests/test_render_golden`, etc.
   `build.sh`, `golden-check.sh`, `dist` assembly and CI keep working with a
   path prefix change only in the *source* paths.

Target names are frozen: `EterBase`, `EterLib`, …, plus their existing
`Foo::Foo` aliases. `Neoeter::*` role targets are additive (`cmake/NeoeterAliases.cmake`);
they exist so new code can depend on a role instead of a package.

## What is deliberately *not* in this repository

- **Python.** `ScriptLib` (CPython 2.7 host) and `EterPythonLib` (window/graphic
  bindings) live in `neoeter-python`. The engine must not include `Python.h`.
- **The game.** `GameLib`, `UserInterface`, `CWebBrowser` and the `metin2client`
  executable live in `client`.
- **Assets.** `bin/` (packs, root Python scripts) stays a devkit-owned asset
  tree; it is not source.

## Script hosts and the ABI boundary

Today the engine exposes C++ only; the Python bridge links against the same
static libraries and calls C++ directly. That is fine for the one bridge that
exists, but it is not a boundary another language can target.

`include/neoeter/` is reserved for a **stable C ABI** (`docs/abi.md`): opaque
handles, POD structs, UTF-8 strings, no C++ types or exceptions across the
line. It is the prerequisite for `neoeter-csharp` (and for any future host:
Lua, Rust, mods). The ABI is additive — the C++ targets keep their current
interfaces.

`docs/scripting-hosts.md` describes the host contract, the Python bridge as it
exists today, and how a C# bridge plugs in without touching the engine.
