#pragma bank 255

#include "actor.h"
#include "data_manager.h"
#include "events.h"
#include "gameplay_hud_icons.h"
#include "menu_pause_snapshot.h"
#include "data/script_input_2.h"

static upoint16_t paused_actor_positions[MAX_ACTORS];
static UBYTE paused_actor_flags[MAX_ACTORS];
static direction_e paused_actor_dirs[MAX_ACTORS];
static UBYTE paused_actor_frames[MAX_ACTORS];
static UBYTE paused_actor_animations[MAX_ACTORS];
static UBYTE paused_actors_len;
static UBYTE menu_pause_snapshot_valid;

UBYTE menu_pause_restore_pending;

static void menu_pause_reattach_start_input(void) {
    input_events[0].handle = 0;
    input_events[0].script_bank = BANK(script_input_2);
    input_events[0].script_addr = (void *)script_input_2;
    input_slots[7] = 0x81u;
}

void menu_pause_capture(SCRIPT_CTX * THIS) OLDCALL BANKED {
    UBYTE i;

    THIS;
    paused_actors_len = actors_len;
    for (i = 1u; i != paused_actors_len; ++i) {
        paused_actor_positions[i] = actors[i].pos;
        paused_actor_flags[i] = actors[i].flags;
        paused_actor_dirs[i] = actors[i].dir;
        paused_actor_frames[i] = actors[i].frame;
        paused_actor_animations[i] = actors[i].animation;
    }
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
    UBYTE i;
    UBYTE restore_len;

    if (!menu_pause_restore_pending || !menu_pause_snapshot_valid) {
        return;
    }

    restore_len = (actors_len < paused_actors_len) ? actors_len : paused_actors_len;
    for (i = 1u; i != restore_len; ++i) {
        actors[i].pos = paused_actor_positions[i];
        actors[i].flags = (paused_actor_flags[i] & ~(ACTOR_FLAG_ACTIVE | ACTOR_FLAG_INTERRUPT));
        actors[i].dir = paused_actor_dirs[i];
        actors[i].animation = paused_actor_animations[i];
        actors[i].frame = paused_actor_frames[i];
        actor_reset_anim(&actors[i]);
    }

    gameplay_hud_redraw_current_items();
    menu_pause_reattach_start_input();
    menu_pause_snapshot_valid = FALSE;
    menu_pause_restore_pending = FALSE;
}
