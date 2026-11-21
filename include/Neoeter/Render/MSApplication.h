#pragma once

#include "MSWindow.h"

class CMSApplication : public CMSWindow
{
	public:
		CMSApplication();
		virtual ~CMSApplication();

		void Initialize(PlatformInstance hInstance);

		void MessageLoop();

		bool IsMessage();
		bool MessageProcess();

	protected:
		void ClearWindowClass();

		intptr_t WindowProcedure(PlatformWindow hWnd, uint32_t uiMsg, uintptr_t wParam, intptr_t lParam);
};
