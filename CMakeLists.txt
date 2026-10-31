cmake_minimum_required(VERSION 3.20)
project(Neoeter VERSION 0.1.0 LANGUAGES C CXX)

# Engine (L0–L3) of the OpenMT2 client. See docs/architecture.md.
#
# The directory list below is the single source of truth for the build order.
# Until TB.1 imports the histories, missing packages are reported and skipped so
# the scaffold configures on its own.

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(NeoeterOptions)

set(NEOETER_PACKAGES
    platform/EterBase
    platform/EterLocale
    platform/SphereLib
    io/EterPack
    image/EterImageLib
    render/EterLib
    audio/MilesLib
    model/EterGrnLib
    model/SpeedTreeLib
    terrain/PRTerrainLib
    effect/EffectLib
)

foreach(_package IN LISTS NEOETER_PACKAGES)
    get_filename_component(_name "${_package}" NAME)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/${_package}/CMakeLists.txt")
        # Pin the binary directory to the historic build/src/<Package> path.
        # Debian packaging, CTest fixtures, golden-check.sh and the devkit's
        # build.sh all resolve binaries through it; the move must not change it.
        add_subdirectory("src/${_package}" "${CMAKE_BINARY_DIR}/src/${_name}")
    else()
        message(STATUS "Neoeter: src/${_package} not imported yet")
    endif()
endforeach()

include(NeoeterAliases)

if(BUILD_TESTS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
    enable_testing()
    add_subdirectory(tests)
endif()

message(STATUS "Neoeter configured (${NEOETER_PACKAGES})")
