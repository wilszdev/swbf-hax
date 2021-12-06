#include "di.h"
#include "hook.h"
#include <cstdio>

extern void hkDirectInput_GetDeviceState(DIMOUSESTATE2* mouseState);

namespace di
{
	void** vtable = nullptr;
	GetDeviceState_fn* GetDeviceStateOriginal = nullptr;

	GET_DEVICE_STATE(hkGetDeviceState)
	{
		HRESULT ret = GetDeviceStateOriginal(_this, dataSize, dataPtr);

		DIMOUSESTATE2* mouseState = (DIMOUSESTATE2*)dataPtr;
		::hkDirectInput_GetDeviceState(mouseState);

		return ret;
	}

	bool GetVtable()
	{
		HINSTANCE hInst = (HINSTANCE)GetModuleHandle(NULL);
		IDirectInput8W* pDirectInput = NULL;

		if (DirectInput8Create(hInst, DIRECTINPUT_VERSION, IID_IDirectInput8W, (LPVOID*)&pDirectInput, 0) != DI_OK)
		{
			return false;
		}

		LPDIRECTINPUTDEVICE8W  mouse;
		if (pDirectInput->CreateDevice(GUID_SysMouse, &mouse, 0) != DI_OK)
		{
			pDirectInput->Release();
			return false;
		}

		vtable = *(void***)mouse;

		mouse->Release();
		pDirectInput->Release();
		return true;
	}

	bool CreateHooks()
	{
		if (!GetVtable()) return false;
		GetDeviceStateOriginal = (GetDeviceState_fn*)VftableHook(vtable, 9, hkGetDeviceState);
		puts("hooked direct input");
		return true;
	}

	void RemoveHooks()
	{
		void* discard = VftableHook(vtable, 9, GetDeviceStateOriginal);
		puts("unhooked direct input");
	}
}