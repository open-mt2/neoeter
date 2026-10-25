cmake_minimum_required(VERSION 3.20)

# Standalone configuration when built directly
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(EterPack LANGUAGES C CXX VERSION 1.0.0)

    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)

    if(WIN32)
        add_compile_definitions(WIN32 _WINDOWS _CRT_SECURE_NO_WARNINGS)
    else()
        add_compile_definitions(__LINUX__ _GNU_SOURCE)
        add_compile_options(
            -Wall
            -Wextra
            -Wno-unused-parameter
            -Wno-invalid-offsetof
            -Wno-multichar
            -Wno-sign-compare
            -fPIC
        )
    endif()

    option(BUILD_TESTING "Build EterPack unit tests" ON)
    if(BUILD_TESTING)
        enable_testing()
    endif()
endif()

# Find dependencies: Threads and libzip
find_package(Threads REQUIRED)
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(LIBZIP QUIET libzip)
endif()
if(NOT LIBZIP_FOUND)
    find_path(LIBZIP_INCLUDE_DIRS NAMES zip.h)
    find_library(LIBZIP_LIBRARIES NAMES zip)
    if(NOT LIBZIP_INCLUDE_DIRS OR NOT LIBZIP_LIBRARIES)
        message(FATAL_ERROR "libzip is required to build EterPack (install libzip-dev / libzip).")
    endif()
endif()

# Source files
set(ETERPACK_SOURCES
    src/EterPackManager.cpp
    src/Folder.cpp
    src/ZIP.cpp
)

set(ETERPACK_HEADERS
    include/EterPack/EterPackManager.h
    include/EterPack/FileProvider.h
    include/EterPack/Folder.h
    include/EterPack/Inline.h
    include/EterPack/StdAfx.h
    include/EterPack/ZIP.h
    include/Platform/Platform.h
)

add_library(EterPack STATIC
    ${ETERPACK_SOURCES}
    ${ETERPACK_HEADERS}
)
add_library(EterPack::EterPack ALIAS EterPack)

target_include_directories(EterPack
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/EterPack>
        $<INSTALL_INTERFACE:include>
        ${LIBZIP_INCLUDE_DIRS}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(EterPack
    PUBLIC
        ${LIBZIP_LIBRARIES}
        Threads::Threads
)

# In monorepo, link with EterBase
if(TARGET EterBase)
    target_link_libraries(EterPack PUBLIC EterBase)
endif()

# Unit Tests
if((CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR AND BUILD_TESTING) OR BUILD_TESTS)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
        add_subdirectory(tests)
    endif()
endif()
