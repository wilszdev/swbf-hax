#include "WindowsLessBloat.h"
#include "hook.h"
#include <cstdint>

void Patch(void* dst, void* src, size_t length)
{
	DWORD oldProtect;
	VirtualProtect(dst, length, PAGE_EXECUTE_READWRITE, &oldProtect);
	memcpy(dst, src, length);
	VirtualProtect(dst, length, oldProtect, &oldProtect);
}

bool Hook(void* src, void* dst, size_t length)
{
	// cant fit JMP
	if (length < 5) return false;

	DWORD oldProtect;
	VirtualProtect(src, length, PAGE_EXECUTE_READWRITE, &oldProtect);
	
	memset(src, 0x90, length); // nop
	
	uintptr_t offset = (uintptr_t)dst - (uintptr_t)src - 5;
	*(char*)src = (char)0xE9; // JMP near
	*(uintptr_t*)((uintptr_t)src + 1) = offset;

	VirtualProtect(src, length, oldProtect, &oldProtect);

	return true;
}

void* TrampolineHook(void* src, void* dst, size_t length)
{
	if (length < 5) return nullptr;
	
	void* gateway = (char*)VirtualAlloc(0, length + 5, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (!gateway) return nullptr;
	memcpy(gateway, src, length);

	if (*(char*)src == (char)0xE9)
	{
		intptr_t jmpOffset = 0;
		memcpy(&jmpOffset, (char*)src + 1, sizeof(intptr_t));
		intptr_t jmpDst = (intptr_t)src + jmpOffset + 5;
		intptr_t newOffset = ((intptr_t)jmpDst) - ((intptr_t)gateway + 5);
		memcpy((char*)gateway + 1, &newOffset, sizeof(intptr_t));
	}

	uintptr_t offset = (uintptr_t)src - (uintptr_t)gateway - 5;
	*((char*)gateway + length) = (char)0xE9;
	*(uintptr_t*)((uintptr_t)gateway + length + 1) = offset;

	if (Hook(src, dst, length))
	{
		return gateway;
	}

	return nullptr;
}

void* VftableHook(void** vftable, size_t index, void* hookFn)
{
	void* original = vftable[index];
	Patch(&vftable[index], &hookFn, sizeof(void*));
	return original;
}
