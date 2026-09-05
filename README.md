# mod-worldforged

An experimental, community-developed AzerothCore module implementing a Worldforged-style exploration and personal-loot system for WotLK 3.3.5a.

Status: prototype. The module loads and has passed a disabled runtime regression smoke test on the pinned playerbots AzerothCore fork. Five prototype item definitions and coordinates are included as review data; enabled Worldforged gameplay has not yet been validated.

Worldforged-style gameplay means discoverable world objects that deliver a reconstructed item once per character. PvE/PvP Power combat effects and the Panic proc are disabled. Original Ascension client archives, assets, captures, databases, and proprietary data are not distributed. This project is not affiliated with or endorsed by Ascension, Project Ascension, or AzerothCore.

Build using the normal AzerothCore module discovery process. Copy the module into `modules/`, configure a static or dynamic module build, and review SQL before applying it. Keep `Worldforged.Enable = 0` for smoke testing.

Contributions are welcome in AzerothCore C++, SQL/schema review, coordinate validation, concurrency safety, tests, and documentation. See `CONTRIBUTING.md` and `docs/ROADMAP.md`.
