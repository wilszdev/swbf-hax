#pragma once
#include "WindowsLessBloat.h"
#include <d3d9.h>
#include <cstdint>

namespace dx
{
#define END_SCENE(name) HRESULT APIENTRY name(LPDIRECT3DDEVICE9 device)
	typedef END_SCENE(EndScene_fn);
	END_SCENE(hkDirectX_EndScene);

#define RESET(name) HRESULT APIENTRY name(LPDIRECT3DDEVICE9 device, D3DPRESENT_PARAMETERS* params)
	typedef RESET(Reset_fn);
	RESET(hkDirectX_Reset);

	bool CopyVtable(HWND window);
	bool CreateHooks(HWND window);
	void RemoveHooks();
}