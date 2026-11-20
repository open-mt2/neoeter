#pragma once

#include "StdAfx.h"

int32_t ShiftJIS_IsLeadByte(const char chByte);
int32_t ShiftJIS_IsTrailByte(const char chByte);
int ShiftJIS_StringCompareCI(LPCSTR szStringLeft, LPCSTR szStringRight, size_t sizeLength);
