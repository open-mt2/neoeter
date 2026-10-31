# EterPack (`pkg_eterpack`)

Virtual File System (VFS) and archive provider library for the Metin2 client engine.

## Overview

`EterPack` provides a unified, case-insensitive virtual file system (`CEterPackManager`) supporting:
- **`FOLDER` Provider**: Recursive directory scanning (`std::filesystem`) with automatic case-insensitive path mapping on Linux filesystems and `ymir work/` $\rightarrow$ `d:/ymir work/` virtual path resolution.
- **`ZIP` Provider**: Standard `.zip` archive reading backed by `libzip`.
- **Search Modes**: Configurable `SEARCH_FILE_FIRST` and `SEARCH_PACK_FIRST` lookup priority.

## Directory Layout (Pitchfork Standard)

```text
EterPack/
├── CMakeLists.txt
├── include/
│   ├── EterPack/
│   │   ├── EterPackManager.h
│   │   ├── FileProvider.h
│   │   ├── Folder.h
│   │   ├── Inline.h
│   │   ├── StdAfx.h
│   │   └── ZIP.h
│   └── Platform/
│       └── Platform.h
├── src/
│   ├── EterPackManager.cpp
│   ├── Folder.cpp
│   └── ZIP.cpp
└── tests/
    ├── CMakeLists.txt
    └── test_eterpack.cpp
```

## Building & Testing Standalone

```bash
cmake -B build -G Ninja -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```
