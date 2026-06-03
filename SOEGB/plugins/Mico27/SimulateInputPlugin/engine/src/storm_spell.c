#pragma bank 255

#include <gbdk/platform.h>
#include <rand.h>

#include "actor.h"
#include "collision.h"
#include "dog_companion.h"
#include "macro.h"
#include "vm.h"
#include "data/game_globals.h"

#define STORM_MAX_RANGE PX_TO_SUBPX(112u)
#define STORM_FREEZE_FRAMES 30u
#define STORM_EFFECT_RESERVED_TILES 23u
#define STORM_EFFECT_FRAMES 44u

static actor_t *storm_frozen_target;
static actor_t *storm_effect_actor;
static UBYTE storm_freeze_timer;
static UBYTE storm_effect_timer;

static UWORD storm_axis_distance(UWORD a, UWORD b) {
    return (a > b) ? (a - b) : (b - a);
}

static UWORD storm_distance_to_player(actor_t *actor) {
    return storm_axis_distance(actor->pos.x, PLAYER.pos.x) +
           storm_axis_distance(actor->pos.y, PLAYER.pos.y);
}

static UBYTE storm_is_targetable(actor_t *actor) {
    if (!actor || actor == &PLAYER) return FALSE;
    if (dog_companion_is_actor(actor)) return FALSE;
    if (!CHK_FLAG(actor->flags, ACTOR_FLAG_ACTIVE) ||
        CHK_FLAG(actor->flags, ACTOR_FLAG_HIDDEN | ACTOR_FLAG_DISABLED)) return FALSE;
    if (!(actor->collision_group & (COLLISION_GROUP_1 | COLLISION_GROUP_2))) return FALSE;
    if (!actor->script.bank || !(actor->hscript_hit & SCRIPT_TERMINATED)) return FALSE;
    return TRUE;
}

static actor_t *storm_find_effect_actor(void) {
    UBYTE i;
    actor_t *actor;

    for (i = 1u; i < MAX_ACTORS; i++) {
        actor = actors + i;
        if (actor->reserve_tiles == STORM_EFFECT_RESERVED_TILES &&
            actor->collision_group == COLLISION_GROUP_NONE) {
            return actor;
        }
    }

    return NULL;
}

static void storm_show_effect(actor_t *target) {
    actor_t *effect = storm_find_effect_actor();

    if (!effect) return;

    storm_effect_actor = effect;
    storm_effect_timer = STORM_EFFECT_FRAMES;

    effect->pos.x = target->pos.x;
    effect->pos.y = (target->pos.y > PX_TO_SUBPX(24u)) ? target->pos.y - PX_TO_SUBPX(24u) : 0u;
    effect->frame = 0u;
    effect->anim_tick = 3u;
    actor_set_frames(effect, 0u, 11u);
    CLR_FLAG(effect->flags, ACTOR_FLAG_HIDDEN | ACTOR_FLAG_DISABLED);
}

void storm_cast_nearest(SCRIPT_CTX *THIS) OLDCALL BANKED {
    actor_t *candidate;
    actor_t *target = NULL;
    UWORD distance;
    UWORD best_distance = STORM_MAX_RANGE + 1u;

    for (candidate = actors_active_head; candidate; candidate = candidate->next) {
        if (!storm_is_targetable(candidate)) continue;
        distance = storm_distance_to_player(candidate);
        if (distance < best_distance) {
            best_distance = distance;
            target = candidate;
        }
    }

    if (target) {
        storm_show_effect(target);
        storm_frozen_target = target;
        storm_freeze_timer = STORM_FREEZE_FRAMES;
        SET_FLAG(target->flags, ACTOR_FLAG_DISABLED);
        VM_GLOBAL(VAR_CURRENTDAMAGE) = (UBYTE)(1u + (rand() & 1u));
        script_execute(target->script.bank, target->script.ptr,
                       &(target->hscript_hit), 1, (UWORD)COLLISION_GROUP_3);
    }

    THIS;
}

void storm_spell_update(void) BANKED {
    if (storm_effect_timer) {
        storm_effect_timer--;
        if (!storm_effect_timer && storm_effect_actor) {
            SET_FLAG(storm_effect_actor->flags, ACTOR_FLAG_HIDDEN);
            storm_effect_actor = NULL;
        }
    }

    if (!storm_freeze_timer) return;

    storm_freeze_timer--;
    if (!storm_freeze_timer && storm_frozen_target) {
        CLR_FLAG(storm_frozen_target->flags, ACTOR_FLAG_DISABLED);
        storm_frozen_target = NULL;
    }
}
