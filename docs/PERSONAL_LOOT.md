# Personal loot

The selected architecture is MAIL_TRANSACTION_DELIVERY. Claim identity is `(character_guid, worldforged_spawn_guid)` with `item_entry`, `state`, `delivered_item_guid`, and `delivered_at`. A unique key prevents concurrent double-click duplication; mail avoids inventory-full loss. Character-delete cleanup removes claim rows. Crash-recovery and enabled live delivery still require runtime validation. Delivered items are never regenerated merely because a player later removes them.
