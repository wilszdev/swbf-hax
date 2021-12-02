#pragma once

// abandoned imgui for this game. it was being huge pain in the butt
#if 0

#include "vendor/imgui/imgui.h"
#include "vendor/imgui/imgui_impl_dx9.h"
#include "vendor/imgui/imgui_impl_win32.h"

static void InitImGui(LPDIRECT3DDEVICE9 device)
{
	ImGui::CreateContext();
	//ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(GetCurrentProcessWindow());
	ImGui_ImplDX9_Init(device);
}

static void ShutdownImGui()
{
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}
static void PollMouseInputForImGui()
{
	for (int i = 0; i < 5; i++) ImGui::GetIO().MouseDown[i] = false;

	if (GetKeyState(VK_LBUTTON) & 0x8000) ImGui::GetIO().MouseDown[0] = true;
	if (GetKeyState(VK_RBUTTON) & 0x8000) ImGui::GetIO().MouseDown[1] = true;
	if (GetKeyState(VK_MBUTTON) & 0x8000) ImGui::GetIO().MouseDown[2] = true;
}

// the wndproc hook for giving input to imgui. for this game, it worked for text but not for mouse button input. weird.
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
static WND_PROC(hkWndProc)
{
	if (showMenu)
	{
		ImGui_ImplWin32_WndProcHandler(window, msg, wparam, lparam);
		return 1;
	}
	return CallWindowProcA((WNDPROC)originalWndProc, window, msg, wparam, lparam);
}

// endscene hook from when i was using imgui. note that it did work, the issue was that it crashed the game on loading screens (rip)

static END_SCENE(hkEndScene2)
{
	if (!d3dDevice)
	{
		InitImGui(device);
		d3dDevice = device;
	}

	PollMouseInputForImGui();

	static bool toggleKeyWasDownLastFrame = false;
	bool toggleKeyDown = GetKeyState(VK_HOME) & 0x8000;
	if (toggleKeyDown && !toggleKeyWasDownLastFrame)
	{
		showMenu = !showMenu;
		ImGui::GetIO().MouseDrawCursor = showMenu;
	}

	static float value = 6.9f;

	if (showMenu)
	{
		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("nice man");

		ImGui::Text("well isnt this just splendid");
		ImGui::DragFloat("here is a cool value", &value, 0.1f, 0.0f, 10.0f);

		ImGui::End();

		ImGui::ShowDemoWindow();

		ImGui::EndFrame();
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	}

	toggleKeyWasDownLastFrame = toggleKeyDown;
	return originalEndScene(device);
}

// was also suspending threads before unhooking, bc the wndproc hook did some weird things sometimes
	//HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, GetCurrentProcessId());
	//if (snapshot && snapshot != INVALID_HANDLE_VALUE)
	//{
	//	std::vector<HANDLE> threads{ 8, 0 };
	//
	//	THREADENTRY32 te = {};
	//	if (Thread32First(snapshot, &te))
	//	{
	//		do
	//		{
	//			if (te.th32ThreadID != GetCurrentThreadId())
	//			{
	//				HANDLE thread = OpenThread(THREAD_ALL_ACCESS, 0, te.th32ThreadID);
	//				if (!thread || thread == INVALID_HANDLE_VALUE)
	//					break;
	//				threads.push_back(thread);
	//				// suspend thread
	//				SuspendThread(thread);
	//				CONTEXT threadContext = {};
	//				GetThreadContext(thread, &threadContext); // thread has to suspended in order for kernel to save context. avoid race condition.
	//			}
	//		} while (Thread32Next(snapshot, &te));
	//	}
	// resume threads
	//	for (HANDLE thread : threads)
	//	{
	//		ResumeThread(thread);
	//		CloseHandle(thread);
	//	}
	//	CloseHandle(snapshot);
	//}

#endif
