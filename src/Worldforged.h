#pragma once
#include "Define.h"
#include "DatabaseEnvFwd.h"
#include <string>
#include <vector>
class Player;
class GameObject;
namespace Worldforged {
struct Config { bool enabled=false; bool prototypeMode=true; bool personalLoot=true; bool debug=true; uint32 gameObjectEntry=900001; static constexpr bool powerEffects=false; };
struct ItemDef { uint32 item; uint32 family; uint32 pve; uint32 pvp; uint32 originalProc; };
struct SpawnDef { std::string stableGuid; uint64 nativeGuid; uint32 item; uint32 family; uint32 map; uint32 area; float x,y,z,o; };
enum class ClaimResult { Delivered, AlreadyDelivered, Failed };
Config& GetConfig(); void LoadConfig();
std::vector<ItemDef> const& Items(); std::vector<SpawnDef> const& Spawns();
ItemDef const* FindItem(uint32); SpawnDef const* FindSpawn(GameObject const*); SpawnDef const* FindSpawnByGuid(std::string const&);
bool HasDelivered(uint32, std::string const&); ClaimResult DeliverByMail(Player*, SpawnDef const&);
bool ResetClaim(Player*, std::string const&); void CleanupCharacterClaims(CharacterDatabaseTransaction, uint32);
}
