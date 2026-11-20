#pragma once

#include "MSWindow.h"

class CMSApplication : public CMSWindow
{
	public:
		CMSApplication();
		virtual ~CMSApplication();

		void Initialize(HINSTANCE hInstance);

		void MessageLoop();

		bool IsMessage();
		bool MessageProcess();

	protected:
		void ClearWindowClass();

		intptr_t WindowProcedure(HWND hWnd, uint32_t uiMsg, uintptr_t wParam, intptr_t lParam);
};
