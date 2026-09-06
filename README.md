# mod-worldforged

An experimental, community-developed AzerothCore module implementing a Worldforged-style exploration and personal-loot system for WotLK 3.3.5a.

Status: v0.2.0-direct-bag candidate, live validated with prototype item 450705. This demonstrates the discovery/delivery framework; it is not a complete CoA reconstruction and does not deploy all 1,864 Worldforged families.

Worldforged-style gameplay means discoverable world objects that deliver a reconstructed item once per character. PvE/PvP Power combat effects and the Panic proc are disabled. Original Ascension client archives, assets, captures, databases, and proprietary data are not distributed. This project is not affiliated with or endorsed by Ascension, Project Ascension, or AzerothCore.

Build using the normal AzerothCore module discovery process and apply the generic `patches/0003-worldforged-transactional-inventory.patch` to the pinned core before rebuilding. Keep `Worldforged.Enable = 0` for smoke testing, then enable it only after reviewing prototype data.

Direct-to-bag delivery performs bag-space preflight, persists `item_instance`, `character_inventory`, and the composite personal claim in one synchronous transaction, then attaches the same item to player memory. A full bag creates no item or claim; transaction failure rolls back; an unexpected post-commit attach failure leaves the database authoritative for relog recovery. A second click is blocked by the claim key. New deliveries do not use mail.

See `docs/ARCHITECTURE.md` and `docs/MIGRATION_V020.md` for the core API and upgrade notes.

Contributions are welcome in AzerothCore C++, SQL/schema review, coordinate validation, concurrency safety, tests, and documentation. See `CONTRIBUTING.md` and `docs/ROADMAP.md`.
