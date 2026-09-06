# Changelog

## v0.1.0-prototype

- Compiles and links on the pinned playerbots AzerothCore fork.
- Loads with Worldforged disabled and passed runtime regression smoke testing.
- Includes a five-item prototype review dataset.
- Not production-ready; enabled gameplay, mass deployment, upgrades, Rune of Ascension, and Power combat remain unvalidated.
# v0.2.0-direct-bag (live validated)

- Replaced active mail delivery with immediate transactional direct-to-bag delivery.
- Added atomic `item_instance` + `character_inventory` + claim persistence.
- Added truthful synchronous commit-result handling and duplicate protection.
- Added bag-space preflight and relog recovery semantics.
- Live validated with item 450705 (Dustrunner Bindings); no mail required.

The v0.1 prototype used mail and required relog for online visibility.
