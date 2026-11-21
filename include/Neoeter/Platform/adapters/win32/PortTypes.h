#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <imm.h>

#include <cstdint>
#include <process.h>

// The port names its directory and semaphore operations after the Win32
// functions; the macros of the same name would rename the interface here.
#undef CreateDirectory
#undef CreateSemaphore

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
using PlatformSocketArg = unsigned long;
using PlatformDword = unsigned long;
using PlatformThreadHandle = HANDLE;
using PlatformSemaphoreHandle = HANDLE;
