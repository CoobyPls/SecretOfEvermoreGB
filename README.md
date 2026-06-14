# Coobsoft VBlank Plugin Demo

This is a tiny GB Studio project showing the VBlank Tile Streamer in action.

Open `VBlank Plugin Demo.gbsproj` in GB Studio 4.3.x and hit Run. The game starts in the Stars scene. Press `A` and it will randomly cast Crush, Flash, or Storm on the cat.

The point of the demo is simple: the effect actor does not need to keep every spell graphic loaded all the time. The plugin streams the sprite tiles in, then the scene shows the effect.

Flash is set up as the moving example. It starts from the player and travels to the cat.

The streamed graphics always use OBJ sprite tiles `192-239`. That keeps them in the same VRAM spot every time, away from GB Studio's text tiles.

## Try The Demo

1. Open `VBlank Plugin Demo.gbsproj` in GB Studio 4.3.x.
2. Run the project.
3. Press `A` in the Stars scene.
4. Watch the reusable spell actor stream in Crush, Flash, or Storm.

## Where Things Are

- Plugin: `plugins/VBlankTileStreamerPlugin`
- Demo scene: `project/scenes/stars/scene.gbsres`
- Effect sprites: `assets/sprites`
- Sounds: `assets/sounds`

## Using It Elsewhere

1. Copy `plugins/VBlankTileStreamerPlugin` into another GB Studio project.
2. Reopen the project so GB Studio sees the plugin events.
3. Open:

```text
plugins/VBlankTileStreamerPlugin/engine/src/coobsoft_vblank_streamer.c
```

4. Register your own sprite tilesets in the `vstream_assets` list.
5. Add one reusable effect actor to your scene.
6. Use the included VBlank Stream events in your scene scripts.

The longer setup notes are in `plugins/VBlankTileStreamerPlugin/README.md`.
