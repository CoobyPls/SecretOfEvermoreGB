#pragma bank 255

#include "actor.h"
#include "gameplay_interact.h"

static UBYTE actor_can_interact(actor_t *actor) {
    return actor && !(actor->collision_group & COLLISION_GROUP_MASK) && actor->script.bank;
}

static actor_t *actor_with_script_near_player(UBYTE margin) {
    actor_t *actor = PLAYER.prev;
    WORD margin_subpx = PX_TO_SUBPX(margin);

    WORD a_left = (WORD)(PLAYER.pos.x + PLAYER.bounds.left) - margin_subpx;
    WORD a_right = (WORD)(PLAYER.pos.x + PLAYER.bounds.right) + margin_subpx;
    WORD a_top = (WORD)(PLAYER.pos.y + PLAYER.bounds.top) - margin_subpx;
    WORD a_bottom = (WORD)(PLAYER.pos.y + PLAYER.bounds.bottom) + margin_subpx;

    while (actor) {
        if (!actor_can_interact(actor)) {
            actor = actor->prev;
            continue;
        }

        if ((WORD)(actor->pos.x + actor->bounds.left) > a_right) {
            actor = actor->prev;
            continue;
        }
        if ((WORD)(actor->pos.x + actor->bounds.right) < a_left) {
            actor = actor->prev;
            continue;
        }
        if ((WORD)(actor->pos.y + actor->bounds.top) > a_bottom) {
            actor = actor->prev;
            continue;
        }
        if ((WORD)(actor->pos.y + actor->bounds.bottom) < a_top) {
            actor = actor->prev;
            continue;
        }

        return actor;
    }

    return NULL;
}

void gameplay_try_interact(SCRIPT_CTX * THIS) OLDCALL BANKED {
    UWORD *interacted = VM_REF_TO_PTR(FN_ARG0);
    actor_t *hit_actor = actor_with_script_in_front_of_player(8);

    if (!actor_can_interact(hit_actor)) {
        hit_actor = actor_with_script_in_front_of_player(16);
    }

    if (!actor_can_interact(hit_actor)) {
        hit_actor = actor_with_script_in_front_of_player(24);
    }

    if (!actor_can_interact(hit_actor)) {
        hit_actor = actor_with_script_near_player(12);
    }

    *interacted = FALSE;
    if (actor_can_interact(hit_actor)) {
        actor_set_dir(hit_actor, FLIPPED_DIR(PLAYER.dir), FALSE);
        script_execute(hit_actor->script.bank, hit_actor->script.ptr, 0, 1, 0);
        *interacted = TRUE;
    }
}
