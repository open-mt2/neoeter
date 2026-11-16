cmake_minimum_required(VERSION 3.20)
set(NEOETER_VERSION "0.1.7" CACHE STRING "Neoeter release version")
project(Neoeter VERSION ${NEOETER_VERSION} LANGUAGES C CXX)

option(NEOETER_DEBUG_UI "Build the in-game ImGui debug panel" ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(NeoeterOptions)

set(NEOETER_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
include_directories("${NEOETER_INCLUDE_DIR}")

enable_testing()

add_subdirectory("external/miniaudio" "${CMAKE_BINARY_DIR}/external/miniaudio")

set(NEOETER_PACKAGES
    platform/base
    platform/locale
    platform/math
    io
    image
    render
    audio
    model/grn
    model/speedtree
    terrain
    effect
)

foreach(_package IN LISTS NEOETER_PACKAGES)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/${_package}/CMakeLists.txt")
        # One binary directory per role/subpackage; keeps the build tree free of
        # the historic package names.
        add_subdirectory("src/${_package}" "${CMAKE_BINARY_DIR}/src/${_package}")
    else()
        message(STATUS "Neoeter: src/${_package} not imported yet")
    endif()
endforeach()

include(NeoeterAliases)
include(NeoeterInstall)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
    add_subdirectory(tests)
endif()

message(STATUS "Neoeter configured (${NEOETER_PACKAGES})")
