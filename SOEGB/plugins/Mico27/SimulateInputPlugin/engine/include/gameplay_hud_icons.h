#ifndef GAMEPLAY_HUD_ICONS_H
#define GAMEPLAY_HUD_ICONS_H

#include <gbdk/platform.h>
#include "vm.h"

void gameplay_hud_draw_icons(SCRIPT_CTX * THIS) OLDCALL BANKED;
void gameplay_hud_redraw_current_items(void) BANKED;
void menu_hud_draw_numbers(SCRIPT_CTX * THIS) OLDCALL BANKED;

#endif
