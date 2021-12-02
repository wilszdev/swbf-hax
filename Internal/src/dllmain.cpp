#define IMGUI_IMPL_WIN32_DISABLE_GAMEPAD
#include "WindowsLessBloat.h"
#include "dx.h"
#include "util.h"
#include "drawing.hpp"
#include <cstdio>

#include "swbf-reclass.h"
//BOOL WINAPI InjectedThread(HMODULE module)
//{
//	BOOL createdConsole = FALSE;
//	FILE* f = nullptr;
//	if ((createdConsole = AllocConsole()) == TRUE)
//	{
//		freopen_s(&f, "CONOUT$", "w", stdout);
//	}
//
//	printf("dll injected at 0x%p\n", module);
//	puts("press INS to terminate this thread (without closing the process)");
//
//	uintptr_t exeBase = (uintptr_t)GetModuleHandleA(NULL);
//#define spawnManager (*((SpawnManager**)(exeBase + 0x62EA50)))
//#define profileName ((wchar_t*)(exeBase + 0x5AB0D2))
//#define helpfulData ((*(TheStartOfSomeRandomDataThing**)(exeBase + 0x01D95D24)))
//
//	bool hasSaidHello = false;
//	while (!GetAsyncKeyState(VK_INSERT))
//	{
//		if (!hasSaidHello &&
//			profileName && *profileName)
//		{
//			wprintf(L"hello there %s\n", profileName);
//			hasSaidHello = true;
//		}
//
//		if (GetAsyncKeyState(VK_NUMPAD1))
//			if (spawnManager && spawnManager->playerCharacter &&
//				spawnManager->playerCharacter->currentClass &&
//				spawnManager->playerCharacter->currentClass->className &&
//				spawnManager->playerCharacter->currentTeam &&
//				spawnManager->playerCharacter->currentTeam->teamName)
//			{
//				wprintf(L"you are on team %s, playing as %s\n",
//					spawnManager->playerCharacter->currentTeam->teamName,
//					spawnManager->playerCharacter->currentClass->className);
//			}
//
//#define playerDataYay helpfulData->localPlayerAimer.classWithPlayerDataYay
//#define entitySoldier helpfulData->localPlayerAimer.classWithPlayerDataYay->currentEntitySoldierInstance
//
//		if (GetAsyncKeyState(VK_NUMPAD2))
//			if (helpfulData && playerDataYay && (void*)playerDataYay != (void*)0x3F800000 && entitySoldier)
//			{
//				printf("health: %f\n", entitySoldier->curHealth);
//			}
//
//		if (GetAsyncKeyState(VK_NUMPAD3))
//			if (helpfulData && playerDataYay && (void*)playerDataYay != (void*)0x3F800000 && entitySoldier)
//			{
//				entitySoldier->curHealth += 100.0f;
//				printf("+100 health (%f)\n", entitySoldier->curHealth);
//			}
//
//		Sleep(50);
//	}
//
//	if (f) fclose(f);
//	if (createdConsole) FreeConsole();
//	FreeLibraryAndExitThread(module, 0);
//	return TRUE;
//}

#define WHITE D3DCOLOR_ARGB(0xFF, 0xFF, 0xFF, 0xFF)
#define BLACK D3DCOLOR_ARGB(0xFF, 0x00, 0x00, 0x00)
#define RED   D3DCOLOR_ARGB(0xFF, 0xFF, 0x00, 0x00)
#define GREEN D3DCOLOR_ARGB(0xFF, 0x00, 0xFF, 0x00)
#define BLUE  D3DCOLOR_ARGB(0xFF, 0x00, 0x00, 0xFF)

#include "vendor/imgui/imgui.h"
#include "vendor/imgui/imgui_impl_dx9.h"
#include "vendor/imgui/imgui_impl_win32.h"

static void InitImGui(LPDIRECT3DDEVICE9 device)
{
	puts("initialising imgui");
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.MouseDrawCursor = true;
	//io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(util::GetCurrentProcessWindow());
	ImGui_ImplDX9_Init(device);
	puts(" completed init.");
}

static void ShutdownImGui()
{
	puts("shutting down imgui");
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	puts(" completed shutdown.");
}

static void PollMouseInputForImGui()
{
	for (int i = 0; i < 5; i++) ImGui::GetIO().MouseDown[i] = false;

	if (GetKeyState(VK_LBUTTON) & 0x8000) ImGui::GetIO().MouseDown[0] = true;
	if (GetKeyState(VK_RBUTTON) & 0x8000) ImGui::GetIO().MouseDown[1] = true;
	if (GetKeyState(VK_MBUTTON) & 0x8000) ImGui::GetIO().MouseDown[2] = true;
}

static LPD3DXFONT font = nullptr;

void hkReset()
{
	if (font)
	{
		font->Release();
		font = nullptr;
	}
	ImGui_ImplDX9_InvalidateDeviceObjects();
}

static bool didShutdown = false;
void hkEndScene(LPDIRECT3DDEVICE9 device)
{
	if (didShutdown) return;
	static bool initialised = false;
	if (!initialised)
	{
		InitImGui(device);
		initialised = true;
	}

	PollMouseInputForImGui();

	bool isInLoadingScreen = *(bool*)((uintptr_t)GetModuleHandle(NULL) + 0x4EED59);
	static bool showImGuiMenu = false;

	static bool toggleKeyDownLastFrame = false;
	bool toggleKeyDownThisFrame = (GetAsyncKeyState(VK_NUMPAD9));
	if (toggleKeyDownThisFrame && !toggleKeyDownLastFrame)
	{
		puts("toggling imgui");
		showImGuiMenu = !showImGuiMenu;
	}

	if (!font) font = drawing::CreateASingleFont(device, "Arial");
	drawing::WriteText(font, "press NUMPAD9 for hax", 25, 100, 100, 100, DT_LEFT, RED);

	drawing::DrawFilledRect(device, 25, 25, 35, 35, WHITE);

	if (showImGuiMenu)
	{
		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("nice man");

		uintptr_t exeBase = (uintptr_t)GetModuleHandleA(NULL);
#define spawnManager (*((SpawnManager**)(exeBase + 0x62EA50)))
#define profileName ((wchar_t*)(exeBase + 0x5AB0D2))
#define helpfulData ((*(TheStartOfSomeRandomDataThing**)(exeBase + 0x01D95D24)))

#define playerDataYay helpfulData->localPlayerAimer.classWithPlayerDataYay
#define entitySoldier helpfulData->localPlayerAimer.classWithPlayerDataYay->currentEntitySoldierInstance
		if (helpfulData && playerDataYay && (void*)playerDataYay != (void*)0x3F800000 && entitySoldier)
		{
			ImGui::DragFloat("health", &(entitySoldier->curHealth), 100.0f, 0.0f, 5000.0f);
		}

		static float value = 6.9f;
		ImGui::Text("well isnt this just splendid");
		ImGui::DragFloat("here is a cool value", &value, 0.1f, 0.0f, 10.0f);

		ImGui::End();

		ImGui::ShowDemoWindow();

		ImGui::EndFrame();
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	}

	if (isInLoadingScreen)
	{
		drawing::DrawFilledRect(device, 100, 25, 25, 25, GREEN);
	}

	toggleKeyDownLastFrame = toggleKeyDownThisFrame;
	if (GetAsyncKeyState(VK_INSERT))
	{
		puts("manz pushed insert key");
		if (font)
		{
			font->Release();
			font = nullptr;
		}
		ShutdownImGui();
		didShutdown = true;
		dx::UnhookDirect3D();
		//HMODULE thisLib = GetModuleHandleA("Internal.dll");
		//if (thisLib && thisLib != INVALID_HANDLE_VALUE)
		//{
		//	printf("attempting to free hax dll (module base %p)", thisLib);
		//	FreeLibrary(thisLib);
		//}
		//else
		//	puts(" could not get handle to Internal.dll");
	}
}

static void WINAPI InjectedThread(HMODULE module)
{
	printf("injected dll at %p", module);
	AllocConsole();
	FILE* f = nullptr;
	freopen_s(&f, "CONOUT$", "w", stdout);
	if (dx::HookDirect3D(util::GetCurrentProcessWindow()))
	{
		while (!GetAsyncKeyState(VK_INSERT))
		{
			Sleep(10);
		}
		//dx::UnhookDirect3D();
		//ShutdownImGui();
		//if (font)
		//{
		//	font->Release();
		//}
	}
	else
	{
		puts("failed to hook d3d. exiting... (this window will remain open)");
	}

	Sleep(1000);
	// release lib
	FreeLibraryAndExitThread(module, 0);
}

BOOL WINAPI DllMain(HMODULE module, DWORD reason, LPVOID reserved)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
#pragma warning(push)
#pragma warning(disable : 6387)
		CloseHandle(CreateThread(0, 0, (LPTHREAD_START_ROUTINE)InjectedThread, module, 0, 0));
#pragma warning(pop)
	}
	return TRUE;
}