# Required core patch

`patches/0003-worldforged-transactional-inventory.patch` is generic
AzerothCore infrastructure. It adds truthful synchronous transaction results,
detached item and inventory persistence helpers, and an already-persisted
`Item*` attachment primitive. It contains no Worldforged tables, spawn IDs,
item IDs, or module-specific logic.
