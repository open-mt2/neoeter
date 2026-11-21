#if !defined(AFX_STDAFX_H__BCF68E23_E7D8_4BF3_A905_AFDBEF92B0F6__INCLUDED_)
#define AFX_STDAFX_H__BCF68E23_E7D8_4BF3_A905_AFDBEF92B0F6__INCLUDED_

#pragma once

#include <Neoeter/Platform/Platform.h>

#include <cassert>
#include <cstdio>
#include <cstdarg>
#include <cstdint>
#include <cstddef>

#include <string>
#include <vector>

inline void _TraceForImage(const char* c_szFormat, ...)
{
	va_list args;
	va_start(args, c_szFormat);

	static char szBuf[1024];
	_vsnprintf(szBuf, sizeof(szBuf), c_szFormat, args);
#ifdef _DEBUG
	OutputDebugString(szBuf);
#endif
	va_end(args);
	printf("%s", szBuf);
}

#endif // !defined(AFX_STDAFX_H__BCF68E23_E7D8_4BF3_A905_AFDBEF92B0F6__INCLUDED_)
