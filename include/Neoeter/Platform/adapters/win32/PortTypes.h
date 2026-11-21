#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <imm.h>

// The port names its directory operation CreateDirectory; the Win32 macro of
// the same name would rename the interface on this target.
#undef CreateDirectory

// Opaque handles the engine and the client exchange. They are the real Win32
// handles on this target and void pointers in the POSIX adapter, so no caller
// outside the platform adapters names a Win32 type.
using PlatformWindow = HWND;
using PlatformDeviceContext = HDC;
using PlatformInstance = HINSTANCE;
using PlatformModule = HMODULE;
using PlatformHandle = HANDLE;
using PlatformCursor = HCURSOR;
using PlatformImeContext = HIMC;
