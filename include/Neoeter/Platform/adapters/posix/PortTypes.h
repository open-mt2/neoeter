#pragma once

// Opaque handles the engine and the client exchange. They are the real Win32
// handles on the Windows target and void pointers here, so no caller outside
// the platform adapters names a Win32 type.
using PlatformWindow = void*;
using PlatformDeviceContext = void*;
using PlatformInstance = void*;
using PlatformModule = void*;
using PlatformHandle = void*;
using PlatformCursor = void*;
using PlatformImeContext = void*;
