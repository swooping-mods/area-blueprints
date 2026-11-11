# Area Blueprints

Save five area layouts in selection flight; rebuild standard structures with vanilla placement checks and costs.

## Install

Install [Moon Sweepers Mod Loader](https://github.com/swooping-mods/moon-sweepers-mod-loader) first.

Download [area-blueprints-0.1.0.msmod](content/area-blueprints-0.1.0.msmod), import it with **Install mod package**, then select the mod and its required frameworks.

## Controls

- Alt+B opens selection flight; F6/F7 select opposite corners and save.
- F8 cycles five slots; Ctrl+R rebuilds at the reticle while holding the normal building tool.
- Normal foundation planning keeps grid snapping. Conveyors and free-pose pieces are excluded.

## Compatibility

Version 0.1.0. Targets the pinned Steam Windows x64 IL2CPP build of Moon Sweepers (Steam app 5335510). On Linux, the game runs through Proton. Experimental: back up saves before using save-affecting mods.

## Build

Run `python3 build.py --check` to verify source hashes, then `python3 build.py` to write a build under `.local/build/`. Requires installed Clang, lld-link and llvm-dlltool.

## Credits

See [CREDITS.txt](CREDITS.txt) and [third-party notices](vendored/THIRD_PARTY_NOTICES.md).
