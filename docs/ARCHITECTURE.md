# Architecture

The module uses AzerothCore static registration with `Addmod_worldforgedScripts()` as the compatibility entry point. Prototype spawn data is compiled into the module and resolved by map/proximity to a configured GameObject entry. Interaction sends the item by mail through the planned MAIL_TRANSACTION_DELIVERY path.

Claims are keyed by `(character_guid, worldforged_spawn_guid)` and target effectively-once delivery. Configuration gates fail closed: Worldforged is disabled by default, Power effects are disabled, and Panic remains provenance-only.
