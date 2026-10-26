cmake_minimum_required(VERSION 3.20)

# Standalone configuration when built directly
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(MilesLib LANGUAGES C CXX VERSION 1.0.0)

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

    option(BUILD_TESTING "Build MilesLib unit tests" ON)
    if(BUILD_TESTING)
        enable_testing()
    endif()

    # Resolve sibling dependencies when built standalone inside the workspace or with deps/
    set(_MILESLIB_DEPS EterBase EterLocale EterPack EterImageLib SphereLib EterLib)
    foreach(_dep IN LISTS _MILESLIB_DEPS)
        if(NOT TARGET ${_dep})
            if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../${_dep}/CMakeLists.txt")
                add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../${_dep}" "${CMAKE_BINARY_DIR}/deps/${_dep}" EXCLUDE_FROM_ALL)
            elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/deps/${_dep}/CMakeLists.txt")
                add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/deps/${_dep}" "${CMAKE_BINARY_DIR}/deps/${_dep}" EXCLUDE_FROM_ALL)
            endif()
        endif()
    endforeach()
endif()

find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(MINIAUDIO REQUIRED miniaudio)

set(MILESLIB_SOURCES
    src/SoundBase.cpp
    src/SoundData.cpp
    src/SoundInstance2D.cpp
    src/SoundInstance3D.cpp
    src/SoundInstanceStream.cpp
    src/SoundManager.cpp
    src/SoundManager2D.cpp
    src/SoundManager3D.cpp
    src/SoundManagerStream.cpp
    src/StdAfx.cpp
    src/Type.cpp
)

set(MILESLIB_HEADERS
    include/MilesLib/MSSCompat.h
    include/MilesLib/SoundBase.h
    include/MilesLib/SoundData.h
    include/MilesLib/SoundInstance.h
    include/MilesLib/SoundManager.h
    include/MilesLib/SoundManager2D.h
    include/MilesLib/SoundManager3D.h
    include/MilesLib/SoundManagerStream.h
    include/MilesLib/StdAfx.h
    include/MilesLib/Type.h
    include/Platform/DirectXMathCompat.h
    include/Platform/Platform.h
)

add_library(MilesLib STATIC
    ${MILESLIB_SOURCES}
    ${MILESLIB_HEADERS}
)
add_library(MilesLib::MilesLib ALIAS MilesLib)

target_include_directories(MilesLib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/MilesLib>
        $<INSTALL_INTERFACE:include>
        ${MINIAUDIO_INCLUDE_DIRS}
        /usr/include/miniaudio
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(MilesLib
    PUBLIC
        EterBase
        EterPack
        EterLib
        Threads::Threads
        ${MINIAUDIO_LIBRARIES}
        dl
        m
)

# Unit Tests
if((CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR AND BUILD_TESTING) OR BUILD_TESTS)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
        add_subdirectory(tests)
    endif()
endif()
