# GBS Magic System

Reusable GB Studio magic/alchemy helper plugin.

This is a prototype plugin package. It is intentionally parked in `plugins_disabled` for now so it does not conflict with the active project build.

## What It Does

- Adds a `Magic: Cast Spell` event.
- Adds a `Magic: Update Timers` event.
- Finds the nearest valid target in range.
- Writes rolled spell damage into a variable you choose.
- Calls the target actor's hit script with a collision group you choose.
- Temporarily disables the target actor so the spell can interrupt enemy behavior.
- Shows one reusable hidden effect actor, then hides it again.

## Required Scene Setup

Each combat scene needs one reusable non-solid spell effect actor.

Recommended setup:

- Name: `Spell Effect`
- Collision Group: `None`
- Solid: off
- Hidden on start
- Reserved Tiles: a unique marker number, such as `23`
- Sprite: whatever spell effect you want to show for this cast

The plugin finds this actor by `Reserved Tiles`. That is a little ugly, but it keeps the event generic and avoids needing one hidden actor per spell.

## Event Setup

Use `Magic: Cast Spell` from a button script, item script, or alchemy menu script.

Important fields:

- `Spell ID`: A number representing the spell. For example, `1 = Storm`.
- `Damage Variable`: The variable your enemy hit scripts read as the incoming damage.
- `Min Damage` / `Max Damage`: Roll range.
- `Range (px)`: Max distance from the player.
- `Freeze Frames`: How long the target is disabled.
- `Effect Actor Reserved Tiles`: The marker used to find the reusable effect actor.
- `Effect Visible Frames`: How long the effect stays visible.
- `Effect Y Offset (px)`: How far above the enemy to draw the effect.
- `Hit Collision Group`: Collision group passed into the target actor's hit script.

Then call `Magic: Update Timers` once per frame while the scene is running. That can be done from a scene/brain actor update script, or by merging `magic_system_update()` into the engine core update loop.

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
- Optional ingredient variables.
- Optional MP/ingredient checks.
- Optional knockback with tile collision safeguards.
- Optional spell table JSON or JS data file for cleaner authoring.
