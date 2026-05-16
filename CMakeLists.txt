cmake_minimum_required(VERSION 3.20)
project(unrar-mini)

# ============================================================================
# Build Configuration (Strictly Single-Threaded / C++23)
# ============================================================================
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Disable RTTI and Exceptions to match QuickView's architecture
if(MSVC)
    add_compile_options(/EHs-c- /GR-)
else()
    add_compile_options(-fno-exceptions -fno-rtti)
endif()

# ============================================================================
# Source Files (ONLY Essential Decompression Core)
# ============================================================================
# NOTE: uowners.cpp, volume.cpp, rar.cpp, etc. are EXCLUDED to maintain
# the 'mini' footprint and avoid Windows/POSIX compatibility issues.
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
    largepage.cpp
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

# ============================================================================
# Optimization Matrix (Aligned with QuickView Peak Performance)
# ============================================================================
if(MSVC OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(unrar-mini PRIVATE /O2 /Gw /Zc:inline /fp:fast /GS-)
    
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(unrar-mini PRIVATE 
            /clang:-fno-ident
            /clang:-fno-unwind-tables
            /clang:-fno-asynchronous-unwind-tables
            /clang:-flto=full
        )
    endif()
endif()

target_compile_definitions(unrar-mini PRIVATE
    RAR_NOCRYPT
    _WIN_ALL
    SILENT
    NOVOLUME
    UNRAR_CORE_BUILD
    _CRT_SECURE_NO_WARNINGS
)

target_include_directories(unrar-mini PUBLIC .)

# ============================================================================
# SIMD Support (Google Highway)
# ============================================================================
if(NOT TARGET hwy::hwy)
    find_package(hwy CONFIG QUIET)
endif()

if(TARGET hwy::hwy)
    message(STATUS "unrar-mini: Google Highway SIMD enabled.")
    target_link_libraries(unrar-mini PUBLIC hwy::hwy)
    target_compile_definitions(unrar-mini PUBLIC UNRAR_SIMD_HWY)
endif()

# Suppress 3rd party noise
if(MSVC)
    target_compile_options(unrar-mini PRIVATE /wd4100 /wd4005 /wd4706 /wd4065 /wd4996)
else()
    target_compile_options(unrar-mini PRIVATE -Wno-unused-parameter)
endif()
