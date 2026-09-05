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
    if(!GetConfig().enabled) return true;
    SpawnDef const* s=FindSpawn(g); ChatHandler out(p->GetSession());
    if(!s){ out.SendSysMessage("Worldforged prototype spawn is not registered."); return true; }
    ClaimResult r=DeliverByMail(p,*s);
    if(r==ClaimResult::Delivered) out.SendSysMessage("Worldforged item sent by mail.");
    else if(r==ClaimResult::AlreadyDelivered) out.SendSysMessage("You have already claimed this Worldforged cache.");
    else out.SendSysMessage("Worldforged delivery did not commit; you may safely try again.");
    return true;
} };
class wf_commands : public CommandScript { public: wf_commands():CommandScript("wf_commands"){} ChatCommandTable GetCommands() const override {
    static ChatCommandTable sub={{"status",Status,SEC_PLAYER,Console::No},{"nearby",Nearby,SEC_GAMEMASTER,Console::No},{"claimstatus",ClaimStatus,SEC_PLAYER,Console::No},{"resetclaim",Reset,SEC_ADMINISTRATOR,Console::No}};
    static ChatCommandTable root={{"worldforged",sub}}; return root;
}
static bool Status(ChatHandler* h){ Player* p=h->GetSession()->GetPlayer(); uint32 pve=0,pvp=0,count=0; for(uint8 slot=EQUIPMENT_SLOT_START;slot<EQUIPMENT_SLOT_END;++slot) if(Item* i=p->GetItemByPos(INVENTORY_SLOT_BAG_0,slot)) if(auto const* d=FindItem(i->GetEntry())){pve+=d->pve;pvp+=d->pvp;++count;} h->PSendSysMessage("Worldforged items: {} | informational PvE Power: {} | PvP Power: {} | POWER EFFECTS DISABLED",count,pve,pvp); return true; }
static bool Nearby(ChatHandler* h){ Player* p=h->GetSession()->GetPlayer(); for(auto const& s:Spawns()) if(s.map==p->GetMapId()) h->PSendSysMessage("{} native={} item={} ({:.1f},{:.1f},{:.1f})",s.stableGuid,s.nativeGuid,s.item,s.x,s.y,s.z); return true; }
static bool ClaimStatus(ChatHandler* h){ h->SendSysMessage("Worldforged delivery is atomic mail delivery; POWER EFFECTS DISABLED."); return true; }
static bool Reset(ChatHandler* h,Optional<std::string> guid){ if(!guid || !FindSpawnByGuid(*guid)){h->SendSysMessage("Usage: .worldforged resetclaim <knownSpawnGuid>");return false;} bool ok=ResetClaim(h->GetSession()->GetPlayer(),*guid);h->SendSysMessage(ok?"Claim eligibility reset; existing items were not removed.":"Claim reset failed.");return ok; }
};
void AddWorldforgedScripts(){new wf_world();new wf_player();new wf_go();new wf_commands();}
void Addmod_worldforgedScripts(){AddWorldforgedScripts();}
