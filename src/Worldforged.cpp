#include "Worldforged.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "GameObject.h"
#include "Item.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "WorldSession.h"

namespace Worldforged {
static Config cfg;
Config& GetConfig() { return cfg; }
void LoadConfig() {
    cfg.enabled = sConfigMgr->GetOption<bool>("Worldforged.Enable", false);
    cfg.prototypeMode = sConfigMgr->GetOption<bool>("Worldforged.PrototypeMode", true);
    cfg.personalLoot = sConfigMgr->GetOption<bool>("Worldforged.PersonalLoot.Enable", true);
    cfg.debug = sConfigMgr->GetOption<bool>("Worldforged.Debug", true);
    cfg.gameObjectEntry = sConfigMgr->GetOption<uint32>("Worldforged.GameObject.Entry", 900001);
    LOG_INFO("module.worldforged", "WFTRACE CONFIG enabled={} prototypeMode={} personalLoot={} goEntry={} debug={}", cfg.enabled, cfg.prototypeMode, cfg.personalLoot, cfg.gameObjectEntry, cfg.debug);
}
static std::vector<ItemDef> items = {
    {450705,450705,20,9,0}, {450523,450523,45,12,303023},
    {450711,450711,38,21,0}, {521079,521079,38,21,0}, {450696,450696,45,12,0}
};
static std::vector<SpawnDef> spawns = {
    {"1-5-0-450705-0.3920-0.5390",0,450705,450705,1,14,-89.1741823f,-4044.71723f,64.2366409f,0.0f},
    {"2-31-0-450523-0.6936-0.7919",0,450523,450523,0,62,-9775.97317f,-868.482484f,39.5238571f,0.0f},
    {"1-5-0-450711-0.5670-0.5291",0,450711,450711,1,372,-57.4491834f,-4959.98342f,21.421875f,0.0f},
    {"1-5-0-521079-0.6014-0.8381",0,521079,521079,1,368,-1363.46164f,-5139.75841f,1.65644681f,0.0f},
    {"1-5-0-450696-0.4896-0.4868",0,450696,450696,1,816,92.0108114f,-4551.78845f,55.7291489f,0.0f}
};
std::vector<ItemDef> const& Items(){ return items; }
std::vector<SpawnDef> const& Spawns(){ return spawns; }
ItemDef const* FindItem(uint32 id){ for (auto const& x:items) if(x.item==id) return &x; return nullptr; }
SpawnDef const* FindSpawnByGuid(std::string const& id){ for(auto const& x:spawns) if(x.stableGuid==id) return &x; return nullptr; }
SpawnDef const* FindSpawn(GameObject const* go){
    if(!go || go->GetEntry()!=cfg.gameObjectEntry) return nullptr;
    LOG_INFO("module.worldforged", "WFTRACE FINDSPAWN_SCAN go={} map={} xyz={},{},{}", go->GetGUID().GetCounter(), go->GetMapId(), go->GetPositionX(), go->GetPositionY(), go->GetPositionZ());
    for(auto const& x:spawns) if(x.map==go->GetMapId()) { float d=go->GetDistance2d(x.x,x.y); LOG_INFO("module.worldforged", "WFTRACE FINDSPAWN_CANDIDATE stable={} distance={}", x.stableGuid, d); if(d<1.0f) return &x; }
    return nullptr;
}
static std::string Escape(std::string value){ CharacterDatabase.EscapeString(value); return value; }
bool HasDelivered(uint32 character, std::string const& spawn){
    return bool(CharacterDatabase.Query("SELECT 1 FROM worldforged_character_loot WHERE character_guid={} AND worldforged_spawn_guid='{}' AND state=1", character, Escape(spawn)));
}
ClaimResult DeliverDirectBag(Player* player, SpawnDef const& spawn){
    LOG_INFO("module.worldforged", "WFTRACE DELIVERY_ENTER character={} spawn={} item={}", player ? player->GetGUID().GetCounter() : 0, spawn.stableGuid, spawn.item);
    if(!player){ LOG_INFO("module.worldforged", "WFTRACE DELIVERY_FAIL player_null"); return ClaimResult::Failed; }
    if(!cfg.enabled){ LOG_INFO("module.worldforged", "WFTRACE DELIVERY_FAIL disabled"); return ClaimResult::Failed; }
    if(!cfg.personalLoot){ LOG_INFO("module.worldforged", "WFTRACE DELIVERY_FAIL personal_loot_disabled"); return ClaimResult::Failed; }
    if(!FindSpawnByGuid(spawn.stableGuid)){ LOG_INFO("module.worldforged", "WFTRACE DELIVERY_FAIL unknown_spawn"); return ClaimResult::Failed; }
    if(!FindItem(spawn.item)){ LOG_INFO("module.worldforged", "WFTRACE DELIVERY_FAIL unknown_item"); return ClaimResult::Failed; }
    uint32 character=player->GetGUID().GetCounter();
    LOG_INFO("module.worldforged", "WFTRACE CLAIM_PRECHECK begin character={} spawn={}", character, spawn.stableGuid);
    bool already=HasDelivered(character,spawn.stableGuid); LOG_INFO("module.worldforged", "WFTRACE CLAIM_PRECHECK already_delivered={}", already);
    if(already) { LOG_INFO("module.worldforged", "WFTRACE BAG_ALREADY_CLAIMED character={} spawn={}", character, spawn.stableGuid); return ClaimResult::AlreadyDelivered; }
    ItemPosCountVec destination;
    InventoryResult inventoryResult = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, destination, spawn.item, 1);
    LOG_INFO("module.worldforged", "WFTRACE BAG_PREFLIGHT character={} item={} result={} destinations={}", character, spawn.item, inventoryResult, destination.size());
    if (inventoryResult != EQUIP_ERR_OK)
    {
        LOG_INFO("module.worldforged", "WFTRACE BAG_FULL character={} item={} result={}", character, spawn.item, inventoryResult);
        player->SendEquipError(inventoryResult, nullptr, nullptr, spawn.item);
        return ClaimResult::Failed;
    }
    LOG_INFO("module.worldforged", "WFTRACE BAG_DEST character={} item={} pos={}", character, spawn.item, destination.back().pos);
    LOG_INFO("module.worldforged", "WFTRACE ITEM_CREATE begin item={}", spawn.item);
    Item* item=Item::CreateItem(spawn.item,1,player);
    if(!item){ LOG_INFO("module.worldforged", "WFTRACE ITEM_CREATE fail"); return ClaimResult::Failed; }
    uint32 itemGuid=item->GetGUID().GetCounter();
    LOG_INFO("module.worldforged", "WFTRACE ITEM_CREATE success itemGuid={}", itemGuid);
    CharacterDatabaseTransaction trans=CharacterDatabase.BeginTransaction();
    LOG_INFO("module.worldforged", "WFTRACE BAG_TX_BEGIN character={} itemGuid={} spawn={}", character, itemGuid, spawn.stableGuid);
    item->AppendNewItemPersistence(trans);
    LOG_INFO("module.worldforged", "WFTRACE BAG_TX_ITEM itemGuid={}", itemGuid);
    player->AppendDetachedInventoryPlacement(trans, item, destination);
    LOG_INFO("module.worldforged", "WFTRACE BAG_TX_INVENTORY itemGuid={} pos={}", itemGuid, destination.back().pos);
    // Plain INSERT is deliberate: a duplicate key must abort and roll back the complete delivery transaction.
    trans->Append("INSERT INTO worldforged_character_loot (character_guid,worldforged_spawn_guid,item_entry,state,delivered_item_guid,delivered_at) VALUES ({},'{}',{},1,{},NOW())",character,Escape(spawn.stableGuid),spawn.item,itemGuid);
    LOG_INFO("module.worldforged", "WFTRACE BAG_TX_CLAIM character={} spawn={} itemGuid={}", character, spawn.stableGuid, itemGuid);
    LOG_INFO("module.worldforged", "WFTRACE BAG_COMMIT_BEGIN character={} itemGuid={}", character, itemGuid);
    int result = CharacterDatabase.DirectCommitTransactionWithResult(trans);
    LOG_INFO("module.worldforged", "WFTRACE BAG_COMMIT_RESULT character={} itemGuid={} result={}", character, itemGuid, result);
    if (result != 0)
    {
        item->SetState(ITEM_REMOVED);
        LOG_ERROR("module.worldforged", "WFTRACE BAG_FAIL transaction_result={} character={} spawn={}", result, character, spawn.stableGuid);
        return ClaimResult::Failed;
    }
    LOG_INFO("module.worldforged", "WFTRACE BAG_ATTACH_BEGIN character={} itemGuid={} pos={}", character, itemGuid, destination.back().pos);
    if (!player->AttachPersistedItem(destination, item, true))
    {
        item->SetState(ITEM_REMOVED);
        LOG_ERROR("module.worldforged", "WFTRACE BAG_ATTACH_RECOVERY_NEEDED character={} itemGuid={} spawn={}", character, itemGuid, spawn.stableGuid);
        return ClaimResult::Failed;
    }
    LOG_INFO("module.worldforged", "WFTRACE BAG_ATTACH_SUCCESS character={} itemGuid={}", character, itemGuid);
    return ClaimResult::Delivered;
}
bool ResetClaim(Player* gm, std::string const& spawnGuid){
    if(!gm || !FindSpawnByGuid(spawnGuid)) return false;
    uint32 character=gm->GetGUID().GetCounter();
    QueryResult q=CharacterDatabase.Query("SELECT delivered_item_guid FROM worldforged_character_loot WHERE character_guid={} AND worldforged_spawn_guid='{}'",character,Escape(spawnGuid));
    uint32 oldItem=q ? q->Fetch()[0].Get<uint32>() : 0;
    CharacterDatabase.DirectExecute("DELETE FROM worldforged_character_loot WHERE character_guid={} AND worldforged_spawn_guid='{}'",character,Escape(spawnGuid));
    LOG_WARN("module.worldforged","GM character {} reset Worldforged claim {} (delivered item GUID {}). Existing item/mail was not removed.",character,spawnGuid,oldItem);
    return true;
}
void CleanupCharacterClaims(CharacterDatabaseTransaction trans,uint32 character){ trans->Append("DELETE FROM worldforged_character_loot WHERE character_guid={}",character); }
}
