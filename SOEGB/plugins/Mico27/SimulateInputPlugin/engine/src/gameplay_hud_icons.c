#pragma bank 255

#include <gbdk/platform.h>

#include "vm.h"
#include "gbs_types.h"
#include "data/sprite_bonemenu_tileset.h"
#include "data/sprite_bonemenu_bank2_tileset.h"
#include "data/sprite_petal_tileset.h"
#include "data/sprite_petal_bank2_tileset.h"
#include "data/sprite_nectar_tileset.h"
#include "data/sprite_nectar_bank2_tileset.h"
#include "data/game_globals.h"
#include "gameplay_hud_icons.h"

#define HUD_EMPTY_TILE 131u
#define HUD_A_ICON_TILE 151u
#define HUD_B_ICON_TILE 155u
#define MENU_DIGIT_TILE_BASE 159u

BANKREF_EXTERN(tileset_menuhudtiles)
extern const struct tileset_t tileset_menuhudtiles;

static const UBYTE hud_empty_block[] = {
    HUD_EMPTY_TILE, HUD_EMPTY_TILE,
    HUD_EMPTY_TILE, HUD_EMPTY_TILE
};
static const UBYTE hud_a_block[] = {
    (UBYTE)(HUD_A_ICON_TILE + 2u), (UBYTE)HUD_A_ICON_TILE,
    (UBYTE)(HUD_A_ICON_TILE + 3u), (UBYTE)(HUD_A_ICON_TILE + 1u)
};
static const UBYTE hud_b_block[] = {
    (UBYTE)(HUD_B_ICON_TILE + 2u), (UBYTE)HUD_B_ICON_TILE,
    (UBYTE)(HUD_B_ICON_TILE + 3u), (UBYTE)(HUD_B_ICON_TILE + 1u)
};

static void load_icon_tiles(UBYTE first_tile, UBYTE item) {
    switch (item) {
        case 1:
            SetBankedBkgData(first_tile, 2, sprite_bonemenu_tileset.tiles, BANK(sprite_bonemenu_tileset));
            SetBankedBkgData(first_tile + 2u, 2, sprite_bonemenu_bank2_tileset.tiles, BANK(sprite_bonemenu_bank2_tileset));
            break;
        case 2:
            SetBankedBkgData(first_tile, 2, sprite_petal_tileset.tiles, BANK(sprite_petal_tileset));
            SetBankedBkgData(first_tile + 2u, 2, sprite_petal_bank2_tileset.tiles, BANK(sprite_petal_bank2_tileset));
            break;
        case 3:
            SetBankedBkgData(first_tile, 2, sprite_nectar_tileset.tiles, BANK(sprite_nectar_tileset));
            SetBankedBkgData(first_tile + 2u, 2, sprite_nectar_bank2_tileset.tiles, BANK(sprite_nectar_bank2_tileset));
            break;
    }
}

static void draw_item_icons(UBYTE a_item, UBYTE b_item) {
    if (a_item) {
        load_icon_tiles(HUD_A_ICON_TILE, a_item);
        set_win_tiles(12, 0, 2, 2, hud_a_block);
    } else {
        set_win_tiles(12, 0, 2, 2, hud_empty_block);
    }

    if (b_item) {
        load_icon_tiles(HUD_B_ICON_TILE, b_item);
        set_win_tiles(17, 0, 2, 2, hud_b_block);
    } else {
        set_win_tiles(17, 0, 2, 2, hud_empty_block);
    }
}

void gameplay_hud_redraw_current_items(void) BANKED {
    UBYTE a_item = (UBYTE)VM_GLOBAL(VAR_EQUIPPEDAITEM);
    UBYTE b_item = (UBYTE)VM_GLOBAL(VAR_EQUIPPEDBITEM);

    draw_item_icons(a_item, b_item);
}

void gameplay_hud_draw_icons(SCRIPT_CTX * THIS) OLDCALL BANKED {
    UBYTE a_item = *(UBYTE *)VM_REF_TO_PTR(FN_ARG1);
    UBYTE b_item = *(UBYTE *)VM_REF_TO_PTR(FN_ARG0);

    draw_item_icons(a_item, b_item);
    THIS;
}

static UBYTE menu_digit(UBYTE value) {
    return (UBYTE)(MENU_DIGIT_TILE_BASE + 6u + value);
}

static void menu_draw_two_digits(UBYTE x, UBYTE y, UBYTE value) {
    UBYTE digits[2];

    if (value > 99u) {
        value = 99u;
    }
    digits[0] = menu_digit((UBYTE)(value / 10u));
    digits[1] = menu_digit((UBYTE)(value % 10u));
    set_bkg_tiles(x, y, 2, 1, digits);
}

static void gameplay_draw_two_digits(UBYTE x, UBYTE y, UBYTE value) {
    UBYTE digits[2];

    if (value > 99u) {
        value = 99u;
    }
    digits[0] = menu_digit((UBYTE)(value / 10u));
    digits[1] = menu_digit((UBYTE)(value % 10u));
    set_win_tiles(x, y, 2, 1, digits);
}

void gameplay_hud_draw_numbers(SCRIPT_CTX * THIS) OLDCALL BANKED {
    UBYTE slash_tile = (UBYTE)(MENU_DIGIT_TILE_BASE + 16u);

    SetBankedBkgData(MENU_DIGIT_TILE_BASE, 17u, tileset_menuhudtiles.tiles, BANK(tileset_menuhudtiles));
    gameplay_draw_two_digits(3u, 0u, (UBYTE)VM_GLOBAL(VAR_PLAYERHP));
    gameplay_draw_two_digits(6u, 0u, (UBYTE)VM_GLOBAL(VAR_PLAYERMAXHP));
    gameplay_draw_two_digits(3u, 1u, (UBYTE)VM_GLOBAL(VAR_DOGHP));
    gameplay_draw_two_digits(6u, 1u, (UBYTE)VM_GLOBAL(VAR_DOGMAXHP));
    set_win_tiles(5u, 0u, 1u, 1u, &slash_tile);
    set_win_tiles(5u, 1u, 1u, 1u, &slash_tile);
    THIS;
}

void menu_hud_draw_numbers(SCRIPT_CTX * THIS) OLDCALL BANKED {
    UBYTE value;
    UWORD talons = (UWORD)VM_GLOBAL(VAR_PLAYERTALONS);
    UBYTE petal_count = (UBYTE)VM_GLOBAL(VAR_ITEMNUMPETAL);
    UBYTE nectar_count = (UBYTE)VM_GLOBAL(VAR_ITEMNUMNECTAR);
    UBYTE talon_tiles[3];
    UBYTE count_tile;

    SetBankedBkgData(MENU_DIGIT_TILE_BASE, 17u, tileset_menuhudtiles.tiles, BANK(tileset_menuhudtiles));
    menu_draw_two_digits(3u, 0u, (UBYTE)VM_GLOBAL(VAR_PLAYERHP));
    menu_draw_two_digits(6u, 0u, (UBYTE)VM_GLOBAL(VAR_PLAYERMAXHP));
    menu_draw_two_digits(3u, 1u, (UBYTE)VM_GLOBAL(VAR_DOGHP));
    menu_draw_two_digits(6u, 1u, (UBYTE)VM_GLOBAL(VAR_DOGMAXHP));
    value = (UBYTE)(MENU_DIGIT_TILE_BASE + 16u);
    set_bkg_tiles(5u, 0u, 1u, 1u, &value);
    set_bkg_tiles(5u, 1u, 1u, 1u, &value);

    if (talons > 999u) {
        talons = 999u;
    }
    talon_tiles[0] = (talons >= 100u) ? menu_digit((UBYTE)(talons / 100u)) : MENU_DIGIT_TILE_BASE;
    talon_tiles[1] = (talons >= 10u) ? menu_digit((UBYTE)((talons / 10u) % 10u)) : MENU_DIGIT_TILE_BASE;
    talon_tiles[2] = menu_digit((UBYTE)(talons % 10u));
    set_bkg_tiles(16u, 6u, 3u, 1u, talon_tiles);

    count_tile = petal_count ? menu_digit((UBYTE)((petal_count > 9u) ? 9u : petal_count)) : MENU_DIGIT_TILE_BASE;
    set_bkg_tiles(5u, 9u, 1u, 1u, &count_tile);
    count_tile = nectar_count ? menu_digit((UBYTE)((nectar_count > 9u) ? 9u : nectar_count)) : MENU_DIGIT_TILE_BASE;
    set_bkg_tiles(5u, 12u, 1u, 1u, &count_tile);
    THIS;
}
