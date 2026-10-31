cmake_minimum_required(VERSION 3.20)

# Standalone configuration when built directly
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(EterImageLib LANGUAGES C CXX VERSION 1.0.0)

    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)

    if(WIN32)
        add_compile_definitions(WIN32 _WINDOWS _CRT_SECURE_NO_WARNINGS)
    else()
        add_compile_definitions(
            OPENMT2_PLATFORM_LINUX=1
            OPENMT2_RENDERER_GL=1
            OPENMT2_SDL_WINDOW=1
            _GNU_SOURCE
        )
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

    option(BUILD_TESTING "Build EterImageLib unit tests" ON)
    if(BUILD_TESTING)
        enable_testing()
    endif()
endif()

# Source files
set(ETERIMAGELIB_SOURCES
    src/DXTCImage.cpp
    src/Image.cpp
    src/StdAfx.cpp
    src/TGAImage.cpp
)

set(ETERIMAGELIB_HEADERS
    include/EterImageLib/DXTCImage.h
    include/EterImageLib/Image.h
    include/EterImageLib/StdAfx.h
    include/EterImageLib/TGAImage.h
    include/Platform/Platform.h
)

add_library(EterImageLib STATIC
    ${ETERIMAGELIB_SOURCES}
    ${ETERIMAGELIB_HEADERS}
)
add_library(EterImageLib::EterImageLib ALIAS EterImageLib)

target_include_directories(EterImageLib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/EterImageLib>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# In monorepo, link with EterBase
if(TARGET EterBase)
    target_link_libraries(EterImageLib PUBLIC EterBase)
endif()

# Unit Tests
if((CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR AND BUILD_TESTING) OR BUILD_TESTS)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
        add_subdirectory(tests)
    endif()
endif()
