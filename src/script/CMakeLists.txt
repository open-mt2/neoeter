cmake_minimum_required(VERSION 3.20)

# Runtime-neutral scripting core: the value model, helper API and native module
# registry shared by every scripting backend (neoeter-python, neoeter-dotnet).
# It deliberately has no dependency on any interpreter or on the rest of the
# engine.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(neoeter_script LANGUAGES CXX VERSION 1.0.0)

    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
    set(NEOETER_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../include")

    option(BUILD_TESTING "Build neoeter_script unit tests" ON)
    if(BUILD_TESTING)
        enable_testing()
    endif()
endif()

add_library(neoeter_script STATIC
    ScriptBuildValue.cpp
    ScriptExceptionSender.cpp
    ScriptHelpers.cpp
    ScriptModule.cpp
    ScriptObject.cpp
    ScriptInternal.h
    ${NEOETER_INCLUDE_DIR}/Neoeter/Script/Script.h
    ${NEOETER_INCLUDE_DIR}/Neoeter/Script/ScriptBackend.h
    ${NEOETER_INCLUDE_DIR}/Neoeter/Script/ScriptExceptionSender.h
)
add_library(Neoeter::Script ALIAS neoeter_script)

set_target_properties(neoeter_script PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(neoeter_script
    PUBLIC
        $<BUILD_INTERFACE:${NEOETER_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

if((CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR AND BUILD_TESTING) OR BUILD_TESTS)
    add_subdirectory(tests)
endif()
