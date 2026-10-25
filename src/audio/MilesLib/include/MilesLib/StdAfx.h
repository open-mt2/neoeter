#ifndef __INC_MILESLIB_STDAFX_H__
#define __INC_MILESLIB_STDAFX_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef _WIN32
#include <windows.h>
#else
#include "Platform/Platform.h"
#endif

//#include <crtdbg.h>

#include <EterBase/CRC32.h>
#include <EterBase/Stl.h>
#include <EterBase/Utils.h>
#include <EterBase/Debug.h>

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif