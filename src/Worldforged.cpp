#include "Worldforged.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "GameObject.h"
#include "Item.h"
#include "Log.h"
#include "Mail.h"
#include "ObjectMgr.h"
#include "Player.h"

namespace Worldforged {
static Config cfg;
Config& GetConfig() { return cfg; }
void LoadConfig() {
    cfg.enabled = sConfigMgr->GetOption<bool>("Worldforged.Enable", false);
    cfg.prototypeMode = sConfigMgr->GetOption<bool>("Worldforged.PrototypeMode", true);
    cfg.personalLoot = sConfigMgr->GetOption<bool>("Worldforged.PersonalLoot.Enable", true);
    cfg.debug = sConfigMgr->GetOption<bool>("Worldforged.Debug", true);
    cfg.gameObjectEntry = sConfigMgr->GetOption<uint32>("Worldforged.GameObject.Entry", 900001);
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
    for(auto const& x:spawns) if(x.map==go->GetMapId() && go->GetDistance2d(x.x,x.y)<1.0f) return &x;
    return nullptr;
}
static std::string Escape(std::string value){ CharacterDatabase.EscapeString(value); return value; }
bool HasDelivered(uint32 character, std::string const& spawn){
    return bool(CharacterDatabase.Query("SELECT 1 FROM worldforged_character_loot WHERE character_guid={} AND worldforged_spawn_guid='{}' AND state=1", character, Escape(spawn)));
}
ClaimResult DeliverByMail(Player* player, SpawnDef const& spawn){
    if(!player || !cfg.enabled || !cfg.personalLoot || !FindSpawnByGuid(spawn.stableGuid) || !FindItem(spawn.item)) return ClaimResult::Failed;
    uint32 character=player->GetGUID().GetCounter();
    if(HasDelivered(character,spawn.stableGuid)) return ClaimResult::AlreadyDelivered;
    Item* item=Item::CreateItem(spawn.item,1,player);
    if(!item) return ClaimResult::Failed;
    uint32 itemGuid=item->GetGUID().GetCounter();
    CharacterDatabaseTransaction trans=CharacterDatabase.BeginTransaction();
    item->SaveToDB(trans);
    MailDraft draft("Worldforged Discovery","A Worldforged item recovered from your discovery is enclosed.");
    draft.AddItem(item);
    draft.SendMailTo(trans,MailReceiver(character),MailSender(MAIL_CREATURE,34337));
    // Plain INSERT is deliberate: a duplicate key must abort and roll back the complete delivery transaction.
    trans->Append("INSERT INTO worldforged_character_loot (character_guid,worldforged_spawn_guid,item_entry,state,delivered_item_guid,delivered_at) VALUES ({},'{}',{},1,{},NOW())",character,Escape(spawn.stableGuid),spawn.item,itemGuid);
    CharacterDatabase.DirectCommitTransaction(trans);
    return HasDelivered(character,spawn.stableGuid) ? ClaimResult::Delivered : ClaimResult::Failed;
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
