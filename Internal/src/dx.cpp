#include "dx.h"
#include "hook.h"
#include <cstdio>

extern void hkEndScene(LPDIRECT3DDEVICE9 device);
extern void hkReset();

// Reset() is index 16
// EndScene() is index 42

namespace dx
{
#define RESET_PATCH_BYTECOUNT 5
#define END_SCENE_PATCH_BYTECOUNT 7

	void* deviceVtable[119] = { 0 };
	uint8_t EndSceneOriginalBytes[END_SCENE_PATCH_BYTECOUNT] = { 0 };
	EndScene_fn* originalEndScene = nullptr;
	uint8_t ResetOriginalBytes[RESET_PATCH_BYTECOUNT] = { 0 };
	Reset_fn* originalReset = nullptr;

	bool HookDirect3D(HWND window)
	{
		if (!CopyVtable(window))
			return false;

		printf("Locating D3D methods...\n EndScene is at %p\n Reset is at %p\n DrawIndexedPrimitive is at %p\n",
			deviceVtable[42], deviceVtable[16], deviceVtable[82]);
		// EndScene is the 43rd entry in the vftable (index 42)
		memcpy(EndSceneOriginalBytes, deviceVtable[42], END_SCENE_PATCH_BYTECOUNT);
		originalEndScene = (EndScene_fn*)TrampolineHook(deviceVtable[42], dx::hkEndScene, END_SCENE_PATCH_BYTECOUNT);

		memcpy(ResetOriginalBytes, deviceVtable[16], RESET_PATCH_BYTECOUNT);
		originalReset = (Reset_fn*)TrampolineHook(deviceVtable[16], dx::hkReset, RESET_PATCH_BYTECOUNT);
		return true;
	}

	void UnhookDirect3D()
	{
		puts("unhooking d3d");
		Patch(deviceVtable[42], EndSceneOriginalBytes, END_SCENE_PATCH_BYTECOUNT);
		Patch(deviceVtable[16], ResetOriginalBytes, RESET_PATCH_BYTECOUNT);
		puts(" done");
	}

	bool CopyVtable(HWND window)
	{
		IDirect3D9* d3d = Direct3DCreate9(D3D_SDK_VERSION);
		if (!d3d) return false;

		D3DPRESENT_PARAMETERS params = {};
		params.Windowed = false;
		params.SwapEffect = D3DSWAPEFFECT_DISCARD;
		params.hDeviceWindow = window;

		IDirect3DDevice9* dummyDevice = nullptr;

		HRESULT createdDummyDevice = d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
			window, D3DCREATE_SOFTWARE_VERTEXPROCESSING,
			&params, &dummyDevice);

		if (createdDummyDevice != S_OK)
		{
			// try again, but windowed
			params.Windowed = true;
			createdDummyDevice = d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
				window, D3DCREATE_SOFTWARE_VERTEXPROCESSING,
				&params, &dummyDevice);
			if (createdDummyDevice != S_OK)
			{
				// failed :(
				d3d->Release();
				return false;
			}
		}

		// copy the vtable
		memcpy(deviceVtable, *(void**)dummyDevice, sizeof(deviceVtable));

		// done
		dummyDevice->Release();
		d3d->Release();
		return true;
	}

	END_SCENE(hkEndScene)
	{
		::hkEndScene(device);
		return originalEndScene(device);
	}

	RESET(hkReset)
	{
		// release our resources here
		::hkReset();
		return originalReset(device, params);
	}
}