cmake_minimum_required(VERSION 3.20)
project(unrar-mini)

# Use C++20 for modern standards and performance
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Minimal Core decoding files for QuickView
# Stripped of legacy OS checks, multi-threading, and non-essential features.
set(UNRAR_SOURCES
    archive.cpp
    arcread.cpp
    blake2s.cpp
    crc.cpp
    encname.cpp
    errhnd.cpp
    extinfo.cpp
    file.cpp
    filefn.cpp
    filestr.cpp
    getbits.cpp
    hash.cpp
    headers.cpp
    match.cpp
    options.cpp
    pathfn.cpp
    rarvm.cpp
    rawread.cpp
    rdwrfn.cpp
    sha1.cpp
    sha256.cpp
    smallfn.cpp
    strfn.cpp
    strlist.cpp
    system.cpp
    timefn.cpp
    unicode.cpp
    unpack.cpp
)

add_library(unrar-mini STATIC ${UNRAR_SOURCES})

# Ensure that users of this library automatically get the correct include paths
target_include_directories(unrar-mini 
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

target_compile_definitions(unrar-mini PRIVATE
    RAR_NOCRYPT             # No encryption needed for QuickView previews
    _WIN_ALL                # Windows platform target
    SILENT                  # No console output
    NOVOLUME                # No multi-volume support needed for basic previews
    UNRAR_CORE_BUILD
    _CRT_SECURE_NO_WARNINGS # Suppress legacy CRT warnings
)

# Strict warnings to maintain high code quality
if(MSVC)
    target_compile_options(unrar-mini PRIVATE /W4 /WX /FS)
else()
    target_compile_options(unrar-mini PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

# Find and link Google Highway for multi-platform SIMD support
# Supports SSE, AVX, NEON, RVV via dynamic dispatch
find_package(hwy CONFIG REQUIRED)
target_link_libraries(unrar-mini PUBLIC hwy::hwy)

# Set common optimization flags for standalone build
if(MSVC)
    target_compile_options(unrar-mini PRIVATE /O2 /Oi /Ot /Oy /GT /GL /Gw)
endif()
