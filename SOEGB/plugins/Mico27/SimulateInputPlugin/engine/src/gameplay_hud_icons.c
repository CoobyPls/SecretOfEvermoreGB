#pragma bank 255

#include <gbdk/platform.h>

#include "vm.h"
#include "gbs_types.h"
#include "data/sprite_bonemenu_tileset.h"
#include "data/sprite_petal_tileset.h"
#include "data/sprite_nectar_tileset.h"
#include "gameplay_hud_icons.h"

#define HUD_EMPTY_TILE 131u
#define HUD_A_ICON_TILE 151u
#define HUD_B_ICON_TILE 153u

static const UBYTE hud_empty_pair[] = { HUD_EMPTY_TILE, HUD_EMPTY_TILE };
static const UBYTE hud_a_pair[] = { (UBYTE)HUD_A_ICON_TILE, (UBYTE)(HUD_A_ICON_TILE + 1u) };
static const UBYTE hud_b_pair[] = { (UBYTE)HUD_B_ICON_TILE, (UBYTE)(HUD_B_ICON_TILE + 1u) };

static void load_icon_tiles(UBYTE first_tile, UBYTE item) {
    switch (item) {
        case 1:
            SetBankedBkgData(first_tile, 2, sprite_bonemenu_tileset.tiles, BANK(sprite_bonemenu_tileset));
            break;
        case 2:
            SetBankedBkgData(first_tile, 2, sprite_petal_tileset.tiles, BANK(sprite_petal_tileset));
            break;
        case 3:
            SetBankedBkgData(first_tile, 2, sprite_nectar_tileset.tiles, BANK(sprite_nectar_tileset));
            break;
    }
}

void gameplay_hud_draw_icons(SCRIPT_CTX * THIS) OLDCALL BANKED {
    UBYTE a_item = *(UBYTE *)VM_REF_TO_PTR(FN_ARG1);
    UBYTE b_item = *(UBYTE *)VM_REF_TO_PTR(FN_ARG0);

    if (a_item) {
        load_icon_tiles(HUD_A_ICON_TILE, a_item);
        set_win_tiles(12, 0, 2, 1, hud_a_pair);
    } else {
        set_win_tiles(12, 0, 2, 1, hud_empty_pair);
    }

    if (b_item) {
        load_icon_tiles(HUD_B_ICON_TILE, b_item);
        set_win_tiles(17, 0, 2, 1, hud_b_pair);
    } else {
        set_win_tiles(17, 0, 2, 1, hud_empty_pair);
    }

    THIS;
}
