# Architecture and persistence

The integration patch updates canonical opcode registration, bounded RoF2 encoders, client handlers/presentation, group membership publication, mercenary AI/lifecycle, resolved zone policy and database persistence. New sources are under `overlay/`; the source map in [GAMEPLAY.md](GAMEPLAY.md) lists each responsibility.

Native variable rows use explicit little-endian serialization, bounded names/counts and exactly one roster capacity trailer. Persistent storage slots are translated to packed native row indices. Selected slot and AutoAssist use character-scoped nested data bucket state.

The `Mercenary::Payment` admission object is required even for zero-cost saves. It carries authenticated writer identity (character/account, epoch/token, process/client, zone/instance), wallet snapshots and pending-operation constraints. Housing's shared infrastructure provides these guards; this prevents stale or delayed inventory work from overwriting a mercenary payment. Charging is optional; admission validation is not.

Initial hire commits generated mercenary identity, row/buffs, actual initial group MercID and any coin/Bayle debit atomically before publishing wallet/roster/HP changes. The zero mercenary identity is never used to remove unrelated group rows. Normal saves and dismissals use the same outcome discipline: Saved, Failed or CommitUncertain. Lost acknowledgments are not blindly retried. Poisoned sessions suppress later wallet/profile writes and require authoritative reload.

Transfers are consensual, bounded pending offers tied to authenticated source/receiver sessions, revision and exact contract. A dedicated non-retrying connection is used only after the normal source save completes, avoiding cross-connection lease deadlock. It locks both sessions/rosters, validates capacity, level/templates/stances, next-upkeep affordability and exact wallet, then moves ownership and charges once. After commit it reconciles runtime state, preserves the receiver's active contract and delivers the arrival suspended. An uncertain outcome fences both actors for reload.

Persistent suspension deadlines and player timers jointly prevent early revival. Death uses compiler-checked NPC virtual signatures and retains HP0. Native timer state separates full recurring period from exact remaining countdown. The original client has an unsafe mixed-unit duration formatter; unsafe custom intervals receive truthful bounded server chat notifications while exact timer fields remain intact. No original-client modification is required.

Resolved zone/version permission is read without automatic query retry and fails closed for mercenary activation. Reload republishes lifecycle state without automatically activating a contract. Level/stat/spell fallback is explicitly rule-gated, bounded to owner100 and only above an entire installed content ceiling; interior data gaps remain errors.

The release requires the pinned combined housing/fellowship baseline for reproducibility. Generic Spirit Shrouds, account commerce and later-client mercenary progression are separate systems and are not supplied by these protocol additions.
