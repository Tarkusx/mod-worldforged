# Migration to v0.2.0-direct-bag

1. Stop the worldserver.
2. Apply `patches/0003-worldforged-transactional-inventory.patch` to the pinned AzerothCore source.
3. Rebuild AzerothCore and update this module.
4. Preserve the existing `worldforged_character_loot` table, claims, and historical mail.
5. Restart and smoke-test with Worldforged disabled before enabling it.

The active flow changes from mail delivery (v0.1) to bag preflight plus a
synchronous atomic item/inventory/claim transaction. Do not delete historical
claims or mail records during migration.
