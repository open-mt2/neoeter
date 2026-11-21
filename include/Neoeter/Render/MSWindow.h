#pragma once

#include <Neoeter/Platform/Base/Stl.h>
#include <Neoeter/Platform/Platform.h>

#include <SDL2/SDL.h>

class CMSWindow
{
	public:
		CMSWindow();

		virtual ~CMSWindow();

		void Destroy();
		bool Create(const char* c_szName, int brush = BLACK_BRUSH, uint32_t cs = 0, uint32_t ws = WS_OVERLAPPEDWINDOW, HICON hIcon = NULL, int iCursorResource = 32512, int width = 0, int height = 0);

		void Show();
		void Hide();

		bool SetFullscreen(bool enabled, int width, int height);
		void Minimize();

		void SetVisibleMode(bool isVisible);

		void SetPosition(int x, int y);
		void SetCenterPosition();

		void SetText(const char* c_szText);

		void AdjustSize(int width, int height);
		void SetSize(int width, int height);

		bool IsVisible();
		bool IsActive();

		void GetMousePosition(POINT* ppt);
		void GetClientRect(RECT* prc);
		void GetWindowRect(RECT* prc);

		int GetScreenWidth();
		int GetScreenHeight();

		PlatformWindow GetWindowHandle();
		PlatformInstance GetInstance();

		static SDL_Window* ms_pSDLWindow;
		static SDL_Renderer* ms_pSDLRenderer;
		static SDL_GLContext ms_pGLContext;
		static SDL_Window* GetSDLWindow() { return ms_pSDLWindow; }
		static SDL_Renderer* GetSDLRenderer() { return ms_pSDLRenderer; }
		static SDL_GLContext GetGLContext() { return ms_pGLContext; }

		// Shows the frame the renderer just drew: swaps the GL context, presents
		// the software renderer, or updates the window surface, in that order.
		static void Present();

		virtual intptr_t WindowProcedure(PlatformWindow hWnd, uint32_t uiMsg, uintptr_t wParam, intptr_t lParam);
		virtual void OnSize(uintptr_t wParam, intptr_t lParam);

	protected:
		PlatformWindow m_hWnd;
		RECT m_rect;
		bool m_isActive;
		bool m_isVisible;

	protected:
		static PlatformInstance ms_hInstance;
};
