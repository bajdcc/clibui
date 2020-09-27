﻿// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // 从 Windows 头中排除极少使用的资料
// Windows 头文件: 
#include <windows.h>

/*
#ifdef _DEBUG
#pragma push_macro("new")
#undef new
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)  
#endif
*/

// C 运行时头文件
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <intrin.h>
#include <utility>
#include <future>

#include <vector>
#include <list>
#include <queue>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <regex>
#include <algorithm>
#include <bitset>
#include <numeric>
#include <utility>

#include <ctime>

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

#include <crtdbg.h>

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // 某些 CString 构造函数将是显式的

#include <atlbase.h>
#include <atlstr.h>

// TODO:  在此处引用程序需要的其他头文件
#ifndef WM_SYSTIMER
#define WM_SYSTIMER 0x0118 //(caret blink)
#endif

#include <Shlwapi.h>
#include <ShlObj.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <winhttp.h>
#include <WinCodec.h>
#include <Commdlg.h>
#include <MMSystem.h>
#include <Vfw.h>
#include <d2d1.h>
#include <d2d1_1.h>
#include <d3d11.h>
#include <dxgi.h>
#include <dwrite.h>
#include <usp10.h>
#include <gdiplus.h>
#include <initguid.h>

#pragma comment(lib, "Gdiplus")
#pragma comment(lib, "WindowsCodecs")
#pragma comment(lib, "Msimg32")
#pragma comment(lib, "WinHttp")
#pragma comment(lib, "d2d1")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "dwrite")
#pragma comment(lib, "Comdlg32")
#pragma comment(lib, "Comctl32")
#pragma comment(lib, "Vfw32")
#pragma comment(lib, "imm32")
#pragma comment(lib, "dxgi")
#pragma comment(lib, "dxguid")
#pragma comment(lib, "winmm")
#pragma comment(lib, "IPHLPAPI")
#pragma comment(lib, "XINPUT9_1_0")

#pragma comment(lib, "lib\\zlib")
#pragma comment(lib, "lib\\libcurl")
#pragma comment(lib, "lib\\libzplay")