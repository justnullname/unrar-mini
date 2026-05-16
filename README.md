# unrar-mini

A modernized, sanitized, and extremely minimal RAR extraction engine optimized for QuickView.

## Goals
- **Minimal Footprint**: Stripped of legacy OS abstractions, multi-threading overhead, and non-essential features.
- **Modern Standards**: Compiled using C++20 with strict warning levels (`/W4 /WX`).
- **High Performance**: Optimized for single-file extraction speed with SIMD support (SSE/SSE2/SSSE3/SSE4.1).
- **License Compliant**: Fully compliant with the UnRAR license.

## Build Requirements
- **Compiler**: MSVC (Visual Studio 2022+), GCC 11+, or Clang 13+.
- **Standard**: C++20.
- **Build System**: CMake 3.20+.

## Features
- Full support for RAR 5.0 and RAR 3.0 archive formats.
- Simplified system abstractions (no Mark-of-the-Web, no legacy NT checks).
- Static library build for easy integration.

## Usage
Add this repository as a submodule or include it directly in your CMake project:
```cmake
add_subdirectory(unrar-mini)
target_link_libraries(your_app PRIVATE unrar-mini)
```

## License
Based on the UnRAR source by Alexander Roshal. Use of this source is governed by the UnRAR License (see `LICENSE` for details). This code may be used for **extraction only**.
