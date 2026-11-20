#ifndef __INC_ETERLIB_DEBUG_H__
#define __INC_ETERLIB_DEBUG_H__

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <cstdint>
#include <cstddef>
#else
#include <Neoeter/Platform/Platform.h>
#endif

extern void SetLogLevel(uint32_t uLevel);
extern void Log(uint32_t uLevel, const char* c_szMsg);
extern void Logn(uint32_t uLevel, const char* c_szMsg);
extern void Logf(uint32_t uLevel, const char* c_szFormat, ...);
extern void Lognf(uint32_t uLevel, const char* c_szFormat, ...);

extern void Trace(const char* c_szMsg);
extern void Tracen(const char* c_szMsg);
extern void Tracenf(const char* c_szFormat, ...);
extern void Tracef(const char* c_szFormat, ...);
extern void TraceError(const char* c_szFormat, ...);
extern void TraceErrorWithoutEnter(const char* c_szFormat, ...);

extern void LogBox(const char* c_szMsg, const char* c_szCaption = NULL, HWND hWnd = NULL);
extern void LogBoxf(const char* c_szMsg, ...);

extern void LogFile(const char* c_szMsg);
extern void LogFilef(const char* c_szMessage, ...);
extern void OpenConsoleWindow(void);
extern void CloseConsoleWindow();
extern void SetupLog(void);

extern void OpenLogFile(bool bUseLogFile = true);
extern void CloseLogFile();

extern HWND g_PopupHwnd;

#define CHECK_RETURN(flag, string) \
	if (flag)                      \
	{                              \
		LogBox(string);            \
		return;                    \
	}

#endif
