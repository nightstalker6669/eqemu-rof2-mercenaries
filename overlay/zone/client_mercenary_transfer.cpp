#include "client_mercenary_transfer.h"
#include "client.h"
#include "merc.h"
#include "groups.h"
#include "zone.h"
#include "inventory_session_owner.h"
#include "../common/eqemu_config.h"
#include "../common/rulesys.h"
#include "../common/housing/coin_pending_guard.h"
#include "../common/housing/house_inventory_transition.h"
#include <algorithm>
#include <array>
#include <chrono>
#include <map>
#include <memory>
#include <limits>
#include <stdexcept>
#include <string>

extern Zone* zone;
namespace MercenaryTransfer {
namespace {
using Ticket = Housing::InventorySessions::ZoneReservation;
using Lease = Housing::InventorySessions::Lease;
constexpr uint64_t LifetimeSeconds = 60;
struct Offer {
 uint32_t source{}, target{}, source_spawn{}, target_spawn{}, merc{}, merc_template{}, gold{};
 uint8_t source_slot{}, source_level{}, target_level{};
 uint32_t stance{};
 uint64_t expires{};
 std::weak_ptr<Ticket> source_ticket, target_ticket;
};
// Zone packet dispatch is single threaded. Offers are not recoverable across a
// process restart or a new authenticated inventory session, deliberately.
std::map<uint32_t, Offer> offers;
uint64_t Now() { return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now().time_since_epoch()).count(); }
uint32_t Read(const EQApplicationPacket& p, size_t offset) {
 uint32_t value=0; for(size_t i=0;i<4;++i)value|=uint32_t(p.pBuffer[offset+i])<<(8*i); return value;
}
void Put(EQApplicationPacket& p,size_t offset,uint32_t value) { for(size_t i=0;i<4;++i)p.pBuffer[offset+i]=uint8_t(value>>(8*i)); }
void Send(Client& c,EmuOpcode opcode,const Offer& offer,uint32_t result) {
 EQApplicationPacket p(opcode,28);
 Put(p,0,offer.source_spawn);Put(p,4,offer.target_spawn);Put(p,8,offer.gold);
 // Gold only. Alternate currency is never inferred from client-supplied fields.
 Put(p,12,0);Put(p,16,0);Put(p,20,0);Put(p,24,result);c.QueuePacket(&p);
}
void Result(Client& source,Client* target,const Offer& offer,uint32_t result) {
 Send(source,OP_MercenaryTransferResponse,offer,result);
 if(target && target!=&source)Send(*target,OP_MercenaryTransferResponse,offer,result);
}
bool Ready(Client& c) {
 return zone && RuleB(Mercs,AllowMercs) && zone->AllowMercs() && c.CharacterID() && c.Connected() && c.ClientDataLoaded() &&
  c.ClientVersion()==EQ::versions::ClientVersion::RoF2 && !c.IsDead() && !c.IsUnconscious();
}
bool Session(Client& c,const std::weak_ptr<Ticket>& expected) {
 const auto ticket=expected.lock(); return ticket && ticket==c.InventorySessionReleaseTicket() && ticket->HeldFor(c) && c.HasInventorySessionReservation();
}
uint32_t Price(uint32_t merc_template,Client& source,Client& target) {
 if(!RuleB(Mercs,ChargeMercPurchaseCost))return 0;
 // The EQ Team mercenary FAQ specifies another full hire fee when ownership
 // moves to a higher-level player, evaluated at the recipient's level.
 return target.GetLevel()>source.GetLevel()?Merc::CalcPurchaseCost(merc_template,target.GetLevel()):0;
}
uint32_t Validate(Client& source,Client& target,const Offer& offer) {
 if(!Ready(source)||!Ready(target)||source.CharacterID()==target.CharacterID() ||
  source.GetID()!=offer.source_spawn || target.GetID()!=offer.target_spawn ||
  !Session(source,offer.source_ticket)||!Session(target,offer.target_ticket))return 8;
 if(source.GetTarget()!=&target)return 8;
 auto* group=source.GetGroup();
 if(!group || group!=target.GetGroup() || !group->IsGroupMember(&source) || !group->IsGroupMember(&target))return 2;
 if(source.HasRaid() || target.HasRaid() || source.GetAggroCount() || target.GetAggroCount() ||
  !source.CanFastRegen() || !target.CanFastRegen())return 7;
 if(source.GetMercSlot()!=offer.source_slot || source.GetMercInfo().mercid!=offer.merc ||
  source.GetMercInfo().MercTemplateID!=offer.merc_template || source.GetMercInfo().Stance!=offer.stance ||
  source.GetLevel()!=offer.source_level || target.GetLevel()!=offer.target_level || Price(offer.merc_template,source,target)!=offer.gold)return 8;
 auto* definition=zone->GetMercTemplate(offer.merc_template);
 if(!definition || static_cast<unsigned>(target.ClientVersion())<definition->ClientVersion)return 5;
 if(target.GetNumberOfMercenaries()>=target.GetMercenaryCapacity())return 3;
 if(auto* merc=source.GetMerc()) {
  if(merc->GetMercenaryID()!=offer.merc || merc->GetMercenaryCharacterID()!=source.CharacterID() || merc->IsEngaged() || merc->GetHP()<=0)return 8;
 }
 return 0;
}
auto Query(Database& db,const std::string& sql) {
 if(db.GetStatus()!=DBcore::Connected)throw std::runtime_error("mercenary transfer disconnected");
 auto r=db.QueryDatabase(sql,false);if(!r.Success())throw std::runtime_error("mercenary transfer query failed");return r;
}
uint64_t Number(const char* text) { if(!text)throw std::runtime_error("mercenary transfer null field");return std::stoull(text); }
std::string Hex(const uint8_t* data,size_t length) {
 const char* digits="0123456789ABCDEF";std::string out;for(size_t i=0;i<length;++i){out+=digits[data[i]>>4];out+=digits[data[i]&15];}return out;
}
std::string Hex(const std::string& value) { return Hex(reinterpret_cast<const uint8_t*>(value.data()),value.size()); }
void LockLease(Database& db,const Lease& lease) {
 const auto actor=std::to_string(lease.actor.character_id);
 auto c=Query(db,"SELECT id FROM character_data WHERE id="+actor+" AND account_id="+std::to_string(lease.actor.account_id)+" AND deleted_at IS NULL FOR UPDATE");
 if(c.RowCount()!=1)throw std::runtime_error("mercenary transfer actor changed");
 auto rows=Query(db,"SELECT account_id,epoch,HEX(token),HEX(process_key),HEX(client_key),zone_id,instance_id,active FROM housing_inventory_sessions WHERE character_id="+actor+" FOR UPDATE");
 if(rows.RowCount()!=1)throw std::runtime_error("mercenary transfer session absent");auto row=rows.begin();
 if(!lease.active || Number(row[0])!=lease.actor.account_id || Number(row[1])!=lease.epoch || !row[2] || std::string(row[2])!=Hex(lease.token.data(),lease.token.size()) ||
  !row[3] || std::string(row[3])!=Hex(lease.owner.process_key) || !row[4] || std::string(row[4])!=Hex(lease.owner.client_key) ||
  Number(row[5])!=lease.owner.zone_id || Number(row[6])!=lease.owner.instance_id || Number(row[7])!=1)throw std::runtime_error("mercenary transfer stale session");
}
struct Coins { uint32_t copper{},silver{},gold{},platinum{}; };
Coins Carried(Client& c) {return {c.GetCopper(),c.GetSilver(),c.GetGold(),c.GetPlatinum()};}
bool Same(const Coins& a,const Coins& b) {return a.copper==b.copper&&a.silver==b.silver&&a.gold==b.gold&&a.platinum==b.platinum;}
uint64_t Total(const Coins& c) {return uint64_t(c.copper)+10ULL*c.silver+100ULL*c.gold+1000ULL*c.platinum;}
Coins Debit(const Coins& c,uint64_t fee) {
 if(Total(c)<fee)throw std::runtime_error("mercenary transfer insufficient funds");
 const auto value=Total(c)-fee;if(value/1000>std::numeric_limits<int32_t>::max())throw std::runtime_error("mercenary transfer coin range");return {uint32_t(value%10),uint32_t(value/10%10),uint32_t(value/100%10),uint32_t(value/1000)};
}
void Currency(Database& db,Client& target,const Coins& before,const Coins& after,uint64_t upkeep,uint32_t bayle) {
 const auto id=std::to_string(target.CharacterID());
 auto leases=Query(db,"SELECT account_id,pending_key FROM housing_live_item_leases WHERE character_id="+id+" FOR UPDATE");
 if(leases.RowCount()>1 || (leases.RowCount() && (!leases.begin()[0] || Number(leases.begin()[0])!=target.AccountID() || (leases.begin()[1]&&*leases.begin()[1]))))throw std::runtime_error("mercenary transfer pending inventory");
 if(Housing::CoinPending::Inspect(db,target.CharacterID(),true)!=Housing::CoinPending::State::Clear ||
  Housing::HouseInventory::InspectPending(db,target.CharacterID(),true)!=Housing::HouseInventory::PendingState::Clear)throw std::runtime_error("mercenary transfer pending coins");
 auto stored=Query(db,"SELECT copper,silver,gold,platinum FROM character_currency WHERE id="+id+" FOR UPDATE");
 if(stored.RowCount()!=1)throw std::runtime_error("mercenary transfer currency absent");auto row=stored.begin();
 if(Number(row[0])!=before.copper || Number(row[1])!=before.silver || Number(row[2])!=before.gold || Number(row[3])!=before.platinum || !Same(Carried(target),before))throw std::runtime_error("mercenary transfer stale coins");
 if(upkeep) {
  auto alternate=Query(db,"SELECT amount FROM character_alt_currency WHERE char_id="+id+" AND currency_id=19 FOR UPDATE");
  if(alternate.RowCount()>1 || (alternate.RowCount()?Number(alternate.begin()[0]):0)!=bayle || target.GetAlternateCurrencyValue(19)!=bayle)throw std::runtime_error("mercenary transfer stale alternate currency");
  if(!bayle && Total(after)<upkeep)throw std::runtime_error("mercenary transfer unaffordable upkeep");
 }
 if(Same(before,after))return;
 auto changed=Query(db,"UPDATE character_currency SET copper="+std::to_string(after.copper)+",silver="+std::to_string(after.silver)+",gold="+std::to_string(after.gold)+",platinum="+std::to_string(after.platinum)+" WHERE id="+id+
  " AND copper="+std::to_string(before.copper)+" AND silver="+std::to_string(before.silver)+" AND gold="+std::to_string(before.gold)+" AND platinum="+std::to_string(before.platinum));
 if(changed.RowsAffected()!=1)throw std::runtime_error("mercenary transfer currency conflict");
}
void Refresh(Client& c,Merc* active,uint8_t preferred_slot) {
 if(active) { c.SetMercSlot(preferred_slot);c.SetMercID(active->GetID()); }
 else {
  c.SetMercID(0);c.GetMercTimer()->Disable();
  if(preferred_slot<MAXMERCS && c.GetMercInfo(preferred_slot).mercid)c.SetMercSlot(preferred_slot);
  else for(uint8_t slot=0;slot<MAXMERCS;++slot)if(c.GetMercInfo(slot).mercid){c.SetMercSlot(slot);break;}
 }
 c.PersistMercenarySelection();c.SendMercPersonalInfo();c.SendMercTimer(active);
}
void Uncertain(Client& source,Client& target) {
 source.RejectMercenaryPersistence();target.RejectMercenaryPersistence();
}
void Commit(Client& source,Client& target,const Offer& offer) {
 auto* moving=source.GetMerc();auto* receiving_active=target.GetMerc();const auto receiving_slot=target.GetMercSlot();
 // Merc::Save has its own atomic session fence on the zone connection. Finish
 // it before starting a second transaction that locks either admission lease.
 if(moving && !moving->Save())throw std::runtime_error("mercenary transfer snapshot failed");
 const auto source_ticket=offer.source_ticket.lock(),target_ticket=offer.target_ticket.lock();
 if(!source_ticket||!target_ticket)throw std::runtime_error("mercenary transfer session lost");
 const auto source_lease=source_ticket->SnapshotFor(source),target_lease=target_ticket->SnapshotFor(target);
 if(!source_lease||!target_lease)throw std::runtime_error("mercenary transfer lease lost");
 const auto& wallet=target.GetPP();
 if(wallet.copper<0 || wallet.silver<0 || wallet.gold<0 || wallet.platinum<0)throw std::runtime_error("mercenary transfer invalid carried coins");
 const auto before=Carried(target);const auto fee=uint64_t(offer.gold)*100;
 const auto after=fee?Debit(before,fee):before;
 const auto upkeep=RuleB(Mercs,ChargeMercUpkeepCost)?uint64_t(Merc::CalcUpkeepCost(offer.merc_template,target.GetLevel()))*100:0;
 const auto bayle=target.GetAlternateCurrencyValue(19);
 const auto* cfg=EQEmuConfig::get();if(!cfg)throw std::runtime_error("mercenary transfer database unavailable");
 Database db;if(!db.Connect(cfg->DatabaseHost,cfg->DatabaseUsername,cfg->DatabasePassword,cfg->DatabaseDB,cfg->DatabasePort,"mercenary-transfer"))throw std::runtime_error("mercenary transfer connect failed");
 bool transaction=false,committing=false;uint8_t destination_slot=MAXMERCS;
 try {
  auto engines=Query(db,"SELECT COUNT(*) FROM information_schema.tables WHERE table_schema=DATABASE() AND engine='InnoDB' AND table_name IN ('mercs','character_data','housing_inventory_sessions','character_currency','housing_live_item_leases','character_alt_currency')");
  if(engines.RowCount()!=1||Number(engines.begin()[0])!=6)throw std::runtime_error("mercenary transfer schema unavailable");
  Query(db,"START TRANSACTION");transaction=true;
  auto connection=Query(db,"SELECT CONNECTION_ID(),@@in_transaction");if(connection.RowCount()!=1||!Number(connection.begin()[1]))throw std::runtime_error("mercenary transfer transaction absent");
  const auto connection_id=Number(connection.begin()[0]);
  if(source.CharacterID()<target.CharacterID()){LockLease(db,*source_lease);LockLease(db,*target_lease);}else{LockLease(db,*target_lease);LockLease(db,*source_lease);}
  // Lock the complete owner ranges, including empty receiver gaps. The owner/slot
  // unique key is an additional durable last line of defense against races.
  auto roster=Query(db,"SELECT MercID,OwnerCharacterID,Slot,TemplateID,StanceID FROM mercs WHERE OwnerCharacterID IN ("+std::to_string(source.CharacterID())+","+std::to_string(target.CharacterID())+") ORDER BY OwnerCharacterID,Slot FOR UPDATE");
  std::array<bool,MAXMERCS> occupied{};bool found=false;unsigned count=0,source_count=0;
  for(auto row:roster) {
   const auto stored_slot=Number(row[2]);
   if(stored_slot>=MAXMERCS)throw std::runtime_error("mercenary transfer corrupt slot");
   auto& owner=Number(row[1])==target.CharacterID()?target:source;
   if(owner.GetMercInfo(stored_slot).mercid!=Number(row[0]))throw std::runtime_error("mercenary transfer stale roster");
   if(&owner==&target) {if(occupied[stored_slot])throw std::runtime_error("mercenary transfer corrupt roster");occupied[stored_slot]=true;++count;}else ++source_count;
   if(Number(row[0])==offer.merc && Number(row[1])==source.CharacterID() && Number(row[2])==offer.source_slot && Number(row[3])==offer.merc_template && Number(row[4])==offer.stance)found=true;
  }
  const auto limit=target.GetMercenaryCapacity();if(limit>MAXMERCS)throw std::runtime_error("mercenary transfer capacity range");uint8_t slot=0;while(slot<limit&&occupied[slot])++slot;
  if(source_count!=source.GetNumberOfMercenaries() || count!=target.GetNumberOfMercenaries() || limit>MAXMERCS || !found || count>=limit || slot>=limit || Validate(source,target,offer))throw std::runtime_error("mercenary transfer changed admission");
  destination_slot=slot;
  Currency(db,target,before,after,upkeep,bayle);
  auto changed=Query(db,"UPDATE mercs SET OwnerCharacterID="+std::to_string(target.CharacterID())+",Slot="+std::to_string(slot)+",IsSuspended=1,SuspendedTime=0 WHERE MercID="+std::to_string(offer.merc)+" AND OwnerCharacterID="+std::to_string(source.CharacterID())+" AND Slot="+std::to_string(offer.source_slot)+" AND TemplateID="+std::to_string(offer.merc_template)+" AND StanceID="+std::to_string(offer.stance));
  if(changed.RowsAffected()!=1)throw std::runtime_error("mercenary transfer ownership conflict");
  auto identity=Query(db,"SELECT CONNECTION_ID(),@@in_transaction");
  if(identity.RowCount()!=1||Number(identity.begin()[0])!=connection_id||!Number(identity.begin()[1]))throw std::runtime_error("mercenary transfer connection changed");
  committing=true;auto result=db.QueryDatabase(std::string("COMMIT"),false);
  if(!result.Success()){Uncertain(source,target);return;}transaction=false;
 } catch(...) {
  if(transaction&&db.GetStatus()==DBcore::Connected)db.QueryDatabase(std::string("ROLLBACK"),false);
  if(committing){Uncertain(source,target);return;}throw;
 }
 // No SQL retries or user callbacks occurred between validation and commit.
 if(fee) {auto& pp=target.GetPP();pp.copper=after.copper;pp.silver=after.silver;pp.gold=after.gold;pp.platinum=after.platinum;target.SendMoneyUpdate();}
 auto transferred=source.GetMercInfo(offer.source_slot);
 transferred.slot=destination_slot;transferred.IsSuspended=true;transferred.SuspendedTime=0;transferred.State=MERC_STATE_SUSPENDED;
 if(moving){moving->Depop();moving->SetOwnerID(0);}
 source.SetMercID(0);source.GetMercInfo(offer.source_slot)=MercInfo{};
 target.GetMercInfo(destination_slot)=transferred;
 // Apply the exact committed delta in memory; no second read can fail after an
 // already durable transfer, and no selected receiver entity is replaced.
 Refresh(source,nullptr,MAXMERCS);Refresh(target,receiving_active,receiving_slot);
 Result(source,&target,offer,0);
}
void Prune() {const auto now=Now();for(auto i=offers.begin();i!=offers.end();)if(i->second.expires<=now || i->second.source_ticket.expired() || i->second.target_ticket.expired())i=offers.erase(i);else ++i;}
}
void Request(Client& source,const EQApplicationPacket& packet) {
 if(packet.size!=28 || !Ready(source))return;Prune();
 auto* candidate=source.GetTarget();if(!candidate || !candidate->IsClient())return;auto& target=*candidate->CastToClient();
 Offer offer;offer.source=source.CharacterID();offer.target=target.CharacterID();offer.source_spawn=source.GetID();offer.target_spawn=target.GetID();
 offer.merc=source.GetMercInfo().mercid;offer.merc_template=source.GetMercInfo().MercTemplateID;offer.source_slot=source.GetMercSlot();
 offer.stance=source.GetMercInfo().Stance;offer.source_level=source.GetLevel();offer.target_level=target.GetLevel();offer.gold=Price(offer.merc_template,source,target);
 offer.source_ticket=source.InventorySessionReleaseTicket();offer.target_ticket=target.InventorySessionReleaseTicket();offer.expires=Now()+LifetimeSeconds;
 // A native request's runtime IDs must identify the actual authenticated target,
 // but cannot select the actor or any different owned mercenary.
 if(Read(packet,0)!=offer.source_spawn || Read(packet,4)!=offer.target_spawn || !offer.merc)return;
 const auto error=Validate(source,target,offer);if(error){Result(source,&target,offer,error);return;}
 for(const auto& existing:offers)if(existing.second.target==offer.target || existing.second.source==offer.source){Result(source,nullptr,offer,4);return;}
 offers.emplace(offer.target,offer);Send(target,OP_MercenaryTransferRequest,offer,0);
}
void Respond(Client& target,const EQApplicationPacket& packet) {
 if(packet.size!=28 || !Ready(target))return;Prune();
 auto entry=offers.find(target.CharacterID());if(entry==offers.end())return;
 const auto offer=entry->second;
 if(!Session(target,offer.target_ticket) || Read(packet,0)!=offer.source_spawn || Read(packet,4)!=offer.target_spawn)return;
 // Consume once, before any write. A replay cannot transfer another mercenary.
 offers.erase(entry);auto* source=entity_list.GetClientByCharID(offer.source);if(!source)return;
 const auto answer=Read(packet,24);if(answer!=0){Result(*source,&target,offer,answer==1?1:8);return;}
 const auto error=Validate(*source,target,offer);if(error){Result(*source,&target,offer,error);return;}
 try{Commit(*source,target,offer);}catch(...){Result(*source,&target,offer,8);}
}
}
