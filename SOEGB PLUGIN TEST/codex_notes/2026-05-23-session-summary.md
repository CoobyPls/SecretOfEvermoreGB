# Codex Session Summary - 2026-05-23

This note records the current unfinished state so the work is visible in Git.

## Combat And Enemy Work

- Investigated Northern Jungle 1 and Mammoth Graveyard combat wiring.
- Confirmed scene-level combat hooks are present for HUD setup, player attack input, contact damage, and projectile/player-hit damage.
- Fixed regular WimpyFlower logic so plants use actor-local state instead of shared globals:
  - `L0`: attack timer
  - `L1`: iframe timer
  - `L2`: HP
  - `L3`: attack direction
- Patched the plant attack-direction failure where the plant could set its selector to `4` and stop attacking.
- Created a regular raptor prefab for future non-unique raptors:
  - always wakes on scene load
  - uses local HP/iframe/chase timer
  - no global defeated flag
  - no room activation switch
  - chases the player using relative-position checks
  - dies/deactivates only when its own local HP reaches zero
- Replaced the Mammoth Graveyard raptors with regular local-combat behavior and linked them to the new prefab.

## Menu Work

- Found and inspected the Menu scene/background.
- Started a scene-stack-based Start menu approach using the installed plugin.
- Rolled that approach back after it blocked GB Studio/project loading.
- Removed the Title Screen Start script per request.
- Left the Menu scene load-safe with no active startup script.
- The menu system is still unfinished and should be reintroduced cautiously in one test scene first.

## Plugin / Build Notes

- GB Studio loaded again after the menu rollback.
- Build then failed with duplicate engine symbol definitions.
- Isolated the problem to engine plugins replacing the same GB Studio core scene-loading files.
- Disabled these plugins by moving them out of the active `plugins` folder:
  - `Mico27/SceneStackExPlugin`
  - `Mico27/FadeStreetPlugin`
- The project should avoid using those plugin events until the engine-plugin conflicts are manually resolved or replaced with compatible versions.

## Remaining Risks

- Need a GB Studio Build/Run confirmation after the raptor and plant changes.
- Need in-editor verification that the renamed prefab resources are fully accepted by GB Studio.
- The future menu needs a safer design that does not depend on conflicting engine plugins.
- In-game HUD redesign to match the menu top HUD remains a design/implementation task.
