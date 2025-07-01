#pragma once

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <d3d9on12.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <dxgi.h>
#include <stdio.h>
#include <strsafe.h>
#include <combaseapi.h>

typedef HRESULT(WINAPI* CreateDXGIFactory2_t)(UINT flags, REFIID iid, void** factory);
typedef HRESULT(WINAPI* D3D12CreateDevice_t)(IUnknown* adapter, D3D_FEATURE_LEVEL feature_level, REFIID iid, void** device);
typedef HRESULT(WINAPI* Direct3DCreate9On12Ex_t)(UINT SDKVersion, D3D9ON12_ARGS* pOverrideList, UINT NumOverrideEntries, IDirect3D9Ex** ppOutputInterface);
typedef IDirect3D9* (WINAPI* Direct3DCreate9On12_t)(UINT SDKVersion, D3D9ON12_ARGS* pOverrideList, UINT NumOverrideEntries);
typedef int (WINAPI* D3DPERF_BeginEvent_t)(D3DCOLOR col, LPCWSTR wszName);
typedef int (WINAPI* D3DPERF_EndEvent_t)(void);
typedef DWORD (WINAPI* D3DPERF_GetStatus_t)(void);
typedef BOOL (WINAPI* D3DPERF_QueryRepeatFrame_t)(void);
typedef void (WINAPI* D3DPERF_SetMarker_t)(D3DCOLOR col, LPCWSTR wszName);
typedef void (WINAPI* D3DPERF_SetRegion_t)(D3DCOLOR col, LPCWSTR wszName);
typedef BOOL (WINAPI* D3DPERF_QueryRepeatFrame_t)(void);
typedef void (WINAPI* D3DPERF_SetOptions_t)(DWORD options);
typedef int (WINAPI* DebugSetLevel_t)(void);
typedef void (WINAPI* DebugSetMute_t)(void);
typedef void (WINAPI* Direct3DShaderValidatorCreate9_t)(void);
typedef HRESULT(WINAPI* PSGPError_t)(void);
typedef HRESULT(WINAPI* PSGPSampleTexture_t)(void);

extern const char* const g_moduleName;

FARPROC getOrigFunc(const char* name);

#define CREATE_DIRECT3D9_FUNC(name) \
    extern "C" IDirect3D9* WINAPI name(UINT SDKVersion) \
    { \
        HRESULT hr = 0; \
    	char path[MAX_PATH]; \
        char d3d9lib[MAX_PATH]; \
        char d3d12lib[MAX_PATH]; \
        char dxgilib[MAX_PATH]; \
        path[0] = 0; \
        GetSystemDirectory(path, sizeof(path)); \
        StringCbCat(path, sizeof(path), "\\"); \
        StringCbCopy(d3d9lib, sizeof(d3d9lib), path); \
        StringCbCopy(d3d12lib, sizeof(d3d12lib), path); \
        StringCbCopy(dxgilib, sizeof(dxgilib), path); \
        StringCbCat(d3d9lib, sizeof(d3d9lib), "d3d9.dll"); \
        StringCbCat(d3d12lib, sizeof(d3d12lib), "d3d12.dll"); \
        StringCbCat(dxgilib, sizeof(dxgilib), "dxgi.dll"); \
        IDirect3D9* d3d9 = NULL; \
        IDXGIAdapter1* adapter = NULL; \
        IDXGIFactory4* factory = NULL; \
        ID3D12Device* pD3D12Device = NULL; \
        D3D9ON12_ARGS Direct3D9On12args = { TRUE, NULL, { NULL, NULL }, 0, 0 }; \
        HMODULE pD3D9 = LoadLibrary(d3d9lib); \
        HMODULE pDXGI = LoadLibrary(dxgilib); \
        HMODULE pD3D12 = LoadLibrary(d3d12lib); \
        if (!pDXGI || !pD3D9 || !pD3D12) { fprintf(stderr, "Could not load dxgi.dll or d3d9.dll or d3d12.dll\n"); return NULL; } \
        CreateDXGIFactory2_t pDXGIFactory2 = (CreateDXGIFactory2_t)GetProcAddress(pDXGI, "CreateDXGIFactory2"); \
		D3D12CreateDevice_t pD3D12CreateDevice = (D3D12CreateDevice_t)GetProcAddress(pD3D12, "D3D12CreateDevice"); \
		Direct3DCreate9On12_t pDirect3DCreate9On12 = (Direct3DCreate9On12_t)GetProcAddress(pD3D9, "Direct3DCreate9On12"); \
        if (!pDXGIFactory2) { fprintf(stderr, "Could not load CreateDXGIFactory 2\n");  return NULL; } \
		if (!pD3D12CreateDevice) { fprintf(stderr, "Could not load D3D12CreateDevice\n"); return NULL; } \
		if (!pDirect3DCreate9On12) { fprintf(stderr, "Could not load Direct3DCreate9On12\n"); return NULL; } \
        hr = pDXGIFactory2(0, __uuidof(IDXGIFactory4), (void **)&factory); \
        if (FAILED(hr)) { fprintf(stderr, "CreateDXGIFactory2 error: 0x%x\n", hr); FreeLibrary(pDXGI); FreeLibrary(pD3D9); FreeLibrary(pD3D12); return NULL; } \
        for (UINT adapterIndex = 0; ; adapterIndex++) { \
            hr = factory->EnumAdapters1(adapterIndex, &adapter); \
            if (hr == DXGI_ERROR_NOT_FOUND) { fprintf(stderr, "EnumAdapters1 error: 0x%x\n", hr); factory->Release(); FreeLibrary(pDXGI); FreeLibrary(pD3D9); FreeLibrary(pD3D12); return NULL; } \
            hr = pD3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), (void **)&pD3D12Device); \
            if (SUCCEEDED(hr)) { break; } \
        } \
        Direct3D9On12args.pD3D12Device = (IUnknown *)pD3D12Device; \
		d3d9 = pDirect3DCreate9On12(SDKVersion, &Direct3D9On12args, 1); \
		FreeLibrary(pDXGI); \
		FreeLibrary(pD3D9); \
		FreeLibrary(pD3D12); \
		return d3d9; \
    }

#define CREATE_DIRECT3D9EX_FUNC(name) \
    extern "C" HRESULT WINAPI name(UINT SDKVersion, IDirect3D9Ex **out) \
    { \
        HRESULT hr = 0; \
    	char path[MAX_PATH]; \
        char d3d9lib[MAX_PATH]; \
        char d3d12lib[MAX_PATH]; \
        char dxgilib[MAX_PATH]; \
        path[0] = 0; \
        GetSystemDirectory(path, sizeof(path)); \
        StringCbCat(path, sizeof(path), "\\"); \
        StringCbCopy(d3d9lib, sizeof(d3d9lib), path); \
        StringCbCopy(d3d12lib, sizeof(d3d12lib), path); \
        StringCbCopy(dxgilib, sizeof(dxgilib), path); \
        StringCbCat(d3d9lib, sizeof(d3d9lib), "d3d9.dll"); \
        StringCbCat(d3d12lib, sizeof(d3d12lib), "d3d12.dll"); \
        StringCbCat(dxgilib, sizeof(dxgilib), "dxgi.dll"); \
        IDirect3D9* d3d9 = NULL; \
        IDXGIAdapter1* adapter = NULL; \
        IDXGIFactory4* factory = NULL; \
        ID3D12Device* pD3D12Device = NULL; \
        D3D9ON12_ARGS Direct3D9On12args = { TRUE, NULL, { NULL, NULL }, 0, 0 }; \
        HMODULE pD3D9 = LoadLibrary(d3d9lib); \
        HMODULE pDXGI = LoadLibrary(dxgilib); \
        HMODULE pD3D12 = LoadLibrary(d3d12lib); \
        if (!pDXGI || !pD3D9 || !pD3D12) { fprintf(stderr, "Could not load dxgi.dll or d3d9.dll or d3d12.dll\n"); return NULL; } \
        CreateDXGIFactory2_t pDXGIFactory2 = (CreateDXGIFactory2_t)GetProcAddress(pDXGI, "CreateDXGIFactory2"); \
        D3D12CreateDevice_t pD3D12CreateDevice = (D3D12CreateDevice_t)GetProcAddress(pD3D12, "D3D12CreateDevice"); \
        Direct3DCreate9On12Ex_t pDirect3DCreate9On12Ex = (Direct3DCreate9On12Ex_t)GetProcAddress(pD3D9, "Direct3DCreate9On12Ex"); \
        if (!pDXGIFactory2) { fprintf(stderr, "Could not load CreateDXGIFactory 2\n");  return NULL; } \
        if (!pD3D12CreateDevice) { fprintf(stderr, "Could not load D3D12CreateDevice\n"); return NULL; } \
		if (!pDirect3DCreate9On12Ex) { fprintf(stderr, "Could not load Direct3DCreate9On12Ex\n"); return NULL; } \
        hr = pDXGIFactory2(0, __uuidof(IDXGIFactory4), (void **)&factory); \
        if (FAILED(hr)) { fprintf(stderr, "CreateDXGIFactory2 error: 0x%x\n", hr); FreeLibrary(pDXGI); FreeLibrary(pD3D9); FreeLibrary(pD3D12); return NULL; } \
        for (UINT adapterIndex = 0; ; adapterIndex++) { \
            hr = factory->EnumAdapters1(adapterIndex, &adapter); \
            if (hr == DXGI_ERROR_NOT_FOUND) { fprintf(stderr, "EnumAdapters1 error: 0x%x\n", hr); factory->Release(); FreeLibrary(pDXGI); FreeLibrary(pD3D9); FreeLibrary(pD3D12); return NULL; } \
            hr = pD3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), (void **)&pD3D12Device); \
            if (SUCCEEDED(hr)) { break; } \
        } \
        Direct3D9On12args.pD3D12Device = (IUnknown *)pD3D12Device; \
        hr = pDirect3DCreate9On12Ex(SDKVersion, &Direct3D9On12args, 1, out); \
        FreeLibrary(pDXGI); \
        FreeLibrary(pD3D9); \
        FreeLibrary(pD3D12); \
        return hr; \
    }

#define CREATE_D3DPERF_BEGINEVENT_FUNC(name) \
    namespace { static D3DPERF_BeginEvent_t g_orig##name = (D3DPERF_BeginEvent_t)getOrigFunc(#name); } \
    extern "C" int WINAPI name(D3DCOLOR col, LPCWSTR wszName) \
	{ \
        return g_orig##name(col, wszName); \
	}

#define CREATE_D3DPERF_ENDEVENT_FUNC(name) \
    namespace { static D3DPERF_EndEvent_t g_orig##name = (D3DPERF_EndEvent_t)getOrigFunc(#name); } \
    extern "C" int WINAPI name(void) \
	{ \
        return g_orig##name(); \
	}

#define CREATE_D3DPERF_GETSTATUS_FUNC(name) \
    namespace { static D3DPERF_GetStatus_t g_orig##name = (D3DPERF_GetStatus_t)getOrigFunc(#name); } \
    extern "C" DWORD WINAPI name(void) \
	{ \
        return g_orig##name(); \
	}

#define CREATE_D3DPERF_QUERYREPEATFRAME_FUNC(name) \
    namespace { static D3DPERF_QueryRepeatFrame_t g_orig##name = (D3DPERF_QueryRepeatFrame_t)getOrigFunc(#name); } \
    extern "C" BOOL WINAPI name(void) \
	{ \
        return g_orig##name(); \
	}

#define CREATE_D3DPERF_SETMARKER_FUNC(name) \
    namespace { static D3DPERF_SetMarker_t g_orig##name = (D3DPERF_SetMarker_t)getOrigFunc(#name); } \
    extern "C" void WINAPI name(D3DCOLOR col, LPCWSTR wszName) \
	{ \
        return g_orig##name(col, wszName); \
	}

#define CREATE_D3DPERF_SETOPTIONS_FUNC(name) \
    namespace { static D3DPERF_SetOptions_t g_orig##name = (D3DPERF_SetOptions_t)getOrigFunc(#name); } \
    extern "C" void WINAPI name(DWORD options) \
	{ \
        g_orig##name(options); \
	}

#define CREATE_D3DPERF_SETREGION_FUNC(name) \
    namespace { static D3DPERF_SetRegion_t g_orig##name = (D3DPERF_SetRegion_t)getOrigFunc(#name); } \
    extern "C" void WINAPI name(D3DCOLOR col, LPCWSTR wszName) \
	{ \
        g_orig##name(col, wszName); \
	}

#define CREATE_DEBUGSETLEVEL_FUNC(name) \
    namespace { static DebugSetLevel_t g_orig##name = (DebugSetLevel_t)getOrigFunc(#name); } \
    extern "C" int WINAPI name(void) \
	{ \
        return g_orig##name(); \
	}

#define CREATE_DEBUGSETMUTE_FUNC(name) \
    namespace { static DebugSetMute_t g_orig##name = (DebugSetMute_t)getOrigFunc(#name); } \
    extern "C" void WINAPI name(void) \
	{ \
        g_orig##name(); \
	}

#define CREATE_CREATED3D9ON12_FUNC(name) \
    namespace { static Direct3DCreate9On12_t g_orig##name = (Direct3DCreate9On12_t)getOrigFunc(#name); } \
    extern "C" IDirect3D9* WINAPI name(UINT SDKVersion, D3D9ON12_ARGS* pOverrideList, UINT NumOverrideEntries) \
	{ \
        return g_orig##name(SDKVersion, pOverrideList, NumOverrideEntries); \
	}

#define CREATE_CREATED3D9ON12EX_FUNC(name) \
    namespace { static Direct3DCreate9On12Ex_t g_orig##name = (Direct3DCreate9On12Ex_t)getOrigFunc(#name); } \
    extern "C" HRESULT WINAPI name(UINT SDKVersion, D3D9ON12_ARGS* pOverrideList, UINT NumOverrideEntries, IDirect3D9Ex** ppOutputInterface) \
	{ \
        return g_orig##name(SDKVersion, pOverrideList, NumOverrideEntries, ppOutputInterface); \
	}

#define CREATE_DIRECT3DSHADERVALIDATORCREATE9_FUNC(name) \
    namespace { static Direct3DShaderValidatorCreate9_t g_orig##name = (Direct3DShaderValidatorCreate9_t)getOrigFunc(#name); } \
    extern "C" void WINAPI name(void) \
	{ \
        g_orig##name(); \
	}

#define CREATE_PSGPERROR_FUNC(name) \
    namespace { static PSGPError_t g_orig##name = (PSGPError_t)getOrigFunc(#name); } \
    extern "C" HRESULT WINAPI name(void) \
	{ \
        return g_orig##name(); \
	}

#define CREATE_PSGPSAMPLETEXTURE_FUNC(name) \
    namespace { static PSGPSampleTexture_t g_orig##name = (PSGPSampleTexture_t)getOrigFunc(#name); } \
    extern "C" HRESULT WINAPI name(void) \
	{ \
        return g_orig##name(); \
	}

#define CREATE_WRAPPER_FUNC(name) \
    namespace { static FARPROC g_orig##name = getOrigFunc(#name); } \
    extern "C" __declspec(dllexport) void name \
	{ \
        g_orig##name(); \
	}
