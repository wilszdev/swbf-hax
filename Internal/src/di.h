#pragma once
#include <dinput.h>

namespace di
{
#define GET_DEVICE_STATE(name) HRESULT WINAPI name(IDirectInputDevice8W* _this, DWORD dataSize, LPVOID dataPtr)
	typedef GET_DEVICE_STATE(GetDeviceState_fn);
	GET_DEVICE_STATE(hkGetDeviceState);

	bool GetVtable();
	bool CreateHooks();
	void RemoveHooks();
}