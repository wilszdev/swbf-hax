#pragma once
#include <cstdint>

struct SpawnManager
{
public:
	char pad_0000[16]; //0x0000
	void* ptrToSelf; //0x0010
	char pad_0014[4]; //0x0014
	struct Team* Teams[8]; //0x0018
	char pad_0038[4]; //0x0038
	struct Character* playerCharacter; //0x003C
	struct ent* LocalPlayerPtrPtr; //0x0040
	char pad_0044[108]; //0x0044
}; //Size: 0x00B0

struct ent
{
public:
	char pad_0000[4]; //0x0000
	uint8_t nKillsTeamOne; //0x0004
	uint8_t nKillsTeamTwo; //0x0005
	char pad_0006[38]; //0x0006
	uint8_t nDeathsTeamOne; //0x002C
	uint8_t nDeathsTeamTwo; //0x002D
	uint8_t isInAMenuOtherThanPauseScreen; //0x002E
	char pad_002F[1]; //0x002F
	struct WeaponCannonClass* N00000553; //0x0030
	char pad_0034[76]; //0x0034
	float timeAsFirstClassSelected; //0x0080
	float timeAsShockTrooper; //0x0084
	float timeAsDarkTrooper; //0x0088
	float timeAsStormTrooper; //0x008C
	float timeAsScoutTrooper; //0x0090
	float N00000748; //0x0094
	char pad_0098[16]; //0x0098
	float timeAsRebelMarksman; //0x00A8
	float timeAsWookieSmuggler; //0x00AC
	float timeAsRebelSoldier; //0x00B0
	float timeAsRebelVanguard; //0x00B4
	float timeAsRebelPilot; //0x00B8
	char pad_00BC[100]; //0x00BC
	struct EntitySoldierClass* currSoldierClass; //0x0120
	char pad_0124[130]; //0x0124
	uint8_t commandPostsCapturedTeamOne; //0x01A6
	uint8_t commandPostsCapturedTeamTwo; //0x01A7
	uint32_t healthProportion; //0x01A8
	char pad_01AC[2076]; //0x01AC
}; //Size: 0x09C8

struct Character
{
public:
	char pad_0000[16]; //0x0000
	void* selfPtr; //0x0010
	char pad_0014[4]; //0x0014
	struct CommandPost* commandPostInstance; //0x0018
	wchar_t characterName[16]; //0x001C
	char pad_003C[104]; //0x003C
	struct Team* currentTeam; //0x00A4
	char pad_00A8[4]; //0x00A8
	struct EntitySoldierClass* currentClass; //0x00AC
	struct CommandPost* spawnCommandPost; //0x00B0
	struct EntitySoldier* currentSoldierMan; //0x00B4
	char pad_00B8[48]; //0x00B8
}; //Size: 0x00E8

struct Team
{
public:
	char pad_0000[8]; //0x0000
	uint32_t indexInTeamsArray; //0x0008
	wchar_t* teamName; //0x000C
	char pad_0010[4]; //0x0010
	uint32_t remainingUnits; //0x0014
	uint32_t startingUnits; //0x0018
	char pad_001C[12]; //0x001C
	uint32_t _startingUnits; //0x0028
	uint32_t numCharactersTotal; //0x002C
	uint32_t numCharactersAlive; //0x0030
	struct character_array* charactersOnThisTeam; //0x0034
	char pad_0038[12]; //0x0038
}; //Size: 0x0044

struct EntitySoldierClass
{
public:
	char pad_0000[16]; //0x0000
	char pad_0010[4]; //0x0010
	char pad_0014[12]; //0x0014
	char N00001478[16]; //0x0020
	wchar_t* className; //0x0030
	char pad_0034[24]; //0x0034
	char pad_004C[4]; //0x004C
	char pad_0050[68]; //0x0050
}; //Size: 0x0094

struct EntitySoldier
{
public:
	char pad_0000[4]; //0x0000
	struct EntitySoldierClass* currentSoldierClass; //0x0004
	char pad_0008[4]; //0x0008
	float curHealth; //0x000C
	float maxHealth; //0x0010
	char pad_0014[48]; //0x0014
}; //Size: 0x0044

struct Aimer
{
public:
	char pad_0000[8]; //0x0000
	void* previous_plus_4; //0x0008
	void* next_plus_4; //0x000C
	void* self; //0x0010
	char pad_0014[4]; //0x0014
	void* selfPtr; //0x0018
	char pad_001C[8]; //0x001C
	struct classWithPlayerDataYay* classWithPlayerDataYay; //0x0024
	char pad_0028[256]; //0x0028
	struct WeaponCannon* currentGun; //0x0128
	char pad_012C[20]; //0x012C
	struct WeaponCannon* gun1; //0x0140
	struct WeaponCannon* gun2; //0x0144
	struct WeaponGrenade* nade1; //0x0148
	struct WeaponGrenade* nade2; //0x014C
	char pad_0150[52]; //0x0150
}; //Size: 0x0184

// ptr to this at battlefront.exe + 0x01D95D24
struct TheStartOfSomeRandomDataThing
{
public:
	char pad_0000[36]; //0x0000
	struct Aimer localPlayerAimer; //0x0024
	char pad_01A8[28]; //0x01A8
}; //Size: 0x01C4

struct classWithPlayerDataYay
{
public:
	char pad_0000[48]; //0x0000
	float X; //0x0030
	float Y; //0x0034
	float Z; //0x0038
	char pad_003C[16]; //0x003C
	void* selfPtr; //0x004C
	char pad_0050[4]; //0x0050
	struct EntitySoldier* currentEntitySoldierInstance; //0x0054
	char pad_0058[4]; //0x0058
}; //Size: 0x005C

struct WeaponCannon
{
public:
	char pad_0000[96]; //0x0000
	struct WeaponCannonClass* weaponCannonclass_instance; //0x0060
	struct WeaponCannonClass* weaponCannonclass_instance_2; //0x0064 // appears to always be the same addr as previous
	char pad_0068[4];//struct EntitySoldier* entitySoldier instance - wrong type, sometimes it is other things; //0x0068
	struct Aimer* TheAimerBelongingToThisWeapon; //0x006C
	char pad_0070[28]; //0x0070
	float proportionOfAmmunitionLeftInMag; //0x008C
	float weaponHeat; //0x0090
	char pad_0094[56]; //0x0094
	struct RedModel* Geometry; //0x00CC
	struct RedModel* GeometryHighRes; //0x00D0
}; //Size: 0x00D4