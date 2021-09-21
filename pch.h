#pragma once

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wrl/client.h>

#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <dxgidebug.h>

#include <string>
#include <memory>
#include <stdint.h>
#include <crtdbg.h>
#include <assert.h>
#include <iostream>
#include <comdef.h>
#include <vector>
#include <chrono>

#if defined(DEBUG) | defined (_DEBUG)
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#else 
#define DBG_NEW new 
#endif