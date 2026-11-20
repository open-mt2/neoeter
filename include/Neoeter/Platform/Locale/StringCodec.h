#pragma once

#include "StdAfx.h"

#if !defined(_WIN32)
int MultiByteToWideChar(
	uint32_t CodePage,
	uint32_t dwFlags,
	LPCSTR lpMultiByteStr,
	int cbMultiByte,
	LPWSTR lpWideCharStr,
	int cchWideChar);

int WideCharToMultiByte(
	uint32_t CodePage,
	uint32_t dwFlags,
	LPCWSTR lpWideCharStr,
	int cchWideChar,
	LPSTR lpMultiByteStr,
	int cbMultiByte,
	LPCSTR lpDefaultChar,
	LPBOOL lpUsedDefaultChar);
#endif

int Ymir_WideCharToMultiByte(
	uint32_t CodePage,
	uint32_t dwFlags,
	LPCWSTR lpWideCharStr,
	int cchWideChar,
	LPSTR lpMultiByteStr,
	int cbMultiByte,
	LPCSTR lpDefaultChar,
	LPBOOL lpUsedDefaultChar);

int Ymir_MultiByteToWideChar(
	uint32_t CodePage,
	uint32_t dwFlags,
	LPCSTR lpMultiByteStr,
	int cbMultiByte,
	LPWSTR lpWideCharStr,
	int cchWideChar);
