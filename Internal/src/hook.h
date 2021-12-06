#pragma once

void Patch(void* dst, void* src, size_t length);

bool Hook(void* src, void* dst, size_t length);

void* TrampolineHook(void* src, void* dst, size_t length);

void* VftableHook(void** vftable, size_t index, void* hookFn);

// wndproc hooks
#if 0
#define WND_PROC(name) LRESULT CALLBACK name(HWND window, UINT msg, WPARAM wparam, LPARAM lparam)
typedef WND_PROC(WndProc_fn);
static WndProc_fn* originalWndProc = nullptr;

static void* HookWndProc(HWND window, void* newWndProc)
{
	return IsWindowUnicode(window) ?
		(WndProc_fn*)SetWindowLongW(window, GWL_WNDPROC, (LONG)newWndProc)
		: (WndProc_fn*)SetWindowLongA(window, GWL_WNDPROC, (LONG)newWndProc);
}

static void UnhookWndProc(HWND window, void* originalWndProc)
{
	if (IsWindowUnicode(window))
		SetWindowLongW(window, GWL_WNDPROC, (LONG)originalWndProc);
	else
		SetWindowLongA(window, GWL_WNDPROC, (LONG)originalWndProc);
}
#endif