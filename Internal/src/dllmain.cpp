#define IMGUI_IMPL_WIN32_DISABLE_GAMEPAD
#include "WindowsLessBloat.h"
#include "di.h"
#include "dx.h"
#include "util.h"
#include "drawing.hpp"
#include <cstdio>

#include "swbf-reclass.h"

#define WHITE D3DCOLOR_ARGB(0xFF, 0xFF, 0xFF, 0xFF)
#define BLACK D3DCOLOR_ARGB(0xFF, 0x00, 0x00, 0x00)
#define RED   D3DCOLOR_ARGB(0xFF, 0xFF, 0x00, 0x00)
#define GREEN D3DCOLOR_ARGB(0xFF, 0x00, 0xFF, 0x00)
#define BLUE  D3DCOLOR_ARGB(0xFF, 0x00, 0x00, 0xFF)

#include "vendor/imgui/imgui.h"
#include "vendor/imgui/imgui_impl_dx9.h"
#include "vendor/imgui/imgui_impl_win32.h"

static void InitImGui(LPDIRECT3DDEVICE9 device, HWND window)
{
	puts("initialising imgui");
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.MouseDrawCursor = true;
	io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(window);
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

static LPD3DXFONT font = nullptr;

void hkDirectX_Reset()
{
	if (font)
	{
		font->Release();
		font = nullptr;
	}
	ImGui_ImplDX9_InvalidateDeviceObjects();
}

static bool showImGuiMenu = false;
static uintptr_t exeBase = 0;
void hkDirectX_EndScene(LPDIRECT3DDEVICE9 device)
{
	static HWND windowHandle = nullptr;
	static bool didShutdown = false;
	static bool initialised = false;
	if (!exeBase) exeBase = (uintptr_t)GetModuleHandleA(NULL);
	if (didShutdown) return;
	if (!windowHandle) windowHandle = util::GetCurrentProcessWindow();
	if (!initialised)
	{
		InitImGui(device, windowHandle);
		initialised = true;
	}

	bool isInLoadingScreen = *(bool*)(exeBase + 0x4EED59);

	static bool toggleKeyDownLastFrame = false;
	bool toggleKeyDownThisFrame = (GetAsyncKeyState(VK_NUMPAD9));
	if (toggleKeyDownThisFrame && !toggleKeyDownLastFrame) showImGuiMenu = !showImGuiMenu;

	if (!font) font = drawing::CreateASingleFont(device, "Arial");
	drawing::WriteText(font, "press NUMPAD9 for hax", 10, 10, 100, 100, DT_LEFT, RED);

#define profileName ((wchar_t*)(exeBase + 0x5AB0D2))
	//#define spawnManager (*((SpawnManager**)(exeBase + 0x62EA50)))
#define helpfulData ((*(TheStartOfSomeRandomDataThing**)(exeBase + 0x01D95D24)))
#define playerDataYay (helpfulData->localPlayerAimer.classWithPlayerDataYay)
#define entitySoldier (helpfulData->localPlayerAimer.classWithPlayerDataYay->currentEntitySoldierInstance)

	static bool lockHealth = false;
	static float healthValue = 300.0f;
	static bool infiniteAmmo = false;

	// if the high 2 bits of an address are 0x3F then it's been cleared by game
	//hopefully
#define PTR_IS_VALID(ptr) ((ptr) && (((uintptr_t)(ptr) & 0xFF000000) != 0x3F000000))

	if (infiniteAmmo &&
		PTR_IS_VALID(helpfulData) &&
		PTR_IS_VALID(helpfulData->localPlayerAimer.currentGun))
	{
		helpfulData->localPlayerAimer.currentGun->proportionOfAmmunitionLeftInMag = 1.0;
		helpfulData->localPlayerAimer.currentGun->weaponHeat = 0.0;
	}

	if (PTR_IS_VALID(helpfulData) &&
		PTR_IS_VALID(playerDataYay) &&
		PTR_IS_VALID(entitySoldier))
	{
		if (lockHealth)
		{
			entitySoldier->curHealth = healthValue;
		}
		else
		{
			healthValue = entitySoldier->curHealth;
		}
	}

	if (showImGuiMenu)
	{
		auto& io = ImGui::GetIO();

		// use the position of the in game cursor, bc it locks to the window for us and stuff like that.
		RECT clientRect = { 0 };
		GetClientRect(windowHandle, &clientRect);

		// for some odd reason, (1024, 624) is the bottom left position for the in-game cursor.
		// it is independent of resolution or window size.
		io.MousePos.x = ((float)(*(int*)(exeBase + 0x7027C4))) / 1024.0f * (float)clientRect.right;
		io.MousePos.y = ((float)(*(int*)(exeBase + 0x7027C8))) / 624.0f * (float)clientRect.bottom;
		io.WantSetMousePos = true;

		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("lmao, swbf hax");

		//ImGui::Text("setting mouse to be at (%d, %d) in the client area", (int)io.MousePos.x, (int)io.MousePos.y);

		if (profileName && *profileName)
		{
			char tmpProfileName[32] = { 0 };
			wcstombs_s(0, tmpProfileName, profileName, 32);
			ImGui::Text("Hello there, %s.", tmpProfileName);
		}

		/*if (PTR_IS_VALID(spawnManager) &&
			PTR_IS_VALID(spawnManager->playerCharacter) &&
			PTR_IS_VALID(spawnManager->playerCharacter->currentClass) &&
			PTR_IS_VALID(spawnManager->playerCharacter->currentClass->className) &&
			PTR_IS_VALID(spawnManager->playerCharacter->currentTeam) &&
			PTR_IS_VALID(spawnManager->playerCharacter->currentTeam->teamName))
		{
			char tmpTeamName[32] = { 0 };
			char tmpClassName[32] = { 0 };

			wcstombs_s(0, tmpTeamName, spawnManager->playerCharacter->currentTeam->teamName, 32);
			wcstombs_s(0, tmpClassName, spawnManager->playerCharacter->currentClass->className, 32);
			ImGui::Text("you are on team %s, playing as %s\n", tmpTeamName, tmpClassName);
		}*/

		if (PTR_IS_VALID(helpfulData) &&
			PTR_IS_VALID(playerDataYay) &&
			PTR_IS_VALID(entitySoldier))
		{
			ImGui::Checkbox("lock health", &lockHealth);
			if (lockHealth)
			{
				ImGui::SliderFloat("health", &healthValue, 0.0f, 5000.0f);
			}
			else
			{
				ImGui::SliderFloat("health", &(entitySoldier->curHealth), 0.0f, 5000.0f);
				healthValue = entitySoldier->curHealth;
			}
		}

		ImGui::Checkbox("infinite ammo (primary weapon)", &infiniteAmmo);

		ImGui::End();

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
		dx::RemoveHooks();
		di::RemoveHooks();
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

void hkDirectInput_GetDeviceState(DIMOUSESTATE2* mouseState)
{
	if (showImGuiMenu)
	{
		if (!exeBase) exeBase = (uintptr_t)GetModuleHandleA(NULL);

		// pass mouse clicks to imgui
		auto& io = ImGui::GetIO();
		for (int i = 0; i < 5; ++i)
			io.MouseDown[i] = mouseState->rgbButtons[i];

		// update in-game mouse cursor ourselves
		int* cursorX = (int*)(exeBase + 0x7027C4);
		int* cursorY = (int*)(exeBase + 0x7027C8);

		*cursorX += mouseState->lX;
		*cursorY += mouseState->lY;

		if (*cursorX < 0) *cursorX = 0;
		if (*cursorY < 0) *cursorY = 0;
		if (*cursorX > 1024) *cursorX = 1024;
		if (*cursorY > 624) *cursorX = 624;

		// block mouse inputs for game
		for (int i = 0; i < 8; ++i)
			mouseState->rgbButtons[i] = 0;
		mouseState->lX = 0;
		mouseState->lY = 0;
		mouseState->lZ = 0;
	}
}

static void WINAPI InjectedThread(HMODULE module)
{
	printf("injected dll at %p\n===========================\n\n", module);
	AllocConsole();
	FILE* f = nullptr;
	freopen_s(&f, "CONOUT$", "w", stdout);
	if (dx::CreateHooks(util::GetCurrentProcessWindow()) && di::CreateHooks())
	{
		while (!GetAsyncKeyState(VK_INSERT))
		{
			Sleep(10);
		}
	}
	else
	{
		puts("failed to create hooks. try injecting again.");
	}
	Sleep(1000);
	// release lib
	puts("uninjecting dll...");
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