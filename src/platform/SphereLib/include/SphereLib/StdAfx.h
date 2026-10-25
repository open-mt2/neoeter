#pragma once

#if (defined(_WIN32) || defined(_WIN64)) && !defined(USE_PLATFORM_COMPAT)
#define WIN32_LEAN_AND_MEAN
#include <d3d9.h>
#include <directxsdk/d3dx9.h>
#include "../eterBase/StdAfx.h"
#else
#include <Platform/Platform.h>
#include <Platform/DirectXMathCompat.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
