# RoF2 mercenary protocol

This reference describes the supplied May 10, 2013 `eqgame.exe`, SHA256 `2a8702ad9f722704f01355c0750be7d6f164a8b9c9128ba0cf286ea32b405b0e`. Addresses are preferred-image virtual addresses for that artifact. All integer wire fields are little-endian; offsets and lengths below are decimal unless prefixed `0x`. Application payload lengths exclude the two-byte opcode and transport framing.

**Core native acceptance passed on the deployed implementation, 2026-10-23.** Liaison/hire, all four roles, lifecycle, payments, assistance, switching, transfer and persistence were exercised in the matching original client. Mapped-only actions and unresolved fields below retain their narrower evidence status; a passing core workflow does not establish every packet semantic.

The outer client dispatcher sends the mercenary cases through `0x4c53a6`, manager getter `0x569b00`, and manager dispatcher `0x569920`. The 27 mapped application messages comprise the 13 existing mappings and 14 recovered mappings. Machine-readable details are in [rof2_opcodes.json](rof2_opcodes.json) and [wire-layouts.json](wire-layouts.json).

## Opcode map

`C→S` means client to server. A bidirectional opcode can have different payloads in its two directions. “Mapped only” means a name and layout were recorded without claiming an implemented gameplay path.

| Canonical name | Value | Native direction | Payload and endpoint |
| --- | --- | --- | --- |
| `OP_MercenaryDataUpdateRequest` | `0x7b89` | C→S | Empty owned-roster refresh; sender `0x5646f0`. |
| `OP_MercenaryDataUpdate` | `0x61a4` | S→C | Selected index, counted owned rows, one final capacity; consumer `0x5684a0`. |
| `OP_MercenaryDataRequest` | `0x11c1` | C→S | Four-byte liaison entity ID; sender `0x564770`. |
| `OP_MercenaryDataResponse` | `0x72ce` | S→C | Grade array and offer rows; consumer `0x568670`. |
| `OP_MercenaryHire` | `0x7169` | C→S / S→C | Request 16 bytes; response four-byte result. Sender `0x564dd0`, response consumer `0x5695b0`. |
| `OP_MercenaryDismiss` | `0x6e83` | C→S / S→C | Empty direct request and dismissal notification; send site `0x566e7c`, consumer `0x567c00`. The management Dismiss button was also observed using command `0x27f2`, command value 1. |
| `OP_MercenaryTimerRequest` | `0x31e4` | C→S | Empty; initialization sender `0x566740`. |
| `OP_MercenaryTimer` | `0x0763` | S→C | 20-byte status/timer; consumer `0x567ca0`. |
| `OP_MercenaryUnknown1` | `0x5d26` | S→C | First byte is preserved in native event 25; consumer `0x567df0`. Final UI meaning unresolved. |
| `OP_MercenaryCommand` | `0x27f2` | C→S / S→C | Eight-byte command/stance request; send site `0x6e5f79`. Incoming consumer `0x567ea0` reads its first word into event 16; a server reply schema beyond that read is unresolved. |
| `OP_MercenarySuspendRequest` | `0x4407` | C→S | One-byte action marker `0x30`; send site `0x5652a2`. |
| `OP_MercenarySuspendResponse` | `0x6f03` | S→C | Four-byte restore deadline; consumer `0x567500`. |
| `OP_MercenaryUnsuspendResponse` | `0x27a0` | S→C | Consumer `0x567590` reads no payload and sets normal state. The server retains a one-byte compatibility payload. |
| `OP_MercenaryAutoAssist` | `0x0b72` | C→S | One-byte Boolean; sender `0x5670a0`. |
| `OP_MercenaryAssist` | `0x7e6f` | C→S | One-byte action marker `0x30`; sender `0x564e90`. |
| `OP_MercenarySwitch` | `0x1b37` | C→S / S→C | Four-byte packed row index; 12-byte result. Sender `0x564a70`, result consumer `0x5680c0`. |
| `OP_MercenaryTransferRequest` | `0x5df8` | C→S / S→C | 28-byte owner-transfer request/invitation; sender `0x564f20`, invitation consumer `0x565320`. |
| `OP_MercenaryTransferResponse` | `0x4333` | C→S / S→C | 28-byte acceptance/result; callback `0x5682b0`, result consumer `0x565560`. |
| `OP_MercenaryUnshroud` | `0x69ca` | S→C | 120-byte upgrade prompt; consumer `0x5662f0`. Mapped only; the complete shrouding workflow is outside this implementation. |
| `OP_MercenaryUnshroudResponse` | `0x6e9f` | C→S | 120-byte prompt echo with choice; callback `0x5682b0`. Mapped only. |
| `OP_MercenaryDead` | `0x038f` | S→C | Four-byte restoration deadline, state 0 / Revive; consumer `0x567890`. |
| `OP_MercenaryRestricted` | `0x0e52` | S→C | No consumed payload, state 2 / prohibited zone; consumer `0x567920`. |
| `OP_MercenaryRestoreReady` | `0x2749` | S→C | No consumed payload, clears restoration deadline; consumer `0x5677e0`. |
| `OP_MercenaryUpkeep` | `0x5f88` | S→C | One-byte paid-with-alternate flag; consumer `0x567710`. |
| `OP_MercenaryCostUpdate` | `0x3cbc` | S→C | Four current cost words; consumer `0x567ff0`. |
| `OP_MercenaryCurrencyMessage` | `0x29ec` | S→C | Plural flag, currency ID, variable text prefix; consumer `0x565890`. |
| `OP_MercenaryTimerUpdate` | `0x2b4f` | S→C | Four-byte remaining upkeep milliseconds; consumer `0x567650`. |

## Shared variable row

The client row reader is `0x569cf0`; counted row arrays are read by `0x56bf90`. Wire rows have a 58-byte prefix, a NUL-terminated name, and `stance_count` pairs of `(u32 index, u32 stance_id)`. There is no per-row trailing capacity word.

| Offset | Type | Meaning / evidence boundary |
| --- | --- | --- |
| 0 | u32 | Template identifier, historically `MercID`. It is not the durable character-owned `mercs.MercID`. |
| 4 | u32 | Type/grade string identifier. |
| 8 | u32 | Subtype/description string identifier. |
| 12 | u32 | Gold purchase cost. |
| 16 | u32 | Gold upkeep cost. |
| 20 | u32 | Status/account requirement field; stored at native row `+0xcc`. Exact Live entitlement policy is not inferred. |
| 24 | u32 | Alternate purchase cost. |
| 28 | u32 | Alternate upkeep cost. |
| 32 | u32 | Alternate currency identifier; historical value 19 denotes Bayle Marks. Advertising a field does not implement alternate payment. |
| 36 | u8 | Unresolved field. |
| 37 | i32 | Remaining upkeep time; milliseconds in the established timer convention. Merchant rows conventionally use -1. |
| 41 | u32 | Native current command/state, stored at row `+0x28` and echoed as command word 0. Historical name `MerchantSlot` is misleading. Current owned sender emits 5 for active and 0 for suspended rows. |
| 45 | u32 | Selected stance **index**, stored at row `+0x2c` and echoed as command option. Historical name `MercUnk02` is misleading. |
| 49 | u32 | Stance count. |
| 53 | u32 | Unresolved field, native row `+0x34`. |
| 57 | u8 | Automatic-assist preference, 0 off / 1 on. Copied to the selected native wrapper and used by the management checkbox. |
| 58 | NUL string | Name; server limits content to 63 bytes and includes a terminator. |
| After name | repeated 8-byte pairs | Stance index then stance ID. Server validates at most 10 pairs, matching the bounded native row storage. |

The production wire helper `rof2_wire.h` limits rows to 100, grades to 10, stances per row to 10, and name content to 63 bytes. The 100-row encoder bound is a server serialization limit, not proof of a fixed native ownership entitlement. The server's owned-contract limit is separately clamped to its compiled three slots.

A row occupies `58 + name_bytes + 1 + 8 × stance_count` bytes. Internal fixed-size emulator structs are not wire layouts and must not be copied directly into packets.

## Offer and owned-roster envelopes

`0x72ce` is `u32 grade_count`, that many `u32 grade_id` values, `u32 offer_count`, then that many shared rows. Length is `8 + 4 × grade_count + sum(row_lengths)`.

`0x61a4` is `i32 selected_index`, `u32 owned_count`, that many shared rows, then **one** final `u32 available_capacity`. Length is `12 + sum(row_lengths)`. Native consumer `0x5684a0` stores the selected index at manager `+0x1fc`, parses the entire counted array, then stores capacity at `+0x1f8`. The old per-row `MercUnk05` convention was correct only for a single row; the encoder now emits capacity after all rows.

Empty ownership is `selected_index=-1`, `owned_count=0`, followed by capacity. A storage-slot hole never produces an empty wire row. The selected index and switch index address the packed vector in transmitted order. Native Switch UI `0x6e7039` obtains its list row data, subtracts one, and calls `0x564a70`, which checks the result against the current vector count.

Manager construction/reset (`0x569800` / `0x566620`) defaults capacity to 1 and selected index to -1. Owned vectors grow dynamically (`0x569c30`), with native row stride `0xd0`; no fixed native limit of three was established.

## Hire, commands, and timers

The 16-byte hire request is template ID at 0, initial advertised stance index at 4, liaison runtime entity ID at 8, and an opaque word at 12. Sender `0x564dd0` explicitly initializes only the low byte of the final word to zero; its upper three bytes are unspecified stack content. They are not reserved-zero bytes and must not be used as a payment selector. Sole caller `0x6ecdb0` proves word +4 is a stance index: it renders entries from manager +0x228 using dbstr stance category 24, stores each stance index as list row data, and passes that row data to the hire sender at `0x6ecdf9`. The four-byte server response is a result code interpreted by `0x5695b0`.

The native Dismiss confirmation uses dialog question ID 2 and a 10,000 ms timeout (`0x566e33`). Yes produces result flag 4; callback `0x56847a` accepts flags masked by 5 and sends the empty `0x6e83` request. A timed-out or cancelled confirmation does not send dismissal. The send path has no roster-command or selected-index gate.

A command request contains `u32 command` at 0 and `i32 stance_index` at 4. The sender reads command/state from the current row and the selected stance index through getter `0x564530`. The server handles dismissal value 1 before stance processing, validates any stance index against the actual advertised list, and preserves lifecycle state. Historical observations include commands 0, 1, 5, 20, and 36; observing a number does not prove every semantic branch.

The 20-byte timer payload is:

| Offset | Type | Meaning |
| --- | --- | --- |
| 0 | u8 | Has-current-mercenary flag consumed by the client. The following three bytes are not consumed as an entity ID by `0x567ca0`. RoF2 server output normalizes the enclosing word to 0 or 1. |
| 4 | u32 | Full configured upkeep period, milliseconds; not the current remaining countdown. |
| 8 | u32 | Restoration/unsuspend interval, milliseconds. |
| 12 | u32 | Lifecycle state; established normal 5 and suspended 1. Other manager states remain distinct from stance. |
| 16 | u32 | Restoration deadline in the existing server Unix-time convention; zero when absent. Exact cross-message clock semantics are not extended beyond the established paths. |

`0x6f03` supplies a four-byte restoration deadline and sets state 1. `0x27a0` sets state 5 and samples the client's clock without reading the supplied payload. Both ordinary suspend/unsuspend requests and manual assist requests carry the literal one-byte marker `0x30`; that byte is not a Boolean direction. Suspend direction follows authenticated server state. Auto-assist uses a separate Boolean `0x0b72`.

The owned-row byte at 57 authoritatively restores the automatic-assist checkbox. The server fills every owned row from the character-scoped persisted preference, including suspended rows and sparse storage slots; it does not substitute a constant enabled value. Reader `0x569e67` stores it at native row +0x38; selected-row copy `0x5644e0` places it at active wrapper +0x44. Getter `0x564610` reads that byte. Checkbox observer `0x6e6090` handles roster refresh event 19 and local-toggle event 21, then sets the check state. The roster consumer emits event 19 through `0x566f70`, so no invented server echo of the client-only `0x0b72` is required.

Call for Assist is proven by the binding chain: control path `mercenary/ManageWnd/CallForAssistButton` at `0x568ead` → registrar `0x56ce40` → factory `0x56b280` → control constructor `0x6e60e0` → callback `0x6e6100` → sender `0x564e90`. The Assist Mode checkbox uses `0x568e6e` → `0x56cdb0` → `0x56b220` → `0x6e6040` → callback `0x6e6060` → sender `0x5670a0`. Independent `/mercassist on/off` callers pass 1 and 0.

## Switching

Request `0x1b37` is a single `u32 packed_index`. Its response has an unresolved first word at 0 (server emits zero), `u32 delay` at 4, and `u32 result` at 8. Consumer `0x5680c0` does not read the first word.

| Result | Native meaning |
| --- | --- |
| 0 | Success; a positive delay sets suspended/restoration state using the native clock. |
| 1 | Owner is not in fast regeneration for switching. |
| 2 | Selected contract cannot be unsuspended; a positive delay is formatted. |
| 3 | Selected slot is unavailable. |
| 4 | Current mercenary is dead. |
| 5 | Cannot switch at this time. |

Selected row changes arrive through a subsequent roster. A bounded direct-call scan did not establish an automatic client unsuspend request when a switch deadline expires. The implemented server switch completes its own suspension/activation sequence rather than depend on that unproven callback.

## Ownership transfer

The 28-byte request/invitation/reply contains:

| Offset | Type | Meaning |
| --- | --- | --- |
| 0 | u32 | Original owner's runtime player entity ID. |
| 4 | u32 | Recipient's runtime player entity ID. |
| 8 | u32 | Gold upgrade fee. |
| 12 | u32 | Alternate-currency fee count. |
| 16 | u32 | Alternate-currency identifier. |
| 20 | u8 + 3 unresolved bytes | Alternate-cost mode flag used by the invitation formatter. |
| 24 | u32 | Acceptance/result. |

The initial `0x5df8` sender clears all 28 bytes then fills only owner/recipient entity IDs. Its UI binding invokes Set Owner and has explicit no-owned-mercenary, no-target, and self-target guards. The server enriches a delivered invitation. The confirmation callback `0x5682b0`, type 0, changes result to 0 for acceptance or 1 for decline and sends `0x4333`; all other bytes are echoed. A second invitation while one is pending produces result 4. Server acceptance uses its own stored offer, not echoed identity, cost, or template claims.

Result display table `0x565718`, consumed by `0x565560`: 0 transferred, 1 denied, 2 recipient must be grouped, 3 recipient capacity full, 4 recipient already has a pending transfer, 5 requirements not met, 6 insufficient membership, 7 both parties must be in fast regeneration, 8 cannot transfer. These messages establish presentation codes, not exact Live eligibility policy. This server implements its documented admission and fee policy and sends matching results.

The client's confirmation flag masks are 5 for accept and `0x0a` for decline. Those are local UI callback flags; they are not extra wire fields. Durable character IDs and durable MercIDs never come from these runtime entity-ID words.

## Additional lifecycle and cost notifications

The native Suspend/Revive button updater `0x6e63b0..0x6e65e3` independently identifies state 0 as Revive and state 2 as zone-restricted suspension. `0x038f` consumes a four-byte deadline and selects state 0. `0x0e52` selects state 2 and clears restoration timing; `0x2749` clears the restoration deadline and updates the UI. The latter two consumers read no payload. Their server payload constants are zero; do not serialize an empty C++ structure whose size would be one.

`0x5f88` consumes a one-byte paid-with-alternate flag: zero formats charged gold upkeep, nonzero formats charged alternate upkeep using the current row's count and currency ID. It sets state 5 and samples the native clock. Formatter `0x569f00` supplies the same messages used by Hire response branches 11 and 12. This notification says a payment was charged; it must not be used to imply a zero/waived payment. No outgoing Bayle-selection operation follows from it.

`0x3cbc` is exactly 16 bytes: gold purchase cost at 0, alternate purchase cost at 4, gold upkeep cost at 8, and alternate upkeep cost at 12. Setters operate on an active wrapper whose row begins at +0x0c, so wrapper offsets must not be mistaken for row offsets. The consumer updates native row +0x0c, +0x14, +0x10, and +0x18 respectively, then emits event 26.

`0x29ec` consumes a plural byte at 0, ignores three alignment bytes at 1–3, reads a currency ID at 4, and reads a NUL-terminated prefix beginning at 8. Resolver `0x565210` falls back to the current row currency when the supplied ID is zero. The displayed text is the prefix followed by the resolved currency name and a period. Complete application usage and a historical maximum prefix length remain unproven; any server producer must apply its own explicit text bound.

`0x2b4f` supplies one `u32 remaining_milliseconds`, updates remaining upkeep, and samples the native clock. None of these seven newly named notifications establishes a client rename, marketplace unlock, or currency preference request.

## Unshroud upgrade boundary

`0x69ca` and `0x6e9f` use the following 120-byte payload: `u32 gold_cost` at 0, `u32 alternate_cost` at 4, `u32 old_level` at 8, `u32 unshrouded_level` at 12, `u32 choice` at 16, and a 100-byte NUL-terminated message prefix at 20. Formatter `0x565740` displays the prefix, old/new levels, and costs. The native prompt explicitly concerns a mercenary lower than its owner's unshrouded form.

Consumer `0x5662f0` retains the 120-byte prompt. Confirmation callback type 1 uses choice 0 for Upgrade and 1 for Dismiss; cancel returns without a reply and retains shrouding. It echoes all 120 bytes on `0x6e9f` and then clears the pending buffer. No durable owner or mercenary identity is established in these consumed fields; a working server shroud implementation would need to bind its own pending operation. This checkout records the mappings/layouts but does not implement that complete general shrouding workflow.

## Unresolved messages and analysis limits

Manager `0x569920` also accepts unassigned incoming `0x20b9`, consumer `0x567a20`, which emits event 7 without an established payload read. Its final semantic name remains unresolved. The final observer semantics of mapped `0x5d26` / event 25 also remain unresolved. No rename, unlock, or payment-preference meaning is assigned to either case.

`0x4820` is an **outgoing candidate, unassigned**. Initialization `0x566740` sends exactly four bytes `08 00 00 00` after the empty timer request. Its direct setup caller is `0x49c7c6`. This does not prove Open Inventory, opening the owned window, a replacement mercenary request, or a required server reply. No canonical mapping was added merely to silence an unknown-opcode log.

For `0x5d26`, one REA pseudocode result omitted the payload parameter. Exact assembly at `0x567e08`/`0x567e0c` reads its first byte and places it at event `+0x1c`; clone `0x56a600` copies that byte. The raw-byte proof takes precedence over the omitted decompiler parameter. Final observer semantics remain unresolved.

## Evidence provenance

REA evidence records from the matching artifact:

| Subject | Evidence ID |
| --- | --- |
| Manager dispatcher | `ev_e40b77bd6c520f8682d59ae54b67a87bc8c9ebf8801c657393dbb26b980a3ea4` |
| Owned envelope | `ev_160826389af067e945b53d50fd209ee81fe21cf46370994d0569292c03b197ef` |
| Row reader | `ev_b8ba5bd30aa06a970138ab0b07d601a43a0564bcd1b9b8c050b4a945c5d7f836` |
| Counted array | `ev_863395a4345df11c9f4c82ee2695717535d63303b56ee8c27df493e16e386dc4` |
| Automatic assist | `ev_ab9bc1cd91d5fb23f9df60bb30adfee462f63ee6ce9289427f70c7db99475ebc` |
| Manual assist | `ev_b01abb8f0d8df293d5c04b5279cc47918bb9bed998a36fd06e2581c529d10352` |
| Switch result | `ev_beb5ade8fb502b71fa7b2d998eae58709e65fb8436321a5e209c387e231dc3f8` |
| Transfer invitation | `ev_1b619eacdf8cb6920d4ed259753390d4a897a8552ecb954daa0584a8c5c819dd` |
| Transfer result | `ev_29b6f41c15f8e82feac9b1b2454150af534d77fa879c45b1f1f8154dcb298bea` |
| Unshroud prompt | `ev_b1d4aa6633246c1f7b446f7f070f8b9fca13577e0cc33f683ab8f4f4c7687f87` |
| Constant-8 initialization | `ev_7cc2fbbc9b86e8334799f298b67a49773bd06eef0c74aab430294b610f4b1877` |

Matching artifact disassembly corroborates outgoing payload construction, UI factories/callbacks, local confirmation masks, variable row parsing, and the event-25 byte clone. A failed decompiler lookup at an interior callback address is not counted as successful REA evidence. Research harnesses, native client files, credentials, fixtures, raw logs, and test reports remain outside production source.

The status billing period and remaining countdown have distinct consumers. `0x0763 +4` stores manager `+0x208`; `0x2b4f` updates the active row countdown and native clock. RoF2 always receives the full configured period in status, with actual saved/live remaining milliseconds only in `0x2b4f`. The paid-message formatter reads the full period. Supplying a restored partial period here can reach the client's unsafe mixed-unit duration formatter.

The matching client's `0x564c40` formats into a 20-byte local buffer before a bounded destination copy. Every mixed hours/minutes/seconds string exceeds that buffer. For configured periods containing more than one nonzero time unit, the server therefore replaces paid `0x5f88` and raw Hire results 11 (coins), 12 (alternate), and 14 (waived) with bounded server chat. Exact timer fields and lifecycle updates are preserved. Single-unit periods such as the default 180000 ms continue using native notifications.

Before any paid/waived native duration notification, the server republishes current status and the full configured interval. This also covers a runtime rule change from a mixed-unit period to a single-unit period; the client cannot format an old unsafe interval. Missing selected ownership context uses server chat.

Static zone-policy reload republishes current state for RoF2 owners, including suspended contracts; it never automatically activates them. Ordinary cooldown-ready replay ends with Restricted when the zone still forbids mercenaries. Dead/Revive readiness requires both the durable restoration deadline and the character cooldown timer to expire.
