# EterImageLib (`pkg_eterimagelib`)

2D image surface manipulation, TGA encoder/decoder, and DDS (DXT1/DXT3/DXT5) texture decompression library for the Metin2 client engine.

## Overview

`EterImageLib` provides zero-dependency image and compressed texture handling:
- **`CImage`**: 32-bit ARGB image surface management (`Create`, `Clear`, `PutImage`, `FlipTopToBottom`).
- **`CTGAImage`**: Uncompressed (16/24/32-bit) and RLE-compressed (type 10) `.tga` reader and writer.
- **`CDXTCImage`**: 64-bit safe `.dds` header parser (`DDSURFACEDESC2`) and software block decompressor for `DXT1`, `DXT3`, `DXT5`, and `ARGB` mipmap surfaces.

## Directory Layout (Pitchfork Standard)

```text
EterImageLib/
├── CMakeLists.txt
├── include/
│   ├── EterImageLib/
│   │   ├── DXTCImage.h
│   │   ├── Image.h
│   │   ├── StdAfx.h
│   │   └── TGAImage.h
│   └── Platform/
│       └── Platform.h
├── src/
│   ├── DXTCImage.cpp
│   ├── Image.cpp
│   ├── StdAfx.cpp
│   └── TGAImage.cpp
└── tests/
    ├── CMakeLists.txt
    └── test_image.cpp
```

## Building & Testing Standalone

```bash
cmake -B build -G Ninja -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```
