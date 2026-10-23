# Mercenary opcode inventory

Target: May10 2013 RoF2, SHA-256 `2a8702ad9f722704f01355c0750be7d6f164a8b9c9128ba0cf286ea32b405b0e`. Addresses are preferred-image virtual addresses.

27 mappings: 13 existing and14 recovered. Application lengths exclude transport framing. See [STRUCTS.md](STRUCTS.md) for field-level layouts and [opcodes.json](opcodes.json) for sender/consumer evidence and implementation status.

| Opcode | Canonical name | Native directions | Implementation |
| --- | --- | --- | --- |

| `0x7b89` | `OP_MercenaryDataUpdateRequest` | C2S | implemented |

| `0x61a4` | `OP_MercenaryDataUpdate` | S2C | implemented |

| `0x11c1` | `OP_MercenaryDataRequest` | C2S | implemented |

| `0x72ce` | `OP_MercenaryDataResponse` | S2C | implemented |

| `0x7169` | `OP_MercenaryHire` | C2S, S2C | implemented |

| `0x6e83` | `OP_MercenaryDismiss` | C2S, S2C | implemented |

| `0x31e4` | `OP_MercenaryTimerRequest` | C2S | implemented |

| `0x0763` | `OP_MercenaryTimer` | S2C | implemented |

| `0x5d26` | `OP_MercenaryUnknown1` | S2C | existing_compatibility_notification_semantics_unresolved |

| `0x27f2` | `OP_MercenaryCommand` | C2S, S2C | implemented |

| `0x4407` | `OP_MercenarySuspendRequest` | C2S | implemented |

| `0x6f03` | `OP_MercenarySuspendResponse` | S2C | implemented |

| `0x27a0` | `OP_MercenaryUnsuspendResponse` | S2C | implemented |

| `0x0b72` | `OP_MercenaryAutoAssist` | C2S | implemented |

| `0x7e6f` | `OP_MercenaryAssist` | C2S | implemented |

| `0x1b37` | `OP_MercenarySwitch` | C2S, S2C | implemented |

| `0x5df8` | `OP_MercenaryTransferRequest` | C2S, S2C | implemented |

| `0x4333` | `OP_MercenaryTransferResponse` | C2S, S2C | implemented |

| `0x69ca` | `OP_MercenaryUnshroud` | S2C | mapped_only_general_shrouding_not_implemented |

| `0x6e9f` | `OP_MercenaryUnshroudResponse` | C2S | mapped_only_general_shrouding_not_implemented |

| `0x038f` | `OP_MercenaryDead` | S2C | implemented |

| `0x0e52` | `OP_MercenaryRestricted` | S2C | implemented |

| `0x2749` | `OP_MercenaryRestoreReady` | S2C | implemented |

| `0x5f88` | `OP_MercenaryUpkeep` | S2C | implemented |

| `0x3cbc` | `OP_MercenaryCostUpdate` | S2C | implemented |

| `0x29ec` | `OP_MercenaryCurrencyMessage` | S2C | mapped_typed_layout_available_no_current_emitter |

| `0x2b4f` | `OP_MercenaryTimerUpdate` | S2C | implemented |
