#pragma bank 255

#include <string.h>

#include "actor.h"
#include "events.h"
#include "projectiles.h"
#include "scroll.h"
#include "camera.h"
#include "system.h"
#include "trigger.h"
#include "ui.h"
#include "vm.h"
#include "gameplay_hud_icons.h"
#include "menu_pause_snapshot.h"

extern projectile_t projectiles[MAX_PROJECTILES];
extern projectile_t *projectiles_inactive_head;
extern UBYTE last_trigger_tx;
extern UBYTE last_trigger_ty;
extern UBYTE last_trigger;

// Current gameplay scenes contain at most nine scene actors plus the player.
// The larger actor limit exists for the Menu scene, which is never paused into itself.
#define MENU_PAUSE_GAMEPLAY_ACTORS 10
#define MENU_PAUSE_SRAM_BANK 3
#define MENU_PAUSE_SRAM_BASE ((menu_pause_state_t *)0xA000u)
#define TEXT_BUFFER_LEN_BANK1 ((UBYTE)(0x100u - TEXT_BUFFER_START_BANK1))

typedef struct menu_pause_state_t {
    actor_t actors[MENU_PAUSE_GAMEPLAY_ACTORS];
    actor_t *actors_active_head;
    actor_t *actors_active_tail;
    actor_t *actors_inactive_head;
    actor_t *player_collision_actor;
    UBYTE player_moving;
    UBYTE player_iframes;
    INT16 scroll_x;
    INT16 scroll_y;
    INT16 draw_scroll_x;
    INT16 draw_scroll_y;
    UINT16 camera_x;
    UINT16 camera_y;
    script_event_t input_events[8];
    UBYTE input_slots[8];
    script_event_t timer_events[MAX_CONCURRENT_TIMERS];
    timer_time_t timer_values[MAX_CONCURRENT_TIMERS];
    projectile_t projectiles[MAX_PROJECTILES];
    projectile_t *projectiles_active_head;
    projectile_t *projectiles_inactive_head;
    UBYTE last_trigger_tx;
    UBYTE last_trigger_ty;
    UBYTE last_trigger;
    UBYTE hud_window_tiles[40];
    UBYTE hud_window_tile_data[TEXT_BUFFER_LEN * 16u];
#ifdef CGB
    UBYTE hud_window_attrs[40];
    UBYTE hud_window_tile_data_bank1[TEXT_BUFFER_LEN_BANK1 * 16u];
#endif
    UBYTE win_pos_x;
    UBYTE win_pos_y;
} menu_pause_state_t;

static UBYTE menu_pause_snapshot_valid;

UBYTE menu_pause_restore_pending;

void menu_pause_capture(SCRIPT_CTX * THIS) OLDCALL BANKED {
    UBYTE previous_ram_bank;
    menu_pause_state_t *paused;

    THIS;
    previous_ram_bank = _current_ram_bank & RAM_BANKS_ONLY;
    SWITCH_RAM_BANK(MENU_PAUSE_SRAM_BANK, RAM_BANKS_ONLY);
    paused = MENU_PAUSE_SRAM_BASE;
    memcpy(paused->actors, actors, sizeof(paused->actors));
    paused->actors_active_head = actors_active_head;
    paused->actors_active_tail = actors_active_tail;
    paused->actors_inactive_head = actors_inactive_head;
    paused->player_collision_actor = player_collision_actor;
    paused->player_moving = player_moving;
    paused->player_iframes = player_iframes;
    paused->scroll_x = scroll_x;
    paused->scroll_y = scroll_y;
    paused->draw_scroll_x = draw_scroll_x;
    paused->draw_scroll_y = draw_scroll_y;
    paused->camera_x = camera_x;
    paused->camera_y = camera_y;
    memcpy(paused->input_events, input_events, sizeof(paused->input_events));
    memcpy(paused->input_slots, input_slots, sizeof(paused->input_slots));
    memcpy(paused->timer_events, timer_events, sizeof(paused->timer_events));
    memcpy(paused->timer_values, timer_values, sizeof(paused->timer_values));
    memcpy(paused->projectiles, projectiles, sizeof(paused->projectiles));
    paused->projectiles_active_head = projectiles_active_head;
    paused->projectiles_inactive_head = projectiles_inactive_head;
    paused->last_trigger_tx = last_trigger_tx;
    paused->last_trigger_ty = last_trigger_ty;
    paused->last_trigger = last_trigger;
    get_win_tiles(0, 0, 20, 2, paused->hud_window_tiles);
    get_win_data(TEXT_BUFFER_START, TEXT_BUFFER_LEN, paused->hud_window_tile_data);
#ifdef CGB
    if (_is_CGB) {
        VBK_REG = 1;
        get_win_tiles(0, 0, 20, 2, paused->hud_window_attrs);
        get_win_data(TEXT_BUFFER_START_BANK1, TEXT_BUFFER_LEN_BANK1, paused->hud_window_tile_data_bank1);
        VBK_REG = 0;
    }
#endif
    paused->win_pos_x = win_pos_x;
    paused->win_pos_y = win_pos_y;
    SWITCH_RAM_BANK(previous_ram_bank, RAM_BANKS_ONLY);
    menu_pause_snapshot_valid = TRUE;
    menu_pause_restore_pending = FALSE;
}

void menu_pause_request_restore(SCRIPT_CTX * THIS) OLDCALL BANKED {
    THIS;
    if (menu_pause_snapshot_valid) {
        menu_pause_restore_pending = TRUE;
    }
}

void menu_pause_restore(void) BANKED {
    UBYTE previous_ram_bank;
    UBYTE i;
    actor_t *actor;
    menu_pause_state_t *paused;

    if (!menu_pause_restore_pending || !menu_pause_snapshot_valid) {
        return;
    }
    previous_ram_bank = _current_ram_bank & RAM_BANKS_ONLY;
    SWITCH_RAM_BANK(MENU_PAUSE_SRAM_BANK, RAM_BANKS_ONLY);
    paused = MENU_PAUSE_SRAM_BASE;
    memcpy(actors, paused->actors, sizeof(paused->actors));
    actors_active_head = paused->actors_active_head;
    actors_active_tail = paused->actors_active_tail;
    actors_inactive_head = paused->actors_inactive_head;
    player_collision_actor = paused->player_collision_actor;
    player_moving = paused->player_moving;
    player_iframes = paused->player_iframes;
    scroll_x = paused->scroll_x;
    scroll_y = paused->scroll_y;
    draw_scroll_x = paused->draw_scroll_x;
    draw_scroll_y = paused->draw_scroll_y;
    camera_x = paused->camera_x;
    camera_y = paused->camera_y;
    memcpy(input_events, paused->input_events, sizeof(paused->input_events));
    memcpy(input_slots, paused->input_slots, sizeof(paused->input_slots));
    for (i = 0; i != 8; ++i) {
        input_events[i].handle = 0;
    }
    memcpy(timer_events, paused->timer_events, sizeof(paused->timer_events));
    memcpy(timer_values, paused->timer_values, sizeof(paused->timer_values));
    for (i = 0; i != MAX_CONCURRENT_TIMERS; ++i) {
        timer_events[i].handle = 0;
    }
    memcpy(projectiles, paused->projectiles, sizeof(paused->projectiles));
    projectiles_active_head = paused->projectiles_active_head;
    projectiles_inactive_head = paused->projectiles_inactive_head;
    last_trigger_tx = paused->last_trigger_tx;
    last_trigger_ty = paused->last_trigger_ty;
    last_trigger = paused->last_trigger;
    set_win_data(TEXT_BUFFER_START, TEXT_BUFFER_LEN, paused->hud_window_tile_data);
    set_win_tiles(0, 0, 20, 2, paused->hud_window_tiles);
#ifdef CGB
    if (_is_CGB) {
        VBK_REG = 1;
        set_win_data(TEXT_BUFFER_START_BANK1, TEXT_BUFFER_LEN_BANK1, paused->hud_window_tile_data_bank1);
        set_win_tiles(0, 0, 20, 2, paused->hud_window_attrs);
        VBK_REG = 0;
    }
#endif
    ui_set_pos(paused->win_pos_x, paused->win_pos_y);
    SWITCH_RAM_BANK(previous_ram_bank, RAM_BANKS_ONLY);
    actor = actors_active_head;
    while (actor) {
        actor->hscript_hit = SCRIPT_TERMINATED;
        actor->hscript_update = SCRIPT_TERMINATED;
        if (actor->script_update.bank) {
            script_execute(actor->script_update.bank, actor->script_update.ptr, &(actor->hscript_update), 0);
        }
        actor = actor->next;
    }
    gameplay_hud_redraw_current_items();
    menu_pause_snapshot_valid = FALSE;
    menu_pause_restore_pending = FALSE;
}
