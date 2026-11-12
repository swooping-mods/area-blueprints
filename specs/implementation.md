# Area Blueprints: behavior and implementation

Describes the retained payload **0.1.0**. The behavior prose describes the retained payload; current candidate source and metadata are identified below. Gameplay qualification is a separate requirement.


<!-- source-only-sync -->
## Current source candidate

Source version **0.2.0** is saved from workshop revision `ca32bbf79ce0840d5166a232bdb3a9fdcd761254`. The retained installable payload remains **0.1.0** with its original bytes. This source update does not rebuild or requalify that payload. The current source and metadata define the candidate; gameplay, rendered behavior and co-op remain unqualified for rebuilt bytes.

The authored behavior and lifecycle descriptions below are retained from the published payload. They are not new qualification evidence for the source candidate. The integration table below follows the current candidate metadata.
<!-- /source-only-sync -->

## Behavior

Alt+B enters survey flight. F6 and F7 select opposite corners of a padded world-aligned volume; F8 cycles five blueprint slots. Capture records at most 128 standard buildings with kind, relative position and yaw. Ctrl+R rebuilds at the reticle with the normal building tool. Each queued piece passes the native planner/checker and normal build/payment path; rejection stops the queue and leaves completed pieces in place. Foundations retain native grid planning. Survey and detached-camera consumers arbitrate through the shared camera service.

## State and lifecycle

Blueprint slots are checksummed files. Conveyors and free-pose records are excluded from capture. The active selection and rebuild queue are session state. Leaving survey restores the captured player pose and flying mode; failed restoration retains the camera claim for retry. Mutations require a ready local-host world and admitted input.

## Integration and configuration

No additional feature-mod runtime dependency is declared. Included shared headers are source reuse, not extra DLL dependencies.

The [module metadata](../source/mods/area-blueprints/mod.json) owns the version and declared integration contract. Native modules start at `MoonLab_ModStart`. Game/Unity integrations target the pinned Steam Windows x64 IL2CPP build.

Packaged defaults in [settings.ini](../source/mods/area-blueprints/settings.ini); supported bounds and interpretation remain in the linked parser.

| Section | Setting | Default |
|---|---|---|
| `[blueprints]` | `slot` | `1` |

## Implementation sources

- [mod.c](../source/mods/area-blueprints/mod.c)
- [input.h](../source/mods/area-blueprints/input.h)
- [camera_restore.h](../source/mods/area-blueprints/camera_restore.h)
- [storage.h](../source/mods/area-blueprints/storage.h)

## Verification boundary

No arbitrary blueprint rotation, inventory/content copy, support remapping or large-distance spherical relocation. Rebuilding, camera restoration, save round trips and combined camera use require game-level qualification.

`python3 build.py --check` verifies the exported source hashes. It does not execute the mod or qualify the behaviors above. Installation and independent build prerequisites are in the [README](../README.md).
