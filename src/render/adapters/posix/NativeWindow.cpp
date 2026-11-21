#include "NativeWindow.h"

#include <SDL2/SDL.h>

void Platform_WarpMouseInWindow(SDL_Window* window, int x, int y)
{
	if (window != nullptr)
		SDL_WarpMouseInWindow(window, x, y);
}
