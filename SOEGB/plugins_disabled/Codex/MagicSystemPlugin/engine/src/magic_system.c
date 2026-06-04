#pragma bank 255

#include <gbdk/platform.h>
#include <rand.h>

#include "actor.h"
#include "collision.h"
#include "macro.h"
#include "vm.h"
#include "magic_system.h"

#define MAGIC_BEHAVIOR_BURST 0u
#define MAGIC_BEHAVIOR_MISSILE 1u
#define MAGIC_TARGET_NEAREST_PLAYER 0u

#define MAGIC_STATE_IDLE 0u
#define MAGIC_STATE_BURST 1u
#define MAGIC_STATE_MISSILE 2u

static actor_t *magic_frozen_target;
static actor_t *magic_effect_actor;
static actor_t *magic_pending_target;
static UBYTE magic_state;
static UBYTE magic_freeze_timer;
static UBYTE magic_effect_timer;
static UBYTE magic_spell_id;
static UBYTE magic_behavior;
static UBYTE magic_target_mode;
static UBYTE magic_range_px;
static UBYTE magic_hit_collision_mask;
static UBYTE *magic_damage_out;
static UBYTE magic_min_damage;
static UBYTE magic_max_damage;
static UBYTE *magic_ingredient1;
static UBYTE *magic_ingredient2;
static UBYTE magic_ingredient1_cost;
static UBYTE magic_ingredient2_cost;
static UBYTE magic_freeze_frames;
static UBYTE magic_effect_reserved_tiles;
static UBYTE magic_effect_frames;
static UBYTE magic_missile_speed_px;
static UBYTE magic_effect_y_offset_px;

static UWORD magic_axis_distance(UWORD a, UWORD b) {
    return (a > b) ? (a - b) : (b - a);
}

static UWORD magic_distance_to_player(actor_t *actor) {
    return magic_axis_distance(actor->pos.x, PLAYER.pos.x) +
           magic_axis_distance(actor->pos.y, PLAYER.pos.y);
}

static UBYTE magic_is_targetable(actor_t *actor, UBYTE collision_mask) {
    if (!actor || actor == &PLAYER) return FALSE;
    if (!CHK_FLAG(actor->flags, ACTOR_FLAG_ACTIVE) ||
        CHK_FLAG(actor->flags, ACTOR_FLAG_HIDDEN | ACTOR_FLAG_DISABLED)) return FALSE;
    if (!(actor->collision_group & collision_mask)) return FALSE;
    if (!actor->script.bank || !(actor->hscript_hit & SCRIPT_TERMINATED)) return FALSE;
    return TRUE;
}

static actor_t *magic_find_effect_actor(UBYTE effect_reserved_tiles) {
    UBYTE i;
    actor_t *actor;

    for (i = 1u; i < MAX_ACTORS; i++) {
        actor = actors + i;
        if (actor->reserve_tiles == effect_reserved_tiles &&
            actor->collision_group == COLLISION_GROUP_NONE) {
            return actor;
        }
    }

    return NULL;
}

static actor_t *magic_find_target(UBYTE target_mode, UBYTE collision_mask, UWORD range_subpx) {
    actor_t *candidate;
    actor_t *target = NULL;
    UWORD distance;
    UWORD best_distance = range_subpx + 1u;

    if (target_mode != MAGIC_TARGET_NEAREST_PLAYER) {
        return NULL;
    }

    for (candidate = actors_active_head; candidate; candidate = candidate->next) {
        if (!magic_is_targetable(candidate, collision_mask)) continue;
        distance = magic_distance_to_player(candidate);
        if (distance < best_distance) {
            best_distance = distance;
            target = candidate;
        }
    }

    return target;
}

static UBYTE magic_can_pay_cost(void) {
    if (magic_ingredient1_cost && (!magic_ingredient1 || *magic_ingredient1 < magic_ingredient1_cost)) return FALSE;
    if (magic_ingredient2_cost && (!magic_ingredient2 || *magic_ingredient2 < magic_ingredient2_cost)) return FALSE;
    return TRUE;
}

static void magic_pay_cost(void) {
    if (magic_ingredient1_cost && magic_ingredient1) *magic_ingredient1 -= magic_ingredient1_cost;
    if (magic_ingredient2_cost && magic_ingredient2) *magic_ingredient2 -= magic_ingredient2_cost;
}

static void magic_roll_damage(void) {
    UBYTE damage_range;

    if (!magic_damage_out) return;

    if (magic_max_damage < magic_min_damage) {
        magic_max_damage = magic_min_damage;
    }

    damage_range = (UBYTE)(magic_max_damage - magic_min_damage + 1u);
    *magic_damage_out = (UBYTE)(magic_min_damage + (rand() % damage_range));
}

static void magic_unfreeze_target(void) {
    if (magic_frozen_target &&
        CHK_FLAG(magic_frozen_target->flags, ACTOR_FLAG_ACTIVE) &&
        !CHK_FLAG(magic_frozen_target->flags, ACTOR_FLAG_HIDDEN)) {
        CLR_FLAG(magic_frozen_target->flags, ACTOR_FLAG_DISABLED);
    }
    magic_frozen_target = NULL;
    magic_freeze_timer = 0u;
}

static void magic_hide_effect(void) {
    if (magic_effect_actor) {
        SET_FLAG(magic_effect_actor->flags, ACTOR_FLAG_HIDDEN);
    }
    magic_effect_actor = NULL;
    magic_effect_timer = 0u;
}

static void magic_start_effect_at_target(actor_t *target) {
    if (!magic_effect_actor) return;

    magic_effect_actor->pos.x = target->pos.x;
    magic_effect_actor->pos.y = (target->pos.y > PX_TO_SUBPX(magic_effect_y_offset_px)) ?
        target->pos.y - PX_TO_SUBPX(magic_effect_y_offset_px) : 0u;
    magic_effect_actor->frame = 0u;
    magic_effect_actor->anim_tick = 3u;
    CLR_FLAG(magic_effect_actor->flags, ACTOR_FLAG_HIDDEN | ACTOR_FLAG_DISABLED);
}

static void magic_start_effect_in_front_of_player(void) {
    UWORD offset = PX_TO_SUBPX(16u);

    if (!magic_effect_actor) return;

    magic_effect_actor->pos.x = PLAYER.pos.x;
    magic_effect_actor->pos.y = PLAYER.pos.y;

    if (PLAYER.dir == DIR_LEFT) {
        magic_effect_actor->pos.x = (PLAYER.pos.x > offset) ? PLAYER.pos.x - offset : 0u;
    } else if (PLAYER.dir == DIR_RIGHT) {
        magic_effect_actor->pos.x = PLAYER.pos.x + offset;
    } else if (PLAYER.dir == DIR_UP) {
        magic_effect_actor->pos.y = (PLAYER.pos.y > offset) ? PLAYER.pos.y - offset : 0u;
    } else {
        magic_effect_actor->pos.y = PLAYER.pos.y + offset;
    }

    magic_effect_actor->frame = 0u;
    magic_effect_actor->anim_tick = 3u;
    CLR_FLAG(magic_effect_actor->flags, ACTOR_FLAG_HIDDEN | ACTOR_FLAG_DISABLED);
}

static void magic_freeze_pending_target(void) {
    if (!magic_freeze_frames || !magic_pending_target) return;

    magic_frozen_target = magic_pending_target;
    magic_freeze_timer = magic_freeze_frames;
    SET_FLAG(magic_pending_target->flags, ACTOR_FLAG_DISABLED);
}

static void magic_land_spell(void) {
    if (magic_pending_target &&
        CHK_FLAG(magic_pending_target->flags, ACTOR_FLAG_ACTIVE) &&
        !CHK_FLAG(magic_pending_target->flags, ACTOR_FLAG_HIDDEN)) {
        script_execute(magic_pending_target->script.bank, magic_pending_target->script.ptr,
                       &(magic_pending_target->hscript_hit), 1, (UWORD)magic_hit_collision_mask);
    }

    magic_pending_target = NULL;
    magic_state = MAGIC_STATE_IDLE;
    magic_hide_effect();
    magic_unfreeze_target();
}

static void magic_update_missile(void) {
    UWORD speed;
    UWORD dx;
    UWORD dy;

    if (magic_effect_timer) {
        magic_effect_timer--;
    } else {
        magic_land_spell();
        return;
    }

    if (!magic_effect_actor || !magic_pending_target ||
        !CHK_FLAG(magic_pending_target->flags, ACTOR_FLAG_ACTIVE) ||
        CHK_FLAG(magic_pending_target->flags, ACTOR_FLAG_HIDDEN)) {
        magic_land_spell();
        return;
    }

    speed = PX_TO_SUBPX(magic_missile_speed_px ? magic_missile_speed_px : 1u);
    dx = magic_axis_distance(magic_effect_actor->pos.x, magic_pending_target->pos.x);
    dy = magic_axis_distance(magic_effect_actor->pos.y, magic_pending_target->pos.y);

    if (dx <= speed && dy <= speed) {
        magic_land_spell();
        return;
    }

    if (dx >= dy) {
        if (magic_effect_actor->pos.x < magic_pending_target->pos.x) {
            magic_effect_actor->pos.x += speed;
        } else if (magic_effect_actor->pos.x > speed) {
            magic_effect_actor->pos.x -= speed;
        } else {
            magic_effect_actor->pos.x = 0u;
        }
    } else {
        if (magic_effect_actor->pos.y < magic_pending_target->pos.y) {
            magic_effect_actor->pos.y += speed;
        } else if (magic_effect_actor->pos.y > speed) {
            magic_effect_actor->pos.y -= speed;
        } else {
            magic_effect_actor->pos.y = 0u;
        }
    }
}

void magic_configure_targeting(SCRIPT_CTX *THIS) OLDCALL BANKED {
    magic_spell_id = *(UBYTE *)VM_REF_TO_PTR(FN_ARG0);
    magic_behavior = *(UBYTE *)VM_REF_TO_PTR(FN_ARG1);
    magic_target_mode = *(UBYTE *)VM_REF_TO_PTR(FN_ARG2);
    magic_range_px = *(UBYTE *)VM_REF_TO_PTR(FN_ARG3);
    magic_hit_collision_mask = *(UBYTE *)VM_REF_TO_PTR(FN_ARG4);

    THIS;
}

void magic_configure_damage(SCRIPT_CTX *THIS) OLDCALL BANKED {
    magic_damage_out = (UBYTE *)VM_REF_TO_PTR(FN_ARG0);
    magic_min_damage = *(UBYTE *)VM_REF_TO_PTR(FN_ARG1);
    magic_max_damage = *(UBYTE *)VM_REF_TO_PTR(FN_ARG2);
    magic_effect_reserved_tiles = *(UBYTE *)VM_REF_TO_PTR(FN_ARG3);

    THIS;
}

void magic_configure_cost(SCRIPT_CTX *THIS) OLDCALL BANKED {
    magic_ingredient1 = (UBYTE *)VM_REF_TO_PTR(FN_ARG0);
    magic_ingredient1_cost = *(UBYTE *)VM_REF_TO_PTR(FN_ARG1);
    magic_ingredient2 = (UBYTE *)VM_REF_TO_PTR(FN_ARG2);
    magic_ingredient2_cost = *(UBYTE *)VM_REF_TO_PTR(FN_ARG3);

    THIS;
}

void magic_configure_effect(SCRIPT_CTX *THIS) OLDCALL BANKED {
    magic_freeze_frames = *(UBYTE *)VM_REF_TO_PTR(FN_ARG0);
    magic_effect_frames = *(UBYTE *)VM_REF_TO_PTR(FN_ARG1);
    magic_missile_speed_px = *(UBYTE *)VM_REF_TO_PTR(FN_ARG2);
    magic_effect_y_offset_px = *(UBYTE *)VM_REF_TO_PTR(FN_ARG3);

    THIS;
}

void magic_cast_configured(SCRIPT_CTX *THIS) OLDCALL BANKED {
    if (magic_state != MAGIC_STATE_IDLE) {
        THIS;
        return;
    }

    magic_pending_target = magic_find_target(magic_target_mode, magic_hit_collision_mask, PX_TO_SUBPX(magic_range_px));
    if (!magic_pending_target || !magic_can_pay_cost()) {
        magic_pending_target = NULL;
        THIS;
        return;
    }

    magic_effect_actor = magic_find_effect_actor(magic_effect_reserved_tiles);
    if (!magic_effect_actor) {
        magic_pending_target = NULL;
        THIS;
        return;
    }

    magic_pay_cost();
    magic_roll_damage();

    if (magic_behavior == MAGIC_BEHAVIOR_MISSILE) {
        magic_state = MAGIC_STATE_MISSILE;
        magic_effect_timer = magic_effect_frames;
        magic_start_effect_in_front_of_player();
    } else {
        magic_state = MAGIC_STATE_BURST;
        magic_effect_timer = magic_effect_frames;
        magic_start_effect_at_target(magic_pending_target);
        magic_freeze_pending_target();
    }

    magic_spell_id;
    THIS;
}

void magic_system_update(void) BANKED {
    if (magic_state == MAGIC_STATE_IDLE) return;

    if (magic_state == MAGIC_STATE_MISSILE) {
        magic_update_missile();
    } else if (magic_effect_timer) {
        magic_effect_timer--;
        if (!magic_effect_timer) {
            magic_land_spell();
        }
    }

    if (magic_freeze_timer) {
        magic_freeze_timer--;
        if (!magic_freeze_timer) {
            magic_unfreeze_target();
        }
    }
}
