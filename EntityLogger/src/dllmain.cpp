#include "WindowsLessBloat.h"
#include <vector>
#include <cstdio>
#include "hook.h"

#define ENTITY_CTOR(name) uintptr_t __fastcall name(uintptr_t _this)
typedef ENTITY_CTOR(entity_ctor_fn);
entity_ctor_fn* OriginalEntityCtor = nullptr;
uint8_t OriginalEntityCtorBytes[6] = { 0 };

std::vector<uintptr_t> entityAddresses;

static ENTITY_CTOR(HkEntityCtor)
{
	entityAddresses.push_back(_this);
	return OriginalEntityCtor(_this);
}

static uintptr_t exeBase = (uintptr_t)GetModuleHandleA(0);
static void InstallHooks()
{
	// the ctor for Entity is here:
	uintptr_t targetAddr = exeBase + 0xCC560;
	OriginalEntityCtor = (entity_ctor_fn*)TrampolineHook(
		(void*)targetAddr, HkEntityCtor, 6);
	puts("installed hook");
}

static void RemoveHooks()
{
	uintptr_t targetAddr = exeBase + 0xCC560;
	Patch((void*)targetAddr, OriginalEntityCtorBytes, 6);
	puts("removed hook");
}

static void WINAPI WorkThread(HMODULE module)
{
	AllocConsole();
	FILE* f = nullptr;
	freopen_s(&f, "CONOUT$", "w", stdout);
	printf("injected dll at 0x%zx\n============\n", (uintptr_t)module);
	InstallHooks();

	size_t index = 0;

	bool oldKeys[3] = { 0 };
	while (!GetAsyncKeyState(VK_INSERT))
	{
		bool newKeys[3] = { 0 };
		newKeys[0] = GetAsyncKeyState(VK_NUMPAD1);
		newKeys[1] = GetAsyncKeyState(VK_NUMPAD2);
		newKeys[2] = GetAsyncKeyState(VK_NUMPAD3);

		// print current
		if (newKeys[0] && !oldKeys[0])
		{
			if (index < entityAddresses.size())
			{
				printf("entity[%zd] is stored at 0x%zx\n", index, entityAddresses[index]);

				uintptr_t addrOfVftable = *(uintptr_t*)entityAddresses[index];
				printf("\tvftable is stored at 0x%zx\n", addrOfVftable);

				uintptr_t addrOf3rdFunc = ((uintptr_t*)addrOfVftable)[2];
				printf("\tthird function in vtable is at 0x%zx\n", addrOf3rdFunc);

				char* entityTypeName = ((char* (*)(void))addrOf3rdFunc)();
				printf("\tentity is of type: %s\n", entityTypeName);
			}
			else
			{
				printf("index %zd is invalid\n", index);
			}
		}

		// increase index
		if (newKeys[1] && !oldKeys[1])
		{
			++index;
		}

		// decrease index
		if (newKeys[2] && !oldKeys[2])
		{
			--index;
		}

		oldKeys[0] = newKeys[0];
		oldKeys[1] = newKeys[1];
		oldKeys[2] = newKeys[2];

		Sleep(10);
	}
	RemoveHooks();
	if (f) fclose(f);
	FreeLibraryAndExitThread(module, 0);
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
#pragma warning(push)
#pragma warning(disable : 6387)
		CloseHandle(CreateThread(0, 0, (LPTHREAD_START_ROUTINE)WorkThread, instance, 0, 0));
#pragma warning(pop)
	}
	return TRUE;
}