# GBS Magic System

Reusable GB Studio magic/alchemy helper plugin.

This is a prototype plugin package. It is intentionally parked in `plugins_disabled` for now so it does not conflict with the active project build.

## What It Does

- Adds a `Magic: Cast Spell` event.
- Adds a `Magic: Update Timers` event.
- Finds the nearest valid target in range.
- Supports two behavior modes: `Burst` and `Missile`.
- Checks and subtracts up to two ingredient variables.
- Writes rolled spell damage into a variable you choose.
- Calls the target actor's hit script with a collision group you choose after the effect lands.
- Temporarily disables the target actor for burst spells so the animation can interrupt enemy behavior.
- Shows one reusable hidden effect actor, then hides it again.

## Required Scene Setup

Each combat scene needs one reusable non-solid spell effect actor.

Recommended setup:

- Name: `Spell Effect`
- Collision Group: `None`
- Solid: off
- Hidden on start
- Reserved Tiles: the spell actor's runtime reserved tile count. For the current `Storm.png` actor in SOEGB, use `23`.
- Sprite: whatever spell effect you want to show for this cast

The plugin finds this actor by `Reserved Tiles`. That is a little ugly, but it keeps the event generic and avoids needing one hidden actor per spell.

## Event Setup

Use `Magic: Cast Spell` from a button script, item script, or alchemy menu script.

Important fields:

- `Spell`: editor label only. The Game Boy uses `Spell ID`.
- `Spell ID`: A number representing the spell. For example, `1 = Storm`.
- `Behavior`: `Burst` appears on the target; `Missile` starts in front of the player and travels toward the target.
- `Damage Variable`: The variable your enemy hit scripts read as the incoming damage.
- `Min Damage` / `Max Damage`: Roll range.
- `Ingredient 1 Variable` / `Ingredient 1 Cost`: first formula ingredient.
- `Ingredient 2 Variable` / `Ingredient 2 Cost`: second formula ingredient.
- `Range (px)`: Max distance from the player.
- `Freeze Frames`: How long the target is disabled.
- `Effect Actor Reserved Tiles`: The runtime reserved tile count used to find the reusable effect actor.
- `Effect Visible Frames`: How long the effect stays visible.
- `Missile Speed (px/frame)`: how fast missile spells travel.
- `Effect Y Offset (px)`: How far above the enemy to draw the effect.
- `Target Collision Group`: Which collision group the spell is allowed to target.
- `Hit Collision Group`: Collision group passed into the target actor's hit script.

Then call `Magic: Update Timers` once per frame while the scene is running. That can be done from a scene/brain actor update script, or by merging `magic_system_update()` into the engine core update loop.

## Behavior Modes

### Burst

Burst spells are for effects like Storm or Crush.

Flow:

1. Find nearest target.
2. Check ingredient costs.
3. Roll damage.
4. Move the effect actor to the target.
5. Freeze the target while the animation window runs.
6. Call the target hit script when the effect window ends.
7. Hide the effect actor and restore the target if it survived.

### Missile

Missile spells are for effects that start near the player and fly toward the target.

Flow:

1. Find nearest target.
2. Check ingredient costs.
3. Roll damage.
4. Spawn the effect actor 16 pixels in front of the player's facing direction.
5. Move it toward the target at `Missile Speed`.
6. Call the target hit script when it reaches the target or when its lifetime expires.
7. Hide the effect actor.

The prototype missile path moves directly toward the target. Wall/path avoidance is a future improvement.

## 32x32 Spell Guidance

32x32 effects are possible, but the safe budget depends on visible tiles per frame, not only canvas size.

Recommended art targets:

- `Small`: up to 4 visible tiles per frame.
- `Medium`: up to 8 visible tiles per frame.
- `Large`: up to 16 visible tiles per frame.

Current test examples:

- `Crush`: 32x32 canvas, 8 visible tiles max per frame, 22-ish unique tiles. Good `Medium` spell.
- `Storm`: 32x32 canvas, 9 visible tiles max per frame, 19-ish unique tiles. Good `Medium` spell.

The current prototype still uses normal GB Studio sprite loading for the effect actor. The next optimization step is runtime tile streaming: reserve a fixed actor tile pool, then copy only the current spell frame into that actor's `base_tile` area.

## Why This Is Plugin-Friendly

The plugin does not hard-code Storm, talons, Secret of Evermore GB variables, or one specific menu. It just provides a reusable spell cast primitive.

Your game-specific layer still decides:

- Which spells are known
- Which spells are equipped
- Which icon appears in the HUD
- Which ingredients are consumed
- Which spell ID means which effect

## Roadmap

Good next features:

- `Magic: Cast Equipped Spell`, using an A/B slot variable instead of a fixed spell ID.
- Cursor targeting mode.
- Spell effect actor state selection, so one actor can display different spell sprites.
- Runtime sprite tile streaming for 32x32 spell effects.
- Optional knockback with tile collision safeguards.
- Optional spell table JSON or JS data file for cleaner authoring.
