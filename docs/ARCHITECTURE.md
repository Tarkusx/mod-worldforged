# Architecture

The module uses AzerothCore static registration with `Addmod_worldforgedScripts()` as the compatibility entry point. Prototype spawn data is compiled into the module and resolved by map/proximity to a configured GameObject entry.

Interaction flow:

```text
GameObject click -> claim precheck -> CanStoreNewItem -> detached Item*
  -> one synchronous CharacterDatabase transaction:
       item_instance + character_inventory + worldforged claim
  -> DirectCommitTransactionWithResult
  -> AttachPersistedItem -> immediate bag visibility
```

The required generic core patch provides truthful commit results, detached
persistence helpers, and already-persisted item attachment. No active mail
delivery is used. A failed transaction leaves no inventory mutation or claim;
an attach failure after commit leaves durable state for normal login recovery.

Claims are keyed by `(character_guid, worldforged_spawn_guid)` and target effectively-once delivery. Configuration gates fail closed: Worldforged is disabled by default, Power effects are disabled, and Panic remains provenance-only.
