#include "util.h"
namespace util
{
	static BOOL CALLBACK EnumProc(HWND window, LPARAM param)
	{
		DWORD pid;
		GetWindowThreadProcessId(window, &pid);
		if (pid == GetCurrentProcessId())
		{
			*reinterpret_cast<HWND*>(param) = window;
			return FALSE;
		}
		return TRUE;
	}

	HWND GetCurrentProcessWindow()
	{
		HWND window = 0;
		EnumWindows(EnumProc, (LPARAM)&window);
		return window;
	}
}