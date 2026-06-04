# Menu Scene First Functional Pass

## Purpose

The `Menu` scene is a pause/inventory screen built over the `Menu.png` background.
Gameplay scenes enter it through the Start button using GB Studio's native scene
stack. Pressing B while browsing restores the prior gameplay scene.

This implementation deliberately avoids the disabled `SceneStackExPlugin`, whose
engine additions previously caused duplicate-definition build failures.

## Controls

- `Start` in gameplay: store the current scene state and fade into `Menu`.
- `Up` or `Down` in the menu: cycle the browse cursor through Bone, Petal, and Nectar.
- `A` while browsing Bone: begin choosing an A/B action slot.
- `A` while browsing Petal: begin choosing a slot only when `ItemNumPetal > 0`.
- `A` while browsing Nectar: choose a slot during the visual menu test; gate it on `ItemNumNectar` once Nectar has an acquisition event.
- `Left` or `Right` while assigning: toggle the flashing cursor between A and B.
- `A` while assigning: write the selected item/icon to the chosen action slot.
- `B` while assigning: cancel back to item browsing.
- `B` while browsing: restore the gameplay scene from the native scene stack.

## Item And Icon Values

- `MenuSelectedItem = 1`: Bone.
- `MenuSelectedItem = 2`: Petal.
- `MenuSelectedItem = 3`: Nectar.
- `MenuAssignSlot = 0`: A slot.
- `MenuAssignSlot = 1`: B slot.
- `MenuAssignmentMode = false`: item browsing.
- `MenuAssignmentMode = true`: flashing A/B slot selection.
- `EquippedAItem` and `EquippedBItem`: selected item IDs for each slot.
- `EquippedAIcon` and `EquippedBIcon`: tile indexes used by the menu's A/B slot actors.
- The menu icon source is the dedicated `menuhudtiles.png` tileset. It is separate from normal text fonts and does not use an overlay.
- The tileset art uses the same GB Studio palette as the refreshed menu background (`#071821`, `#476860`, `#86c06c`, `#e0f8cf`) so the importer does not silently remap colors.
- Tile `0`: blank.
- Tile `1`: Petal.
- Tile `2`: Nectar.
- Tile `3`: Bone.
- Tile `4`: empty/depleted boxed X.
- Tile `5`: Talon.
- Tiles `6-15`: digits `0` through `9`.
- Tile `16`: HP separator slash.
- Tiles `17-18`: Boy and Dog faces held for possible future changing portrait states; the current menu background already draws the portraits.
- Tiles `19-31`: reserved blank tiles.

## Menu HUD Slot Rendering

- `Menu HUD - Equipped A Icon` and `Menu HUD - Equipped B Icon` are `8x8` actors positioned in the top menu header.
- Each actor owns a separate blank sprite resource made from tile `0` of the HUD strip. This is intentional: swapping A cannot accidentally change B if the two slots need different icons, and the base sprites contain no tileset-only colors before replacement.
- The `Sprite Tile Replacer` plugin copies the chosen `menuhudtiles` icon into each slot actor when the Menu scene opens and immediately after an assignment is confirmed.
- This renders dynamic icons directly over `Menu.png` without restoring the opaque overlay that previously hid the whole menu.

## Party HP Header

- `Menu.png` permanently contains the Boy portrait, Dog portrait, and both `HP:` labels.
- Five `8x8` display actors after each colon render current tens, current ones, slash, maximum tens, and maximum ones.
- Boy HP is read from live `PlayerHP` and `PlayerMaxHP`.
- Dog HP is read from `DogHP` and `DogMaxHP`. Since dog combat HP does not exist elsewhere yet, the menu initializes these to `10/10` only when `DogMaxHP` has never been set.
- The scene derives tens and ones using divide/modulus math, then adds `6` because digit `0` starts at HUD tile `6`.
- Merely drawing Boy or Dog face tiles in the tile sheet does not trigger replacement. A tile changes only when a script explicitly targets a display actor.

## Live Values And Overlay Warning

The earlier overlay HUD pass is disabled inside the visual Menu scene. GB Studio's
overlay is an opaque window layer: opening it at `Y=0` covers the full menu
background with a blank field, hiding the frame and sprite actors.

The top A/B item icons and the Boy/Dog HP rows now use non-destructive sprite tile
rendering. Remaining live values that need this treatment later are:

- `PlayerTalons`
- `ItemNumPetal`
- `ItemNumNectar`
- changing item and currency counts

Do not call `Open Menu HUD` or `Update Menu HUD` from the Menu scene unless the
rendering strategy is replaced with one that does not cover `Menu.png`.

## Healing Item Actions

- `Use Item - Petal Heal 5`: consumes one Petal only when injured, restores up to 5 HP, clamps to `PlayerMaxHP`, then redraws the HUD.
- `Use Item - Nectar Heal 10`: consumes one Nectar only when injured, restores up to 10 HP, clamps to `PlayerMaxHP`, then redraws the HUD.

## Intentional Boundary

This pass implements menu display, assignment state, and reusable healing action
scripts. It does not yet replace the existing live Bone attack input dispatch:
equipped A/B use should be connected after the new menu navigation is first tested
in-engine, keeping combat diagnosis separate from interface diagnosis.
