# Personal loot

The selected architecture is transactional direct-to-bag delivery. Claim identity
is `(character_guid, worldforged_spawn_guid)` with `item_entry`, `state`,
`delivered_item_guid`, and `delivered_at`. A unique key prevents concurrent
double-click duplication. Bag space is checked before creating an item; a full
bag leaves the discovery unclaimed. Item persistence, inventory placement, and
the claim commit together, and the same item is attached to player memory only
after a successful commit. Historical mail from the v0.1 prototype is retained
as history; new deliveries do not create mail.
