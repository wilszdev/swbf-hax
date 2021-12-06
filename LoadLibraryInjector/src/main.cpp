#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <TlHelp32.h>
#include <cstdio>

DWORD GetProcIdByName(const char* pName);

int main(int argc, char** argv)
{
	if (argc < 3)
	{
		printf("usage: %s <process name> <dll path>", argv[0]);
		return -1;
	}

	DWORD pid = GetProcIdByName(argv[1]);
	if (!pid) return -1;
	printf("found '%s', pid %d (0x%04x)\n", argv[1], pid, pid);

	HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, 0, pid);
	if (!process || process == INVALID_HANDLE_VALUE) return -1;
	printf("obtained process handle 0x%04zx\n", (uintptr_t)process);

	void* memory = VirtualAllocEx(process, 0, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (!memory) return -1;
	printf("allocated memory at 0x%04zx\n", (uintptr_t)memory);

	DWORD length = GetFullPathNameA(argv[2], 0, 0, 0);
	char* buffer = new char[length];

	if (!GetFullPathNameA(argv[2], length, buffer, 0))
	{
		delete[] buffer;
		puts("[-] failed to expand relative path");
		return -1;
	}

	if (GetFileAttributesA(buffer) == INVALID_FILE_ATTRIBUTES)
	{
		delete[] buffer;
		printf("[-] could not find file \"%s\"\n", argv[2]);
		return -1;
	}

	if (!WriteProcessMemory(process, memory, buffer, length, 0))
	{
		delete[] buffer;
		VirtualFreeEx(process, memory, 0, MEM_RELEASE);
		return -1;
	}
	printf("wrote dll path ('%s') into memory\n", buffer);
	delete[] buffer;

	HANDLE thread = CreateRemoteThread(process, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, memory, 0, 0);
	if (!thread || thread == INVALID_HANDLE_VALUE)
	{
		VirtualFreeEx(process, memory, 0, MEM_RELEASE);
		return -1;
	}
	printf("created remote thread, handle 0x%04zx\n", (uintptr_t)thread);

	WaitForSingleObject(thread, INFINITE);
	CloseHandle(thread);

	VirtualFreeEx(process, memory, 0, MEM_RELEASE);

	CloseHandle(process);

	puts("done");
	return 0;
}

DWORD GetProcIdByName(const char* pName)
{
	DWORD pid = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hSnap != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32 procEntry = {};
		procEntry.dwSize = sizeof(procEntry);

		BOOL ret = Process32First(hSnap, &procEntry);
		while (ret)
		{
			if (!_strcmpi(pName, procEntry.szExeFile))
			{
				pid = procEntry.th32ProcessID;
				break;
			}
			ret = Process32Next(hSnap, &procEntry);
		}
	}
	CloseHandle(hSnap);
	return pid;
}