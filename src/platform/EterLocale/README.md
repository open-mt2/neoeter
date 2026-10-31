# EterLocale (`pkg_eterlocale`)

Localization and character encoding library for the Metin2 Linux port. Provides multi-codepage string translation, Shift-JIS lead/trail byte verification, Vietnamese custom encoding, and Arabic presentation form shaping.

[![EterLocale CI](https://github.com/open-mt2/pkg_eterlocale/actions/workflows/ci.yml/badge.svg)](https://github.com/open-mt2/pkg_eterlocale/actions)

---

## Features

- **POSIX Standard String Transcoding**: Implements `MultiByteToWideChar` and `WideCharToMultiByte` via POSIX `iconv`, supporting all legacy Windows codepages (`CP949` Korean, `CP1252` Latin, `CP1250` Central Europe, `CP932` Shift-JIS, `CP1251` Cyrillic, `CP1254` Turkish, and `UTF-8`).
- **Vietnamese Support**: High-performance custom CP1258 translation tables via `EL_String_Encode_Vietnamese` and `EL_String_Decode_Vietnamese`.
- **Japanese Support**: Shift-JIS lead/trail byte identification and case-insensitive multibyte string comparison (`ShiftJIS_StringCompareCI`).
- **Arabic Text Shaping**: Bidirectional contextual presentation forms and glyph joining (`Arabic_MakeShape`).

---

## Directory Structure

```text
pkg_eterlocale/
├── CMakeLists.txt              # Dual-mode CMake configuration (standalone / submodule)
├── README.md                   # Documentation & build instructions
├── .github/workflows/ci.yml    # GitHub Actions matrix CI (GCC & Clang, Release & Debug)
├── include/                    # Public API headers
│   ├── EterLocale/             # Library public headers
│   │   ├── Arabic.h
│   │   ├── CodePageId.h
│   │   ├── Japanese.h
│   │   ├── StdAfx.h
│   │   ├── StringCodec.h
│   │   └── StringCodec_Vietnamese.h
│   └── Platform/               # Platform compatibility definitions
│       └── Platform.h
├── src/                        # Implementation source files
│   ├── Arabic.cpp
│   ├── Japanese.cpp
│   ├── StdAfx.cpp
│   ├── StringCodec.cpp
│   └── StringCodec_Vietnamese.cpp
└── tests/                      # Automated CTest test suite
    ├── CMakeLists.txt
    └── test_locale.cpp
```

---

## Building

### Requirements
- CMake 3.20+
- GCC 11+ or Clang 14+ (C++17 support)
- Ninja or Make

### Commands
```bash
# Configure
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON

# Compile
cmake --build build

# Run unit tests
ctest --test-dir build --output-on-failure
```

---

## Integration

### As a CMake Subdirectory
```cmake
add_subdirectory(extern/pkg_eterlocale)
target_link_libraries(YourTarget PRIVATE EterLocale)
```
Include paths (`EterLocale` and `Platform`) are automatically exported via `EterLocale`'s public interface.
