cmake_minimum_required(VERSION 3.16)
project(PRTerrainLib LANGUAGES C CXX)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(PRTERRAINLIB_STANDALONE ON)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
    option(BUILD_TESTING "Build PRTerrainLib tests" ON)
else()
    set(PRTERRAINLIB_STANDALONE OFF)
endif()

if(PRTERRAINLIB_STANDALONE)
    include(FetchContent)

    foreach(dep EterBase:pkg_eterbase EterLocale:pkg_eterlocale EterPack:pkg_eterpack EterImageLib:pkg_eterimagelib SphereLib:pkg_spherelib EterLib:pkg_eterlib)
        string(REPLACE ":" ";" dep_list ${dep})
        list(GET dep_list 0 dep_name)
        list(GET dep_list 1 dep_repo)
        if(NOT TARGET ${dep_name})
            if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../${dep_name}/CMakeLists.txt")
                set(BUILD_TESTING_SAVED ${BUILD_TESTING})
                set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
                add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../${dep_name}" "${CMAKE_BINARY_DIR}/deps/${dep_name}")
                set(BUILD_TESTING ${BUILD_TESTING_SAVED} CACHE BOOL "" FORCE)
            else()
                FetchContent_Declare(
                    ${dep_name}
                    GIT_REPOSITORY https://github.com/open-mt2/${dep_repo}.git
                    GIT_TAG main
                )
                set(BUILD_TESTING_SAVED ${BUILD_TESTING})
                set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
                FetchContent_MakeAvailable(${dep_name})
                set(BUILD_TESTING ${BUILD_TESTING_SAVED} CACHE BOOL "" FORCE)
            endif()
        endif()
    endforeach()
endif()

file(GLOB_RECURSE PRTERRAINLIB_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(PRTerrainLib STATIC ${PRTERRAINLIB_SOURCES})
add_library(OpenMT2::PRTerrainLib ALIAS PRTerrainLib)

target_include_directories(PRTerrainLib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/PRTerrainLib>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(PRTerrainLib
    PUBLIC
        EterBase
        EterPack
        EterImageLib
        EterLib
)

if(UNIX)
    target_compile_definitions(PRTerrainLib PUBLIC
        OPENMT2_PLATFORM_LINUX=1
        OPENMT2_RENDERER_GL=1
        OPENMT2_SDL_WINDOW=1
        _GNU_SOURCE
    )
endif()

if((PRTERRAINLIB_STANDALONE AND BUILD_TESTING) OR (NOT PRTERRAINLIB_STANDALONE AND BUILD_TESTS))
    enable_testing()
    add_subdirectory(tests)
endif()
