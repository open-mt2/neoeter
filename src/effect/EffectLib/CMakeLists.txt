cmake_minimum_required(VERSION 3.16)
project(EffectLib LANGUAGES C CXX)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(EFFECTLIB_STANDALONE ON)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
    option(BUILD_TESTING "Build EffectLib tests" ON)
else()
    set(EFFECTLIB_STANDALONE OFF)
endif()

if(EFFECTLIB_STANDALONE)
    include(FetchContent)

    foreach(dep EterBase:pkg_eterbase EterLocale:pkg_eterlocale EterPack:pkg_eterpack EterImageLib:pkg_eterimagelib SphereLib:pkg_spherelib CWebBrowser:pkg_cwebbrowser EterLib:pkg_eterlib MilesLib:pkg_mileslib)
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

file(GLOB_RECURSE EFFECTLIB_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(EffectLib STATIC ${EFFECTLIB_SOURCES})
add_library(OpenMT2::EffectLib ALIAS EffectLib)

target_include_directories(EffectLib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/EffectLib>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(EffectLib
    PUBLIC
        EterBase
        EterPack
        EterLib
        MilesLib
)

if(UNIX)
    target_compile_definitions(EffectLib PUBLIC __LINUX__ _GNU_SOURCE)
endif()

if((EFFECTLIB_STANDALONE AND BUILD_TESTING) OR (NOT EFFECTLIB_STANDALONE AND BUILD_TESTS))
    enable_testing()
    add_subdirectory(tests)
endif()
