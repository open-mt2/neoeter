cmake_minimum_required(VERSION 3.20)

# Standalone configuration when built directly
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(SphereLib LANGUAGES C CXX VERSION 1.0.0)

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

    option(BUILD_TESTING "Build SphereLib unit tests" ON)
    if(BUILD_TESTING)
        enable_testing()
    endif()
endif()

# Source files
set(SPHERELIB_SOURCES
    src/frustum.cpp
    src/sphere.cpp
    src/spherepack.cpp
    src/StdAfx.cpp
)

set(SPHERELIB_HEADERS
    include/SphereLib/frustum.h
    include/SphereLib/pool.h
    include/SphereLib/sphere.h
    include/SphereLib/spherepack.h
    include/SphereLib/StdAfx.h
    include/SphereLib/vector.h
    include/Platform/Platform.h
    include/Platform/DirectXMathCompat.h
)

add_library(SphereLib STATIC
    ${SPHERELIB_SOURCES}
    ${SPHERELIB_HEADERS}
)
add_library(SphereLib::SphereLib ALIAS SphereLib)

target_include_directories(SphereLib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/SphereLib>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# In monorepo, link with EterBase
if(TARGET EterBase)
    target_link_libraries(SphereLib PUBLIC EterBase)
endif()

# Unit Tests
if((CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR AND BUILD_TESTING) OR BUILD_TESTS)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
        add_subdirectory(tests)
    endif()
endif()
