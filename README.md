# Neoeter

3D Game engine based off the eterlib from the old Metin2 Client sources.

## Build

Requires CMake 3.20+ and a C++17 compiler.

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
```

Consumers link the `Neoeter::*` CMake targets and include headers as
`<Neoeter/<Role>/...>`. The shared D3D9 compatibility shim stays
`<Platform/...>`.

## Documentation

https://open-mt2.github.io/docs/neoeter/
