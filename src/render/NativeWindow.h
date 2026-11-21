#pragma once

struct SDL_Window;

// Moves the cursor to a window-relative position. The Windows adapter keeps
// the Win32 screen conversion the SDL global warp does not provide inside a
// fullscreen window.
void Platform_WarpMouseInWindow(SDL_Window* window, int x, int y);
