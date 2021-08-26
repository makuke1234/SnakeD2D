#pragma once


#include "common.h"

#include <d2d1.h>
#include <dwrite.h>
#include <wincodec.h>

#include <string_view>
#include <vector>

#ifdef _DEBUG
	#include <cstdio>
	#define dp(...) std::printf(__VA_ARGS__)
#else
	#define dp(...)
#endif
