#define IMGUI_IMPL_WIN32_DISABLE_GAMEPAD
#include "WindowsLessBloat.h"
#include "di.h"
#include "dx.h"
#include "util.h"
#include "drawing.hpp"
#include "scan.h"
#include <cstdio>
#include <unordered_map>

#include "swbf-reclass.h"

#include "vendor/imgui/imgui.h"
#include "vendor/imgui/imgui_impl_dx9.h"
#include "vendor/imgui/imgui_impl_win32.h"

struct shellcode_info
{
	void* GetModuleHandleA;
	void* FreeLibary;
	void* CloseHandle;
	void* Sleep;
};

DWORD WINAPI uninject_shellcode(shellcode_info* param)
{
	((void(*)(DWORD ms))param->Sleep)(1000);
	HMODULE module = ((HMODULE(*)(LPCSTR moduleName))param->GetModuleHandleA)("Internal.dll");
	if (module && module != INVALID_HANDLE_VALUE)
	{
		((BOOL(*)(HMODULE m))param->FreeLibary)(module);
		((BOOL(*)(HANDLE h))param->CloseHandle)(module);
	}
	return 0;
}

static void Uninject()
{
	puts("uninjecting dll...");
	void* memory = VirtualAlloc(0, 0x1000, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (!memory)
	{
		puts(" failed");
		return;
	}

	uint8_t* shellcodeAddress = (uint8_t*)uninject_shellcode;

	// it's a jumptable entry, lets find destination address
	if (*shellcodeAddress == 0xE9)
	{
		int32_t offset = 0;
		memcpy(&offset, shellcodeAddress + 1, 4);
		offset += 5; // length of jmp instruction
		shellcodeAddress += offset;
	}

	printf(" shellcode is located at 0x%zx\n", (uintptr_t)shellcodeAddress);

	memcpy(memory, shellcodeAddress, 0x200); // theres no way the function is all that big

	shellcode_info info = { 0 };
	info.CloseHandle = CloseHandle;
	info.FreeLibary = FreeLibrary;
	info.GetModuleHandleA = GetModuleHandleA;
	info.Sleep = Sleep;

	memcpy((char*)memory + 0x200, &info, sizeof(info));

	printf(" wrote shellcode into memory at 0x%zx\n invoking shellcode...\n", (uintptr_t)memory);

	FreeConsole();

#pragma warning(push)
#pragma warning(disable: 6387)
	CloseHandle(CreateThread(0, 0, (LPTHREAD_START_ROUTINE)memory, (char*)memory + 0x200, 0, 0));
#pragma warning(pop)
}

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

static void ShutdownHax()
{
	if (font)
	{
		font->Release();
		font = nullptr;
	}
	ShutdownImGui();
	dx::RemoveHooks();
	di::RemoveHooks();
	Uninject();
}

// really weird macro, but i guess it works most of the time
#define PTR_IS_VALID(ptr) ((ptr) && (((uintptr_t)(ptr) & 0xFF000000) != 0x3F000000) && (((uintptr_t)(ptr)) != 0x2580E1C) && (((uintptr_t)(ptr)) != 0xF334F334) && (((uintptr_t)(ptr)) >= 0x1000))

static bool showImGuiMenu = false;
static uintptr_t exeBase = 0;

static bool WorldToScreen(LPDIRECT3DDEVICE9 device, D3DXVECTOR3* pos, D3DXVECTOR3* out) {
	D3DVIEWPORT9 viewPort;
	D3DXMATRIX view, projection, world;

	device->GetViewport(&viewPort);
	device->GetTransform(D3DTS_VIEW, &view);
	device->GetTransform(D3DTS_PROJECTION, &projection);
	D3DXMatrixIdentity(&world);

	D3DXVec3Project(out, pos, &viewPort, &projection, &view, &world);
	if (out->z < 1) {
		return true;
	}

	return false;
}

static bool DrawTeamEsp(LPDIRECT3DDEVICE9 device, size_t teamIndex, uint32_t colour)
{
	if (!PTR_IS_VALID((SpawnManager**)(exeBase + 0x62EA50)) ||
		!PTR_IS_VALID(*(SpawnManager**)(exeBase + 0x62EA50))) return false;

	SpawnManager* spawnManager = *(SpawnManager**)(exeBase + 0x62EA50);
	Team* team = spawnManager->Teams[teamIndex];

	if (!PTR_IS_VALID(team)) return false;

	bool retval = true;

	for (int i = 0; i < team->numCharactersTotal; ++i)
	{
		Character* character = team->charactersOnThisTeam[i];

		if (character == spawnManager->playerCharacter) continue; // dont esp ourselves

		if (!PTR_IS_VALID(character) ||
			!PTR_IS_VALID(character->currentSoldierMan) ||
			!PTR_IS_VALID(character->currentSoldierMan->yeAimer.selfPtr))
		{
			retval = false;
			continue;
		};

#define position character->currentSoldierMan->yeAimer.pos

		D3DXVECTOR3 worldPos{ position[0], position[1], position[2] };
		D3DXVECTOR3 screenPos{};

		if (WorldToScreen(device, &worldPos, &screenPos))
		{
			drawing::DrawFilledRect(device, screenPos.x - 2, screenPos.y - 2, 4, 4, colour);
		}
	}
	return retval;
}

void hkDirectX_EndScene(LPDIRECT3DDEVICE9 device)
{
	static bool initialised = false;
	static HWND windowHandle = util::GetCurrentProcessWindow();
	if (!exeBase) exeBase = (uintptr_t)GetModuleHandleA(NULL);
	if (!initialised)
	{
		InitImGui(device, windowHandle);
		initialised = true;
	}

	static bool toggleKeyDownLastFrame = false;
	bool toggleKeyDownThisFrame = (GetAsyncKeyState(VK_DELETE));
	if (toggleKeyDownThisFrame && !toggleKeyDownLastFrame) showImGuiMenu = !showImGuiMenu;

	if (!font) font = drawing::CreateASingleFont(device, "Arial");
	drawing::WriteText(font, "press DELETE for hax", 10, 10, 100, 100, DT_LEFT, D3DCOLOR_ARGB(0xFF, 0xFF, 0x00, 0x00));

	static bool lockHealth = false;
	static float healthValue = 300.0f;
	static bool infiniteAmmo = false;

#pragma region macros for accessing data
#define profileName ((wchar_t*)(exeBase + 0x5AB0D2))
#define spawnManagerPtr (((SpawnManager**)(exeBase + 0x62EA50)))
#define spawnManager (*spawnManagerPtr)
#define helpfulDataPtr (((TheStartOfSomeRandomDataThing**)(exeBase + 0x01D95D24)))
#define helpfulData (*helpfulDataPtr)
#define playerDataYay (helpfulData->localPlayerAimer.classWithPlayerDataYay)
#define entitySoldier (helpfulData->localPlayerAimer.classWithPlayerDataYay->currentEntitySoldierInstance)
#pragma endregion

#pragma region infinite ammo
	if (infiniteAmmo &&
		PTR_IS_VALID(spawnManagerPtr) &&
		PTR_IS_VALID(spawnManager) &&
		PTR_IS_VALID(spawnManager->playerCharacter) &&
		!PTR_IS_VALID(spawnManager->playerCharacter->ptrToControllingThing) &&

		PTR_IS_VALID(helpfulDataPtr) &&
		PTR_IS_VALID(helpfulData) &&
		PTR_IS_VALID(helpfulData->localPlayerAimer.currentWeapon))
	{
		helpfulData->localPlayerAimer.currentWeapon->proportionOfAmmunitionLeftInMag = 1.0;
		helpfulData->localPlayerAimer.currentWeapon->weaponHeat = 0.0;
	}
#pragma endregion

#pragma region health lock
	if (PTR_IS_VALID(spawnManagerPtr) &&
		PTR_IS_VALID(spawnManager) &&
		PTR_IS_VALID(spawnManager->playerCharacter) &&
		!PTR_IS_VALID(spawnManager->playerCharacter->ptrToControllingThing) &&

		PTR_IS_VALID(helpfulDataPtr) &&
		PTR_IS_VALID(helpfulData) &&
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
#pragma endregion

#pragma region esp

	static int espTeamIndex = 1;
	static float espColourRGB[3] = { 0.0f, 1.0f, 0.0f }; ;
	static bool espEnabled = false;

	if (espEnabled)
	{
		DrawTeamEsp(device, espTeamIndex, D3DCOLOR_ARGB(0xFF,
			(uint8_t)(0xFF * espColourRGB[0]),
			(uint8_t)(0xFF * espColourRGB[1]),
			(uint8_t)(0xFF * espColourRGB[2])
		));
	}

#pragma endregion

	bool shouldShutdownHax = false;

	if (showImGuiMenu)
	{
#pragma region cursor
		auto& io = ImGui::GetIO();

		// use the position of the in game cursor, bc it locks to the window for us and stuff like that.
		RECT clientRect = { 0 };
		GetClientRect(windowHandle, &clientRect);

		// for some odd reason, (1024, 624) is the bottom left position for the in-game cursor.
		// it is independent of resolution or window size.
		io.MousePos.x = ((float)(*(int*)(exeBase + 0x7027C4))) / 1024.0f * (float)clientRect.right;
		io.MousePos.y = ((float)(*(int*)(exeBase + 0x7027C8))) / 624.0f * (float)clientRect.bottom;
		io.WantSetMousePos = true;
#pragma endregion

		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

#pragma region main cheat window

#pragma region imgui::begin with profile name
		if (profileName && *profileName)
		{
			char tmpProfileName[32] = { 0 };
			wcstombs_s(0, tmpProfileName, profileName, 32);
			char title[64] = { 0 };
			sprintf_s(title, 64, "Hello there, %s!", tmpProfileName);
			ImGui::Begin(title, &showImGuiMenu);
		}
		else
		{
			ImGui::Begin("Hello there!", &showImGuiMenu);
		}
#pragma endregion

#pragma region team and class names
		if (PTR_IS_VALID(spawnManagerPtr) && PTR_IS_VALID(spawnManager) &&
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
			ImGui::Text("team:%s\nclass:%s\n", tmpTeamName, tmpClassName);
		}
#pragma endregion

#pragma region health lock toggle
		if (PTR_IS_VALID(spawnManagerPtr) &&
			PTR_IS_VALID(spawnManager) &&
			PTR_IS_VALID(spawnManager->playerCharacter) &&
			!PTR_IS_VALID(spawnManager->playerCharacter->ptrToControllingThing) &&

			PTR_IS_VALID(helpfulDataPtr) &&
			PTR_IS_VALID(helpfulData) &&
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
#pragma endregion

#pragma region infinite ammo toggle
		if (PTR_IS_VALID(spawnManagerPtr) &&
			PTR_IS_VALID(spawnManager) &&
			PTR_IS_VALID(spawnManager->playerCharacter) &&
			!PTR_IS_VALID(spawnManager->playerCharacter->ptrToControllingThing) &&

			PTR_IS_VALID(helpfulDataPtr) &&
			PTR_IS_VALID(helpfulData) &&
			PTR_IS_VALID(helpfulData->localPlayerAimer.currentWeapon))
		{
			ImGui::Checkbox("infinite ammo", &infiniteAmmo);
		}
#pragma endregion

#pragma region instant win

		// instant win by setting remaining units of other team to zero
		if (PTR_IS_VALID(spawnManagerPtr) &&
			PTR_IS_VALID(spawnManager) &&
			PTR_IS_VALID(spawnManager->playerCharacter) &&
			PTR_IS_VALID(spawnManager->playerCharacter->currentTeam) &&
			ImGui::Button("instant win"))
		{
			int index = spawnManager->playerCharacter->currentTeam->indexInTeamsArray;

			int targetIndex = -1;
			if (index == 1) targetIndex = 2;
			else if (index == 2) targetIndex = 1;

			if (targetIndex != -1)
			{
				spawnManager->Teams[targetIndex]->remainingUnits = 0;
			}
		}
#pragma endregion

#pragma region esp

		ImGui::Checkbox("esp enabled", &espEnabled);
		ImGui::ColorEdit3("esp colour", espColourRGB);
		ImGui::SliderInt("esp team index", &espTeamIndex, 1, 3);

#pragma endregion

		static bool ordnanceWindowOpen = false;
		if (ImGui::Button(ordnanceWindowOpen ? "Hide ordnance window" : "Show ordnance window"))
			ordnanceWindowOpen = !ordnanceWindowOpen;

		if (ImGui::Button("Uninject hax dll"))
		{
			shouldShutdownHax = true;
		}

		ImGui::End();

#pragma endregion

#pragma region ordnance changer window

		if (ordnanceWindowOpen)
		{
			static std::unordered_map<wchar_t*, uintptr_t> ordnance{};

			if (ImGui::Begin("ordnance changer", &ordnanceWindowOpen))
			{
#pragma region scan
				if (ImGui::Button("scan for ordnance types"))
				{
					ordnance.clear();

					constexpr uintptr_t WEAPON_CANNON_CLASS_VFTABLE_RVA = 0x4273e8;
					constexpr uintptr_t WEAPON_LAUNCHER_CLASS_VFTABLE_RVA = 0x428188;
					constexpr uintptr_t WEAPON_GRENADE_CLASS_VFTABLE_RVA = 0x427e98;
					constexpr uintptr_t WEAPON_CATAPULT_CLASS_VFTABLE_RVA = 0x427584;
					constexpr uintptr_t WEAPON_DESTRUCT_CLASS_VFTABLE_RVA = 0x427700;
					constexpr uintptr_t WEAPON_DETONATOR_CLASS_VFTABLE_RVA = 0x427884;
					constexpr uintptr_t WEAPON_DISPENSER_CLASS_VFTABLE_RVA = 0x427b84;
					constexpr uintptr_t WEAPON_DISGUISE_CLASS_VFTABLE_RVA = 0x427a00;
					constexpr uintptr_t WEAPON_BINOCULARS_CLASS_VFTABLE_RVA = 0x42726c;
					constexpr uintptr_t WEAPON_GRAPPLING_HOOK_CLASS_VFTABLE_RVA = 0x427d20;
					constexpr uintptr_t WEAPON_LASER_CLASS_VFTABLE_RVA = 0x428004;
					constexpr uintptr_t WEAPON_MELEE_CLASS_VFTABLE_RVA = 0x4282f4;
					constexpr uintptr_t WEAPON_SHIELD_CLASS_VFTABLE_RVA = 0x4287c4;
					constexpr uintptr_t WEAPON_TOW_CABLE_CLASS_VFTABLE_RVA = 0x428948;
					constexpr uintptr_t WEAPON_REPAIR_CLASS_VFTABLE_RVA = 0x4285ec;
					constexpr uintptr_t WEAPON_REMOTE_CLASS_VFTABLE_RVA = 0x428478;

					auto VFTABLE_RVA_LIST = {
						WEAPON_CANNON_CLASS_VFTABLE_RVA,
						WEAPON_LAUNCHER_CLASS_VFTABLE_RVA,
						WEAPON_GRENADE_CLASS_VFTABLE_RVA,
						WEAPON_CATAPULT_CLASS_VFTABLE_RVA,
						WEAPON_DESTRUCT_CLASS_VFTABLE_RVA,
						WEAPON_DETONATOR_CLASS_VFTABLE_RVA,
						WEAPON_DISPENSER_CLASS_VFTABLE_RVA,
						WEAPON_DISGUISE_CLASS_VFTABLE_RVA,
						WEAPON_BINOCULARS_CLASS_VFTABLE_RVA,
						WEAPON_GRAPPLING_HOOK_CLASS_VFTABLE_RVA,
						WEAPON_LASER_CLASS_VFTABLE_RVA,
						WEAPON_MELEE_CLASS_VFTABLE_RVA,
						WEAPON_SHIELD_CLASS_VFTABLE_RVA,
						WEAPON_TOW_CABLE_CLASS_VFTABLE_RVA,
						WEAPON_REPAIR_CLASS_VFTABLE_RVA,
						WEAPON_REMOTE_CLASS_VFTABLE_RVA
					};

					for (uintptr_t rva : VFTABLE_RVA_LIST)
					{
						std::vector<uintptr_t> addresses{};

						uint32_t wccVftableAddress = exeBase + rva;
						char pattern[5] = { 0 };
						memcpy(pattern, &wccVftableAddress, 5);

						int numMatches = scan::InternalPatternScanAll(&addresses, pattern, "xxxx", 0x09000000, 0x3000000);
						printf("scan found %d matches:\n", numMatches);

						for (const uintptr_t addr : addresses)
						{
							WeaponCannonClass* wcc = reinterpret_cast<WeaponCannonClass*>(addr);
							if (PTR_IS_VALID(wcc->weaponName) && PTR_IS_VALID(wcc->GeometryName) && PTR_IS_VALID(wcc->ordnancePtr))
							{
								ordnance[wcc->weaponName] = (uintptr_t)wcc->ordnancePtr;
								wprintf(L"ordnance at 0x%zx is for %s\n", (uintptr_t)wcc->ordnancePtr, wcc->weaponName);
							}
						}
					}
				}
#pragma endregion

#pragma region render all the buttons

				std::vector<wchar_t*> namesUsed{ 32, 0 };

				for (auto pair : ordnance)
				{
					wchar_t* name = pair.first;
					uintptr_t ordnancePtr = pair.second;

					char text[32] = { 0 };
					wcstombs_s(0, text, name, 32);

					// if name used already, append "duplicate"
					for (const auto& n : namesUsed)
					{
						if (lstrcmpW(name, n) == 0)
						{
							strcat_s(text, 32, " (duplicate)");
							break;
						}
					}

					namesUsed.emplace_back(name);

					if (ImGui::Button(text))
					{
						if (PTR_IS_VALID(spawnManagerPtr) &&
							PTR_IS_VALID(spawnManager) &&
							PTR_IS_VALID(spawnManager->playerCharacter) &&
							PTR_IS_VALID(spawnManager->playerCharacter->currentSoldierMan) &&
							PTR_IS_VALID(spawnManager->playerCharacter->currentSoldierMan->yeAimer.currentWeapon) &&
							PTR_IS_VALID(spawnManager->playerCharacter->currentSoldierMan->yeAimer.currentWeapon->weaponCannonclass_instance) &&
							PTR_IS_VALID(spawnManager->playerCharacter->currentSoldierMan->yeAimer.currentWeapon->weaponCannonclass_instance->ordnancePtr))
						{
							printf("ptrs are valid, changing ordnance type to that of %s\n", text);
							spawnManager->playerCharacter->currentSoldierMan->yeAimer.currentWeapon->weaponCannonclass_instance->ordnancePtr = (void*)ordnancePtr;
						}
					}
				}
#pragma endregion
			}
			ImGui::End();
		}
#pragma endregion

		ImGui::EndFrame();
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	}

	toggleKeyDownLastFrame = toggleKeyDownThisFrame;
	if (shouldShutdownHax || GetAsyncKeyState(VK_INSERT))
	{
		ShutdownHax();
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
	AllocConsole();
	FILE* f = nullptr;
	freopen_s(&f, "CONOUT$", "w", stdout);
	printf(
		"===========================\n"
		"injected dll at 0x%zx\n"
		"===========================\n", (uintptr_t)module);

	HWND window = util::GetCurrentProcessWindow();

	if (IsWindowUnicode(window))
	{
		wchar_t buffer[128] = { 0 };
		GetWindowTextW(window, buffer, sizeof(buffer) / sizeof(*buffer));
		wprintf(L"window text: %s\n", buffer);
	}
	else
	{
		char buffer[128] = { 0 };
		GetWindowTextA(window, buffer, sizeof(buffer) / sizeof(*buffer));
		printf("window text: %s\n", buffer);
	}

	bool hooksSuccess = dx::CreateHooks(window) && di::CreateHooks();

	if (!hooksSuccess)
	{
		puts("failed to create hooks. try injecting again.");
		FreeConsole();
		FreeLibraryAndExitThread(module, 0);
	}
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