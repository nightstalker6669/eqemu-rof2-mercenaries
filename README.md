# EQEmu RoF2 Mercenaries

Production mercenary support for EQEmu targeting the **May 10, 2013 Rain of Fear 2 client**. Published October 23, 2026. This is a source extension of a pinned EQEmu checkout, not a complete server distribution.

The package includes recovered opcodes and structures, native liaison and management windows, durable owned contracts, all four combat roles, paid hiring/upkeep, suspension/death/revival, assistance and stances, multi-contract switching, ownership transfer, SQL migrations and deployment documentation.

## Exact dependencies

Apply these versions in order:

1. EQEmu upstream `8175ae61874cbf20d7b05102821f6f8a4f4d1c39`.
2. [eqemu-rof2-housing](https://github.com/nightstalker6669/eqemu-rof2-housing) `9aad4cc0857bdd14677dd45ab24f3eafe20d489b`.
3. [eqemu-rof2-fellowship](https://github.com/nightstalker6669/eqemu-rof2-fellowship) `9382453264f4f84ac7e83a9ee5c9c9a1391c6017`.
4. This mercenary extension.

Mercenary transactions use the shared authenticated inventory writer leases and pending-operation guards supplied by housing. The published source baseline also includes fellowship and its required DBcore connection-storage fix. These are implementation and packaging dependencies, not inherent EverQuest mercenary requirements. Their code is supplied by the pinned companion repositories; it is not silently copied into this package. See [installation](docs/INSTALL.md) and [architecture](docs/ARCHITECTURE.md).

## Supported behavior

- Native liaison catalogue, advertised initial stance, hiring and immediate group membership/HP publication.
- Tank, healer, melee damage and caster damage roles; Passive and role-aware stance behavior; automatic assistance, manual group-enemy assistance and healer Puller override.
- Up to three owned contracts by explicit server rule, one active at a time; packed native roster indices correctly handle sparse storage slots.
- Independent purchase/upkeep charging, stored Bayle Mark fallback, atomic coin debits, Passive-period upkeep waiver and exact timer restoration.
- Suspension, durable death/revival deadlines, zone permissions, explicit resume and level 86–100 stat/spell fallback.
- Consensual ownership transfer with expiry, decline, receiver capacity and pricing checks, durable ownership change and preservation of the receiver's active mercenary.
- Authenticated atomic row/buff/payment/group persistence with guarded failure and uncertain-commit handling.

Core behavior passed original-client acceptance, including four-role combat, healer resurrection, six-member group replacement, transfer, recurring and mixed-unit upkeep periods, death/revival, zone restrictions and zone/relog/server-restart persistence. Disposable accounts and client profiles were removed afterward. See [validation status](docs/STATUS.md).

**Exact historical Live balance is not claimed.** Three-contract capacity, fee/scaling choices and assistance policies are documented emulator behavior. Marketplace Buy Slot and premium rename commerce, full player shrouding and later mercenary AA/player-equipped gear are outside the implemented scope. See [known gaps](docs/KNOWN-GAPS.md).

## Protocol documentation

The matching client has **27 mapped mercenary application messages: 13 existing and 14 recovered**. Documentation separates implemented handlers, mapped-only actions and unresolved semantics.

- [Opcode inventory, directions and implementation status](protocol/OPCODES.md)
- [Structures, offsets, envelopes, command/result values and timers](protocol/STRUCTS.md)
- [Machine-readable opcodes](protocol/opcodes.json) and [wire layouts](protocol/wire-layouts.json)
- [Recovery evidence and confidence limits](protocol/EVIDENCE.md)
- Authoritative native wire helper: [rof2_wire.h](overlay/common/mercenary/rof2_wire.h)

## Install and layout

Read [INSTALL.md](docs/INSTALL.md) before changing sources or databases. After the exact companion overlays are applied:

```sh
python3 tools/apply_overlay.py /path/to/EQEmu --check
python3 tools/apply_overlay.py /path/to/EQEmu --apply
```

The installer checks package hashes, every prerequisite source path and patch applicability. It changes sources only; it does not migrate SQL, build or restart a server.

| Path | Contents |
| --- | --- |
| `overlay/` | New production wire/runtime/transaction/transfer sources and four additive SQL migrations |
| `patches/eqemu-mercenaries.patch` | Mercenary integration against the pinned combined baseline |
| `patches/base-manifest.json` | SHA-256 inventory of all prerequisite modified/new source files |
| `protocol/` | Opcodes, structures and evidence limits |
| `docs/` | Installation, architecture, gameplay policies, validation and gaps |
| `tools/apply_overlay.py` | Guarded source installer |
| `source-manifest.json` | Publication inventory and hashes |

Production code, SQL, installation tooling and documentation only. No test suites, fixtures, generated reports, logs, credentials, client assets, binaries or personal runtime configuration are included. Unrelated petition changes in the development checkout are excluded. GPLv3: see [LICENSE](LICENSE) and [NOTICE](NOTICE).
