#include "pch.h"
#include "proc.h"
#include "mem.h"
#include "PointerPath.h"

int main()
{
	DWORD pid = proc::GetProcIdByName("battlefront.exe");
	if (!pid) return -1;

	uintptr_t modBase = proc::GetModuleBaseAddress(pid, "battlefront.exe");
	if (!modBase) return -1;

	HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, 0, pid);
	if (!process || process == INVALID_HANDLE_VALUE) return -1;

	PointerPath healthPtrPath{ process, modBase + 0x01D95D24, { 0x3C, 0x24, 0x54, 0x0C } };
	PointerPath maxHealthPtrPath{ process, modBase + 0x01D95D24, { 0x3C, 0x24, 0x54, 0x10 } };
	PointerPath soldierClassNamePtrPath{ process, modBase + 0x01D95D24, { 0x3C, 0x24, 0x54, 0x04, 0x20 } };

	int ticks = 0;
	while (1)
	{
		healthPtrPath.Resolve();
		maxHealthPtrPath.Resolve();
		soldierClassNamePtrPath.Resolve();

		if (ticks % 20 == 0)
		{
			system("cls");

			float health;
			if (healthPtrPath.Read(&health, sizeof(float)))
			{
				printf("health is at 0x%p (value of %f)\n", (void*)healthPtrPath.Ptr(), health);
			}
			else
			{
				puts("failed to read health value");
			}

			char className[16] = {};
			if (soldierClassNamePtrPath.Read(&className, sizeof(className)))
			{
				printf("selected class: %s\n", className);
			}
			else
			{
				puts("failed to read soldier class");
			}
		}

		if (GetAsyncKeyState(VK_INSERT) & 1)
		{
			break;
		}

		if (GetAsyncKeyState(VK_NUMPAD1) & 1)
		{
			float maxHealth;
			if (maxHealthPtrPath.Read(&maxHealth, sizeof(float)))
			{
				healthPtrPath.Write(&maxHealth, sizeof(float));
			}
		}

		Sleep(25);
		++ticks;
		ticks %= 20;
	}

	CloseHandle(process);

	return 0;
}