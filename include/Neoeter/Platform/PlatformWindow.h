#pragma once

#include <atomic>

#include <Neoeter/Platform/PortTypes.h>

// Window state shared by the SDL event loop and the code that queries the
// window without reaching for the OS. It is defined once for every target.
struct PlatformWindowState
{
		int windowWidth;
		int windowHeight;
		int mouseX;
		int mouseY;
		PlatformWindow capturedWindow;
};

inline PlatformWindowState& GetPlatformWindowState()
{
	static PlatformWindowState s_state = {1024, 768, 0, 0, (PlatformWindow)0};
	return s_state;
}

inline void Platform_SetCursorPos(int x, int y)
{
	GetPlatformWindowState().mouseX = x;
	GetPlatformWindowState().mouseY = y;
}

typedef void (*Platform_WarpMouseCallback)(int x, int y);

inline Platform_WarpMouseCallback& Platform_GetWarpMouseCallback()
{
	static Platform_WarpMouseCallback s_callback = nullptr;
	return s_callback;
}

inline void Platform_SetWarpMouseCallback(Platform_WarpMouseCallback callback)
{
	Platform_GetWarpMouseCallback() = callback;
}

inline void Platform_WarpMouse(int x, int y)
{
	Platform_WarpMouseCallback callback = Platform_GetWarpMouseCallback();
	if (callback)
		callback(x, y);
}

inline void Platform_SetWindowResolution(int width, int height)
{
	GetPlatformWindowState().windowWidth = width;
	GetPlatformWindowState().windowHeight = height;
}

inline void Platform_GetWindowResolution(int* width, int* height)
{
	if (width)
		*width = GetPlatformWindowState().windowWidth;
	if (height)
		*height = GetPlatformWindowState().windowHeight;
}

typedef void (*Platform_QuitCallback)(int);

inline Platform_QuitCallback& Platform_GetQuitCallback()
{
	static Platform_QuitCallback s_callback = nullptr;
	return s_callback;
}

inline void Platform_SetQuitCallback(Platform_QuitCallback callback)
{
	Platform_GetQuitCallback() = callback;
}

inline std::atomic<bool>& Platform_GetQuitRequested()
{
	static std::atomic<bool> s_quitRequested(false);
	return s_quitRequested;
}

inline bool Platform_IsQuitRequested()
{
	return Platform_GetQuitRequested().load();
}
