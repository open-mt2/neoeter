cmake_minimum_required(VERSION 3.20)

add_library(miniaudio STATIC miniaudio.c)
add_library(Neoeter::Miniaudio ALIAS miniaudio)

target_include_directories(miniaudio
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
        $<INSTALL_INTERFACE:include>
)

find_package(Threads REQUIRED)

if(UNIX)
    target_link_libraries(miniaudio PUBLIC
        Threads::Threads
        $<$<PLATFORM_ID:Linux>:dl>
        $<$<PLATFORM_ID:Linux>:m>
    )
endif()
