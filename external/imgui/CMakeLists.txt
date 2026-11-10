cmake_minimum_required(VERSION 3.20)

add_library(imgui STATIC
    imgui.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
    backends/imgui_impl_sdl2.cpp
    backends/imgui_impl_opengl3.cpp
)
add_library(Neoeter::ImGui ALIAS imgui)

target_include_directories(imgui
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/backends>
        $<INSTALL_INTERFACE:include/imgui>
)

find_package(SDL2 REQUIRED)

if(UNIX)
    find_package(OpenGL REQUIRED)
endif()

target_link_libraries(imgui
    PUBLIC
        SDL2::SDL2
        $<$<PLATFORM_ID:Linux,Darwin>:OpenGL::GL>
)
