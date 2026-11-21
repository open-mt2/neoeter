#include "NativeWindow.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

void Platform_WarpMouseInWindow(SDL_Window* window, int x, int y)
{
	if (window == nullptr)
		return;

	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);

	if (SDL_GetWindowWMInfo(window, &info) && info.subsystem == SDL_SYSWM_WINDOWS)
	{
		POINT point = {x, y};
		ClientToScreen(info.info.win.window, &point);
		SetCursorPos(point.x, point.y);
		return;
	}

	int windowX = 0;
	int windowY = 0;
	SDL_GetWindowPosition(window, &windowX, &windowY);
	SDL_WarpMouseGlobal(windowX + x, windowY + y);
}
