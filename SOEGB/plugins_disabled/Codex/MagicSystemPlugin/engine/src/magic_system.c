#pragma bank 255

#include <gbdk/platform.h>
#include <rand.h>

#include "actor.h"
#include "collision.h"
#include "macro.h"
#include "vm.h"
#include "magic_system.h"

#define MAGIC_TARGET_NEAREST_PLAYER 0u

static actor_t *magic_frozen_target;
static actor_t *magic_effect_actor;
static UBYTE magic_freeze_timer;
static UBYTE magic_effect_timer;
static UBYTE magic_spell_id;
static UBYTE magic_target_mode;
static UBYTE magic_range_px;
static UBYTE magic_hit_collision_mask;
static UBYTE *magic_damage_out;
static UBYTE magic_min_damage;
static UBYTE magic_max_damage;
static UBYTE magic_freeze_frames;
static UBYTE magic_effect_reserved_tiles;
static UBYTE magic_effect_frames;
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

static void magic_show_effect(actor_t *target, UBYTE effect_reserved_tiles, UBYTE effect_frames, UBYTE y_offset_px) {
    actor_t *effect;

    if (!effect_frames) return;

    effect = magic_find_effect_actor(effect_reserved_tiles);
    if (!effect) return;

    magic_effect_actor = effect;
    magic_effect_timer = effect_frames;

    effect->pos.x = target->pos.x;
    effect->pos.y = (target->pos.y > PX_TO_SUBPX(y_offset_px)) ? target->pos.y - PX_TO_SUBPX(y_offset_px) : 0u;
    effect->frame = 0u;
    effect->anim_tick = 3u;
    CLR_FLAG(effect->flags, ACTOR_FLAG_HIDDEN | ACTOR_FLAG_DISABLED);
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

void magic_configure_targeting(SCRIPT_CTX *THIS) OLDCALL BANKED {
    magic_spell_id = *(UBYTE *)VM_REF_TO_PTR(FN_ARG0);
    magic_target_mode = *(UBYTE *)VM_REF_TO_PTR(FN_ARG1);
    magic_range_px = *(UBYTE *)VM_REF_TO_PTR(FN_ARG2);
    magic_hit_collision_mask = *(UBYTE *)VM_REF_TO_PTR(FN_ARG3);

    THIS;
}

void magic_configure_damage(SCRIPT_CTX *THIS) OLDCALL BANKED {
    magic_damage_out = (UBYTE *)VM_REF_TO_PTR(FN_ARG0);
    magic_min_damage = *(UBYTE *)VM_REF_TO_PTR(FN_ARG1);
    magic_max_damage = *(UBYTE *)VM_REF_TO_PTR(FN_ARG2);
    magic_effect_reserved_tiles = *(UBYTE *)VM_REF_TO_PTR(FN_ARG3);

    THIS;
}

void magic_configure_effect(SCRIPT_CTX *THIS) OLDCALL BANKED {
    magic_freeze_frames = *(UBYTE *)VM_REF_TO_PTR(FN_ARG0);
    magic_effect_frames = *(UBYTE *)VM_REF_TO_PTR(FN_ARG1);
    magic_effect_y_offset_px = *(UBYTE *)VM_REF_TO_PTR(FN_ARG2);

    THIS;
}

void magic_cast_configured(SCRIPT_CTX *THIS) OLDCALL BANKED {
    actor_t *target = magic_find_target(magic_target_mode, magic_hit_collision_mask, PX_TO_SUBPX(magic_range_px));
    UBYTE damage_range;

    if (target) {
        if (magic_max_damage < magic_min_damage) {
            magic_max_damage = magic_min_damage;
        }

        damage_range = (UBYTE)(magic_max_damage - magic_min_damage + 1u);
        if (magic_damage_out) {
            *magic_damage_out = (UBYTE)(magic_min_damage + (rand() % damage_range));
        }

        magic_show_effect(target, magic_effect_reserved_tiles, magic_effect_frames, magic_effect_y_offset_px);

        if (magic_freeze_frames) {
            magic_frozen_target = target;
            magic_freeze_timer = magic_freeze_frames;
            SET_FLAG(target->flags, ACTOR_FLAG_DISABLED);
        }

        script_execute(target->script.bank, target->script.ptr,
                       &(target->hscript_hit), 1, (UWORD)magic_hit_collision_mask);
    }

    magic_spell_id;
    THIS;
}

void magic_system_update(void) BANKED {
    if (magic_effect_timer) {
        magic_effect_timer--;
        if (!magic_effect_timer && magic_effect_actor) {
            SET_FLAG(magic_effect_actor->flags, ACTOR_FLAG_HIDDEN);
            magic_effect_actor = NULL;
        }
    }

    if (magic_freeze_timer) {
        magic_freeze_timer--;
        if (!magic_freeze_timer && magic_frozen_target) {
            CLR_FLAG(magic_frozen_target->flags, ACTOR_FLAG_DISABLED);
            magic_frozen_target = NULL;
        }
    }
}
