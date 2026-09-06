#include "Worldforged.h"
#include "Chat.h"
#include "ChatCommand.h"
#include "CommandScript.h"
#include "GameObject.h"
#include "GameObjectScript.h"
#include "Item.h"
#include "Player.h"
#include "PlayerScript.h"
#include "ScriptMgr.h"
#include "WorldScript.h"
using namespace Acore::ChatCommands;
using namespace Worldforged;
class wf_world : public WorldScript { public: wf_world():WorldScript("wf_world",{WORLDHOOK_ON_AFTER_CONFIG_LOAD}){} void OnAfterConfigLoad(bool) override { LoadConfig(); } };
class wf_player : public PlayerScript { public: wf_player():PlayerScript("wf_player",{PLAYERHOOK_ON_DELETE_FROM_DB}){} void OnPlayerDeleteFromDB(CharacterDatabaseTransaction trans,uint32 guid) override { CleanupCharacterClaims(trans,guid); } };
class wf_go : public GameObjectScript { public: wf_go():GameObjectScript("worldforged_gameobject"){} bool OnGossipHello(Player* p,GameObject* g) override {
    LOG_INFO("module.worldforged", "WFTRACE GOSSIP_ENTER player={} go={} entry={} map={} goxyz={},{},{} playerxyz={},{},{}", p ? p->GetGUID().GetCounter() : 0, g ? g->GetGUID().GetCounter() : 0, g ? g->GetEntry() : 0, g ? g->GetMapId() : 0, g ? g->GetPositionX() : 0.0f, g ? g->GetPositionY() : 0.0f, g ? g->GetPositionZ() : 0.0f, p ? p->GetPositionX() : 0.0f, p ? p->GetPositionY() : 0.0f, p ? p->GetPositionZ() : 0.0f);
    if(!GetConfig().enabled){ LOG_INFO("module.worldforged", "WFTRACE GOSSIP_DISABLED"); return true; }
    SpawnDef const* s=FindSpawn(g); ChatHandler out(p->GetSession());
    if(!s){ LOG_INFO("module.worldforged", "WFTRACE FINDSPAWN_FAIL"); out.SendSysMessage("Worldforged prototype spawn is not registered."); return true; }
    LOG_INFO("module.worldforged", "WFTRACE FINDSPAWN_OK stable={} item={} map={} distance={}", s->stableGuid, s->item, s->map, g->GetDistance2d(s->x,s->y));
    LOG_INFO("module.worldforged", "WFTRACE DELIVERY_CALL");
    ClaimResult r=DeliverDirectBag(p,*s);
    if(r==ClaimResult::Queued){ LOG_INFO("module.worldforged", "WFTRACE GOSSIP_RESULT queued"); out.SendSysMessage("Worldforged delivery queued."); }
    else if(r==ClaimResult::Delivered){ LOG_INFO("module.worldforged", "WFTRACE GOSSIP_RESULT delivered"); out.SendSysMessage("Worldforged item added to your bags."); }
    else if(r==ClaimResult::AlreadyDelivered){ LOG_INFO("module.worldforged", "WFTRACE GOSSIP_RESULT already_delivered"); out.SendSysMessage("You have already claimed this Worldforged cache."); }
    else { LOG_INFO("module.worldforged", "WFTRACE GOSSIP_RESULT failed"); out.SendSysMessage("Worldforged delivery did not commit; you may safely try again."); }
    return true;
} };
class wf_commands : public CommandScript { public: wf_commands():CommandScript("wf_commands"){} ChatCommandTable GetCommands() const override {
    static ChatCommandTable sub={{"status",Status,SEC_PLAYER,Console::No},{"nearby",Nearby,SEC_GAMEMASTER,Console::No},{"claimstatus",ClaimStatus,SEC_PLAYER,Console::No},{"resetclaim",Reset,SEC_ADMINISTRATOR,Console::No}};
    static ChatCommandTable root={{"worldforged",sub}}; return root;
}
static bool Status(ChatHandler* h){ Player* p=h->GetSession()->GetPlayer(); uint32 pve=0,pvp=0,count=0; for(uint8 slot=EQUIPMENT_SLOT_START;slot<EQUIPMENT_SLOT_END;++slot) if(Item* i=p->GetItemByPos(INVENTORY_SLOT_BAG_0,slot)) if(auto const* d=FindItem(i->GetEntry())){pve+=d->pve;pvp+=d->pvp;++count;} h->PSendSysMessage("Worldforged items: {} | informational PvE Power: {} | PvP Power: {} | POWER EFFECTS DISABLED",count,pve,pvp); return true; }
static bool Nearby(ChatHandler* h){ Player* p=h->GetSession()->GetPlayer(); for(auto const& s:Spawns()) if(s.map==p->GetMapId()) h->PSendSysMessage("{} native={} item={} ({:.1f},{:.1f},{:.1f})",s.stableGuid,s.nativeGuid,s.item,s.x,s.y,s.z); return true; }
static bool ClaimStatus(ChatHandler* h){ h->SendSysMessage("Worldforged delivery is atomic direct-bag delivery; POWER EFFECTS DISABLED."); return true; }
static bool Reset(ChatHandler* h,Optional<std::string> guid){ if(!guid || !FindSpawnByGuid(*guid)){h->SendSysMessage("Usage: .worldforged resetclaim <knownSpawnGuid>");return false;} bool ok=ResetClaim(h->GetSession()->GetPlayer(),*guid);h->SendSysMessage(ok?"Claim eligibility reset; existing items were not removed.":"Claim reset failed.");return ok; }
};
void AddWorldforgedScripts(){new wf_world();new wf_player();new wf_go();new wf_commands();}
void Addmod_worldforgedScripts(){AddWorldforgedScripts();}
