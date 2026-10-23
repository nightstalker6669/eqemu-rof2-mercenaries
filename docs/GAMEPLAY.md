# RoF2 mercenaries

This implementation extends the existing EQEmu mercenary system for the matching May 10, 2013 Rain of Fear client. It covers liaison offers, hiring, owned contracts, native management, stances, suspension, upkeep, automatic and manual assistance, roster switching, and ownership transfer. Mercenary rows, buffs, carried coin payments, and stored Bayle Mark upkeep payments are persisted with authenticated session checks and explicit transaction outcomes.

**Native acceptance: core May 10, 2013 RoF2 cases passed on the deployed build (2026-10-23).** Original-client tests covered liaison offers and initial stances; all four combat roles, healer resurrection and level-100 spell fallback; paid hiring, Bayle-to-coin fallback and Passive upkeep waiver; manual/automatic assistance and the healer Puller override; suspension, death/revival deadlines and zone restrictions; three owned contracts, sparse slots, capacity rejection and replacement in a six-member group; declined and accepted priced ownership transfer; and zone, relog and server-restart persistence. Tests also confirmed the full recurring upkeep period and safe notifications across mixed-unit interval changes. This verifies the implemented core behavior; marketplace slot/rename purchases, later-client progression systems and exact historical Live balance are outside that claim.

Protocol details are in [PROTOCOL.md](../protocol/STRUCTS.md), with machine-readable [opcode](../protocol/opcodes.json) and [wire-layout](../protocol/wire-layouts.json) references. These distinguish established binary behavior, emulator policy, and unresolved fields.

## Integration prerequisites

This code targets the current combined EQEmu checkout, including its housing inventory-session infrastructure. It is not a standalone patch for an unmodified upstream server.

Mercenary payment and ownership transactions reuse the shared authenticated inventory writer lease: character/account, epoch, token, process, client, zone, and instance must identify the live session. Paid paths also refuse pending custody, coin, and house transitions, preventing a delayed housing operation or stale session from overwriting a mercenary payment. These checks use the existing housing session and pending-state schemas and the zone reservation APIs. The same dependencies protect ordinary mercenary saves and dismissals through their atomic finalizers. Install the matching shared infrastructure and schema before enabling this implementation; copying only the mercenary files will leave missing types, tables, and safety contracts.

Character-scoped data buckets store `eqemu.mercenary.selected_slot` and the automatic-assist preference. World/client expansion visibility, zone permission, and the active server ruleset remain separate controls. Do not enable unrelated expansion content to expose mercenaries.

## Content and activation

The pinned optional content bootstrap is `utils/sql/merc_tables_bootstrap.sql`, SHA256 `18106ea9cb70bd933a2e3ac57dac015c5c140b3c95a219e604bb7cb2bc04efd1`. It defines these 17 tables:

`merc_armorinfo`, `merc_buffs`, `merc_inventory`, `merc_merchant_entries`, `merc_merchant_template_entries`, `merc_merchant_templates`, `merc_name_types`, `merc_npc_types`, `merc_spell_list_entries`, `merc_spell_lists`, `merc_stance_entries`, `merc_stats`, `merc_subtypes`, `merc_templates`, `merc_types`, and `merc_weaponinfo`.

The bootstrap contains destructive `DROP TABLE` statements, including owned-character tables. Back up existing mercenary rows and buffs first, and import only missing schema/content when upgrading an established server. The normal upstream enable command is `world mercs:enable`; review its download source before using it for a pinned installation. The current checkout's local SQL provides a reproducible content source.

After the standard catalogue and shared infrastructure exist, apply owned-contract migrations 001/003 to the character database and zone-policy migration 004 to the content database when those are configured separately:

1. Apply `utils/sql/mercenary/001_owned_roster.sql`. It adds a unique `(OwnerCharacterID, Slot)` index. Existing duplicate slots cause the migration to fail rather than discard contracts; review and migrate duplicates explicitly.
2. Apply `utils/sql/mercenary/003_passive_upkeep.sql`. It adds the bounded `PassiveUpkeepElapsedMS` accumulator without removing owned rows. It accumulates only while active in Passive, freezes across suspension/zoning/logoff, and resets on a stance change.
3. Apply `utils/sql/mercenary/004_zone_permissions.sql`. The resolved `zone` row (including version fallback) now controls `allow_mercs`; 1 allows and 0 forbids mercenaries. Missing policy schema/read failures disable mercenaries for that zone. The default preserves existing allowed content. Operators must explicitly choose restricted rows; this migration does not infer Live raid restrictions. Reload static zone data or restart zones after a policy change.
4. Review `utils/sql/mercenary/002_enable_rof2.sql` and set its ruleset variable to the ruleset actually used by the server. For split databases, run its `SET` and `rule_values` insert in the character database, and its `npc_types`/`merc_merchant_entries` update in the content database; do not execute the entire file against either database alone. This activation explicitly enables mercenaries, opts into three owned contracts, enables the high-level fallback, and makes appropriately titled, catalogue-linked NPCs class 71.
5. Rebuild and restart the affected world and zone processes so opcode mappings, rules, mercenary templates, stances, and spells are loaded together. A rule reload alone does not populate an already-running zone's missing catalogue maps.
6. Check that a liaison's NPC **type ID** joins to `merc_merchant_entries` and its templates. Native shop requests subsequently identify the liaison's **runtime entity ID**. An arbitrary class 71 NPC without a catalogue relationship will not provide offers.

The standard content snapshot contains 553 templates, 2,380 stat rows across 28 NPC types at levels 1–85, and 437 spell-list entries. It is content supplied by the optional bootstrap, not a reconstruction of every historical Live mercenary.

## Rules and policies

| Setting | Behavior |
| --- | --- |
| `Mercs:AllowMercs` | Default false. Enables the server mercenary paths. |
| `Mercs:MaxMercenaries` | Default 1, clamped to compiled capacity 3. The supplied activation SQL explicitly chooses 3. One contract may be active at a time. |
| `Mercs:ChargeMercPurchaseCost` | Controls purchase charging and advertised purchase costs. Default false. Also controls the transfer upgrade-fee policy below. |
| `Mercs:ChargeMercUpkeepCost` | Independently controls upkeep charging and advertised upkeep costs. Default false. |
| `Mercs:UpkeepIntervalMS` | Current source default 180,000 ms. Native wire intervals use milliseconds; do not assume a fixed Live 15-minute period. |
| `Mercs:SuspendIntervalMS` / `Mercs:SuspendIntervalS` | Current source defaults 10,000 ms / 10 s. Keep related cooldown settings coherent. |
| `Mercs:ReviveIntervalS` | Default 300 seconds. Death saves zero HP, suspension and its restoration deadline together; the native Revive state survives relogging. |
| `Mercs:AllowMercSuspendInCombat` | Controls ordinary suspension. Ownership transfers still require both players out of combat and in fast regeneration. |
| `Mercs:ScaleAboveContentLevel` | Default true. Allows the explicit level 86–100 fallback when installed exact-level content is absent above a template's content ceiling. |
| Other mercenary rules | Existing aggro, puller radius, resurrection radius, scale rate, and haste rules continue to apply. |

Three contracts are an emulator capacity policy. No marketplace purchase, paid account entitlement, or external cash-service transaction is inferred from that setting. Native Buy Slot commerce and premium rename commerce are not implemented here. Account-specific Live unlock requirements and prices are not claimed reproduced.

Ownership transfers use a server-computed gold fee when purchase charging is enabled: `purchase_cost(template, recipient_level)` when the recipient is higher level, and zero otherwise. The result is in gold and is converted to copper for the transaction. This follows the historical EQ-team FAQ describing another full hire fee for a higher-level recipient. The configured purchase calculator and prices remain emulator content, rather than a recovered Live price table. The offered fee, owner levels, template, current selected contract, and sessions are bound to the pending offer. Client-echoed fees never authorize a debit. The locked transfer also verifies the recipient can afford the next upkeep after that fee, using remaining coins or a matching stored currency 19 balance. It does not deduct that future upkeep until activation. There is no recovered client payment-selector opcode. For positive charged upkeep, this server uses one stored Bayle Mark (alternate currency 19) when available, then falls back to carried coins. This automatic preference is server policy; the recovered hire stance word is never treated as a payment selector. Hiring and transfer fees remain coin charges. A zero or Passive-waived periodic upkeep interval consumes neither currency. Explicit unsuspend/revive still charges one activation upkeep when charging is enabled, including a contract with an earned Passive waiver. The accumulated unchanged Passive time is retained, so subsequent periodic bills remain waived until the stance changes. This activation-versus-periodic distinction is server policy.

## Owned contracts and gameplay

All valid owned rows are sent in storage-slot order. The native selected index and switch request are **packed row indices**, so storage slots 0 and 2 become native rows 0 and 1. The durable selection remains a storage-slot value in the character bucket. Missing or stale bucket selections fall back to an available owned row. Only the selected contract may spawn; switching saves and suspends the old one before activating the new one. A failed activation retains both owned records and leaves the old contract safely suspended.

The initial hire request and later stance selection use indices into the same advertised stance list and ordering as the native roster. It validates signed indices before changing the mercenary and persists the resulting stance. Stance IDs and lifecycle state are separate values. Passive interrupts automatic combat/casting and clears combat acquisition while preserving following. Automatic-assist preference is persistent and is sent in byte 57 of every owned roster row, so selection changes and relogging restore the native checkbox from server state. Manual Call for Assist checks the mercenary's state and can be used with automatic assist disabled. Combat roles discover legal nearby NPCs already fighting group members or their pets and exclude mezzed targets. Healers receive a three-second window to heal a nearby group puller outside the normal puller radius; they do not require an enemy target.

Merchant hiring checks liaison class, range, and the actual offered template list. Existing contracts must be suspended before hiring another. Raid, zone, group, combat, cooldown, and capacity restrictions continue through the relevant spawn/hire paths. Contract names, appearances, stance, resources, buffs, and remaining upkeep time are preserved across the supported save/load paths. Active zone-in resumes the saved countdown without an extra explicit-unsuspend payment.

A transfer requires the actual targeted player in the same live group and zone, both players alive and in fast regeneration, the sender's selected owned contract, receiver capacity, and unchanged authenticated sessions. A delivered offer expires after 60 seconds and can be consumed once. Acceptance rechecks the entire admission and updates the existing mercenary's owner/slot in one transaction. Its buff rows keep the same durable MercID. The transferred contract arrives suspended; the receiver's existing active mercenary remains active. An upkeep affordability check is admission only; transferring a suspended contract never secretly charges another upkeep interval. Replays, changed owners, changed slots/templates/stances/levels, stale sessions, and pending paid-inventory transitions are rejected.

## Persistence and failure contracts

`zone/mercenary_transaction.h` defines `SaveResult::{Saved, Failed, CommitUncertain}` and a required guarded `Payment` admission object, even for a zero-cost save. It contains exact carried coin values before/after, an optional exact alternate-currency counter before/after, and a same-connection admission guard. `ZoneDatabase::SaveMercenaryAtomic` commits the owned row, complete buff snapshot, and optional wallet or stored alternate-currency change together. Nested transactions are refused. The caller supplies the live writer lease and pending-transition checks inside that transaction; connection identity and `@@in_transaction` are checked around the guard and before commit.

Initial group membership is inserted with the generated MercID in the same hire transaction; provisional ID-zero membership is never written or removed. The owner’s exact persisted group membership is locked and authenticated, and the group table must use InnoDB. Fresh and restored spawn names match their durable contract names so the native group window can bind its HP gauges by exact name.

The initial mercenary ID and runtime wallet are published only after an acknowledged commit. Persistence or guard failure and uncertain commit require an authoritative reload; poisoned sessions suppress subsequent normal profile/currency saves. A failed or uncertain first hire must not delete a contract that may already have committed. Suspend, unsuspend, upkeep, and dismissal must honor these outcomes rather than report success after an ignored database failure.

Transfers use a dedicated non-retrying connection. The moving mercenary's normal atomic save finishes **before** that connection locks either inventory admission lease, avoiding a cross-connection deadlock. The transfer then locks both actors/sessions and the owner rosters, validates capacity and exact ownership, compares the wallet even for a zero fee, checks pending transitions and next-upkeep affordability, debits any fee, and changes owner/slot. After commit it applies the exact runtime delta and removes the former owner's zone entity. A lost commit acknowledgment fences both clients for reload; it is never retried as a new transfer.

## Level 86–100 fallback

When exact owner-level stats are missing **above the entire installed content ceiling** for the matching NPC type/race, the highest available lower-level row can be cloned up to level 100. Missing interior content rows remain errors. Let `r = owner_level / content_level`:

- Positive HP/current HP, mana, minimum damage, and maximum damage become `floor(base × r²)`.
- Positive AC, ATK, attributes, resists, and HP/mana regeneration become `floor(base × r)`.
- Values saturate at their actual field maximum. Zero and negative values remain unchanged.
- Level becomes the owner level. Attack delay/count, appearance, and spell/proc definitions remain content-driven; the ordinary `Mercs:ScaleRate` then applies.
- Template inventory may retain the highest lower-level subtype equipment above its content ceiling.

This formula is explicit emulator policy. It is not historical Live balance data. Owner level changes also refresh actual inherited skill caps, spell lists, innate procs, and attack timers.

## Source integration map

| Area | Production files and purpose |
| --- | --- |
| Native serialization | `common/mercenary/rof2_wire.h`, `common/patches/rof2.cpp`, shared/patch packet structures, opcode list, and `utils/patches/patch_RoF2.conf`: bounded variable rows, one roster-capacity trailer, recovered mappings. |
| Client commands and presentation | `zone/client_packet.cpp`, `zone/client_packet.h`, `zone/client.cpp`, `zone/client.h`: authorized hiring, stance validation, all-owned roster, persistent selection, assist preference, manual targeting, switching, and finalization gates. |
| Ownership transfer | `zone/client_mercenary_transfer.cpp/.h`: delivered pending invitations, admission checks, atomic owner/slot/fee changes, and post-commit runtime reconciliation. |
| Mercenary gameplay | `zone/merc.cpp`, `zone/merc.h`, `zone/mercenary_runtime.h`: lifecycle, timers, AI/Passive/assist behavior, group scans, stat/skill/spell refresh, and template content lookups. |
| Persistence | Mercenary sections of `zone/zonedb.cpp/.h` and `zone/mercenary_transaction.h`: owned rows, bounded nondestructive buff restoration, atomic saves/dismissal/payment outcomes. |
| High-level fallback | `zone/mercenary_scaling.h`: explicit, bounded level 86–100 scaling policy. |
| Configuration and schema | `common/ruletypes.h`, `utils/sql/mercenary/001_owned_roster.sql`, `002_enable_rof2.sql`, `003_passive_upkeep.sql`, `004_zone_permissions.sql`, `zone/zone.cpp`, and zone CMake integration. Standard content remains in the existing optional bootstrap. |

## Feature boundary and validation

The matching May 2013 client predates the Call of the Forsaken mercenary AA and player-equipped gear systems announced on [October 1, 2013](https://www.everquest.com/news/call-of-the-forsaken-mercenary-aa-equipment) for the October 8 release. This implementation does not invent later AA experience, AA purchases, or player-equippable mercenary gear messages. Bootstrap template equipment is a separate existing NPC-content mechanism.

The native unshroud upgrade prompt and reply are decoded and mapped for documentation. The emulator's complete player-shrouding workflow is not implemented here, so a decoded prompt alone does not establish an operational end-to-end shrouding feature. Unknown manager messages and constant-8 initialization request `0x4820` remain explicitly unresolved. Buy Slot, premium rename, external cash-service commerce, a client Bayle payment selector, exact Live confidence/stance tuning, and exact Live fee/stat curves are not claimed complete.

Healer resurrection searches eligible player corpses within the configured radius, including authenticated group roster members who zoned away. It skips already resurrected or ineligible corpses and selects the nearest eligible target. Mercenary death and AI startup use compiler-checked overrides of the current NPC lifecycle signatures.

The historical [EQ-team mercenary FAQ](https://www.everquest.com/news/imported-eq-enus-51873) also describes group-enemy awareness for manual tank assistance, a three-second healer override for pullers, and waived upkeep after one unchanged Passive billing period. The implementation includes those paths and persists bounded active Passive elapsed time. Their original-client combat, healing, and billing behavior passed native acceptance; this remains distinct from static button decoding. [Bayle Marks](https://www.everquest.com/news/imported-eq-enus-51874) were already a mercenary-payment feature in 2010. Existing alternate currency 19 balances can support server-selected upkeep payments, while marketplace purchases and conversion or redemption of physical bag items remain separate features. Automatic stored-Bayle-first preference is the explicit server policy described above.

Production validation includes bounded wire serialization, actual handler/roster extraction under ASan/UBSan, buff and scaling boundaries, and isolated real-database atomic save/payment/transfer faults. Tests include row/buff rollback, stale wallet/lease rejection, concurrent writers, process crashes, killed connections, and lost commit acknowledgments. These establish their stated code and persistence boundaries. Separate original-client acceptance passed for gameplay, combat/healing, UI behavior, zoning/relog/restart persistence, and guarded disposable-account cleanup. Research tests, fixtures, credentials, logs, client assets, and analysis reports are excluded from production documentation and code exports.

When an installed class/proficiency spell list ends below the mercenary's level, `Mercs:ScaleAboveContentLevel` also retains the repertoire at that list's highest valid installed level through owner level 100. For example, the installed wizard lists end at level 89, so a level 100 wizard uses their level 89 spells. This preserves proficiency, stance and rank selection and refreshes innate procs normally. It does not fill interior level gaps, borrow another proficiency's spells, invent higher-level spells, or claim Live-equivalent spell balance. Disable the rule to require exact installed level eligibility.

RoF2 status advertises the full upkeep period separately from the exact remaining countdown. A restored partial period never becomes the next billing period. The matching client has an unsafe mixed-unit duration formatter; custom periods containing multiple nonzero hour/minute/second units use bounded server chat for payment/waiver notifications while retaining exact timers. This avoids modifying the original client.
