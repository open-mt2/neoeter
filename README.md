# Neoeter

Engine (L0–L3) of the OpenMT2 client: the historic Metin2 client libraries,
consolidated by dependency layer (`D14`).

**Status:** scaffold. The `pkg_*` sources are not imported yet (`TB.1`).

| Path | Packages | Role target |
|---|---|---|
| `src/platform/` | EterBase, EterLocale, SphereLib | `Neoeter::Platform` |
| `src/io/` | EterPack | `Neoeter::IO` |
| `src/image/` | EterImageLib | `Neoeter::Image` |
| `src/render/` | EterLib | `Neoeter::Render` |
| `src/audio/` | MilesLib | `Neoeter::Audio` |
| `src/model/` | EterGrnLib, SpeedTreeLib | `Neoeter::Model` |
| `src/terrain/` | PRTerrainLib | `Neoeter::Terrain` |
| `src/effect/` | EffectLib | `Neoeter::Effect` |

Packages keep their historic directory names, include roots (`<EterBase/...>`),
target names and test names. `layout.map` records the old repository → path
mapping used by `devkit/scripts/import-repo.sh`.

Docs: `docs/architecture.md`, `docs/abi.md`, `docs/scripting-hosts.md`.
