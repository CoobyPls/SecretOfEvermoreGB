#pragma bank 255

#include <rand.h>

#include "actor.h"
#include "collision.h"
#include "game_time.h"
#include "macro.h"
#include "math.h"
#include "projectiles.h"
#include "vm.h"
#include "data/game_globals.h"

#include "dog_companion.h"

#define DOG_STEP                    29u
#define DOG_KO_STEP                 13u
#define DOG_FOLLOW_DISTANCE         PX_TO_SUBPX(24u)
#define DOG_WARP_DISTANCE           PX_TO_SUBPX(48u)
#define DOG_NOTICE_DISTANCE         PX_TO_SUBPX(63u)
#define DOG_BITE_REACH              PX_TO_SUBPX(24u)
#define DOG_BITE_LANE               PX_TO_SUBPX(8u)
#define DOG_LEASH_DISTANCE          PX_TO_SUBPX(56u)
#define DOG_CONTACT_DISTANCE        PX_TO_SUBPX(12u)
#define DOG_BITE_COOLDOWN           60u
#define DOG_HURT_COOLDOWN           60u
#define DOG_RETREAT_TIME            22u
#define DOG_RETREAT_REST_TIME       60u
#define DOG_DECISION_DELAY          36u
#define DOG_WINDUP_TIME             2u
#define DOG_AXIS_HOLD_TIME          10u
#define DOG_DIAGONAL_BAND           PX_TO_SUBPX(8u)
#define SKEETO_ALERT_DISTANCE       PX_TO_SUBPX(32u)
#define ENEMY_AGGRO_STEP            16u

static actor_t *dog_actor;
static actor_t *dog_target;
static UBYTE dog_bite_cooldown;
static UBYTE dog_hurt_cooldown;
static UBYTE dog_retreat_time;
static UBYTE dog_retreat_rest_time;
static UBYTE dog_decision_delay;
static UBYTE dog_windup_time;
static UBYTE dog_follow_axis;
static UBYTE dog_follow_axis_time;
static UBYTE dog_attack_script_bank;
static UBYTE *dog_attack_script_ptr;

extern projectile_t *projectiles_active_head;

static void take_damage_from(actor_t *attacker);

static UWORD axis_distance(UWORD a, UWORD b) {
    return a > b ? a - b : b - a;
}

static UWORD dog_distance(actor_t *a, actor_t *b) {
    return axis_distance(a->pos.x, b->pos.x) + axis_distance(a->pos.y, b->pos.y);
}

static UBYTE dog_has_bite_line(actor_t *target) {
    UWORD dx = axis_distance(dog_actor->pos.x, target->pos.x);
    UWORD dy = axis_distance(dog_actor->pos.y, target->pos.y);

    return ((dx <= DOG_BITE_REACH) && (dy <= DOG_BITE_LANE)) ||
           ((dy <= DOG_BITE_REACH) && (dx <= DOG_BITE_LANE));
}

static UBYTE is_enemy(actor_t *actor) {
    if (!actor || actor == dog_actor || actor == &PLAYER) return FALSE;
    if (!CHK_FLAG(actor->flags, ACTOR_FLAG_ACTIVE) ||
        CHK_FLAG(actor->flags, ACTOR_FLAG_HIDDEN | ACTOR_FLAG_DISABLED)) return FALSE;
    return (actor->collision_group & (COLLISION_GROUP_1 | COLLISION_GROUP_2)) != 0u;
}

static UBYTE blocked_by_wall(actor_t *moving_actor, upoint16_t *next, direction_e dir) {
    UBYTE start;
    UBYTE end;
    UBYTE edge;

    if (dir == DIR_LEFT || dir == DIR_RIGHT) {
        start = SUBPX_TO_TILE(next->y + moving_actor->bounds.top);
        end = SUBPX_TO_TILE(next->y + moving_actor->bounds.bottom);
        edge = SUBPX_TO_TILE(next->x + (dir == DIR_LEFT ? moving_actor->bounds.left : moving_actor->bounds.right));
        return tile_col_test_range_y(dir == DIR_LEFT ? COLLISION_RIGHT : COLLISION_LEFT, edge, start, end) != 0u;
    }

    start = SUBPX_TO_TILE(next->x + moving_actor->bounds.left);
    end = SUBPX_TO_TILE(next->x + moving_actor->bounds.right);
    edge = SUBPX_TO_TILE(next->y + (dir == DIR_UP ? moving_actor->bounds.top : moving_actor->bounds.bottom));
    return tile_col_test_range_x(dir == DIR_UP ? COLLISION_BOTTOM : COLLISION_TOP, edge, start, end) != 0u;
}

static UBYTE try_step(direction_e dir, UBYTE step) {
    upoint16_t next = dog_actor->pos;
    actor_t *other;

    point_translate_dir(&next, dir, step);
    if (blocked_by_wall(dog_actor, &next, dir)) return FALSE;
    if (bb_intersects(&dog_actor->bounds, &next, &PLAYER.bounds, &PLAYER.pos)) return FALSE;
    other = actor_overlapping_bb(&dog_actor->bounds, &next, dog_actor);
    if (other && other != &PLAYER) return FALSE;

    dog_actor->pos = next;
    actor_set_dir(dog_actor, dir, TRUE);
    return TRUE;
}

static void move_relative_to(actor_t *other, UBYTE away, UBYTE step) {
    UWORD dx = axis_distance(dog_actor->pos.x, other->pos.x);
    UWORD dy = axis_distance(dog_actor->pos.y, other->pos.y);
    direction_e horizontal = dog_actor->pos.x < other->pos.x ? DIR_RIGHT : DIR_LEFT;
    direction_e vertical = dog_actor->pos.y < other->pos.y ? DIR_DOWN : DIR_UP;

    if (away) {
        horizontal = FLIPPED_DIR(horizontal);
        vertical = FLIPPED_DIR(vertical);
    }
    if (dx >= dy) {
        if (!try_step(horizontal, step)) try_step(vertical, step);
    } else {
        if (!try_step(vertical, step)) try_step(horizontal, step);
    }
}

static void move_toward_player(UBYTE step) {
    UWORD dx = axis_distance(dog_actor->pos.x, PLAYER.pos.x);
    UWORD dy = axis_distance(dog_actor->pos.y, PLAYER.pos.y);
    direction_e horizontal = dog_actor->pos.x < PLAYER.pos.x ? DIR_RIGHT : DIR_LEFT;
    direction_e vertical = dog_actor->pos.y < PLAYER.pos.y ? DIR_DOWN : DIR_UP;
    UBYTE horizontal_first;

    if (axis_distance(dx, dy) <= DOG_DIAGONAL_BAND) {
        if (!dog_follow_axis_time) {
            dog_follow_axis = (dx >= dy) ? 0u : 1u;
            dog_follow_axis_time = DOG_AXIS_HOLD_TIME;
        } else {
            dog_follow_axis_time--;
        }
        horizontal_first = (dog_follow_axis == 0u);
    } else {
        dog_follow_axis_time = 0u;
        horizontal_first = (dx >= dy);
    }

    if (horizontal_first) {
        if (!try_step(horizontal, step)) {
            try_step(vertical, step);
            dog_follow_axis = 1u;
            dog_follow_axis_time = 0u;
        }
    } else {
        if (!try_step(vertical, step)) {
            try_step(horizontal, step);
            dog_follow_axis = 0u;
            dog_follow_axis_time = 0u;
        }
    }
}

static UBYTE try_enemy_step(actor_t *enemy, actor_t *target, direction_e dir) {
    upoint16_t next = enemy->pos;
    actor_t *other;

    point_translate_dir(&next, dir, ENEMY_AGGRO_STEP);
    if (blocked_by_wall(enemy, &next, dir)) return FALSE;
    if (target == &PLAYER && bb_intersects(&enemy->bounds, &next, &PLAYER.bounds, &PLAYER.pos)) {
        player_register_collision_with(enemy);
        return FALSE;
    }
    if (target == dog_actor && bb_intersects(&enemy->bounds, &next, &dog_actor->bounds, &dog_actor->pos)) {
        if (!dog_hurt_cooldown) take_damage_from(enemy);
        return FALSE;
    }
    other = actor_overlapping_bb(&enemy->bounds, &next, enemy);
    if (other) return FALSE;

    enemy->pos = next;
    actor_set_dir(enemy, dir, TRUE);
    return TRUE;
}

static void move_enemy_toward(actor_t *enemy, actor_t *target) {
    UWORD dx = axis_distance(enemy->pos.x, target->pos.x);
    UWORD dy = axis_distance(enemy->pos.y, target->pos.y);
    direction_e horizontal = enemy->pos.x < target->pos.x ? DIR_RIGHT : DIR_LEFT;
    direction_e vertical = enemy->pos.y < target->pos.y ? DIR_DOWN : DIR_UP;

    if (dx >= dy) {
        if (!try_enemy_step(enemy, target, horizontal)) try_enemy_step(enemy, target, vertical);
    } else {
        if (!try_enemy_step(enemy, target, vertical)) try_enemy_step(enemy, target, horizontal);
    }
}

static void face_target(actor_t *target) {
    UWORD dx = axis_distance(dog_actor->pos.x, target->pos.x);
    UWORD dy = axis_distance(dog_actor->pos.y, target->pos.y);

    if (dx >= dy) {
        actor_set_dir(dog_actor, dog_actor->pos.x < target->pos.x ? DIR_RIGHT : DIR_LEFT, FALSE);
    } else {
        actor_set_dir(dog_actor, dog_actor->pos.y < target->pos.y ? DIR_DOWN : DIR_UP, FALSE);
    }
}

static void approach_bite_line(actor_t *target) {
    UWORD dx = axis_distance(dog_actor->pos.x, target->pos.x);
    UWORD dy = axis_distance(dog_actor->pos.y, target->pos.y);
    direction_e horizontal = dog_actor->pos.x < target->pos.x ? DIR_RIGHT : DIR_LEFT;
    direction_e vertical = dog_actor->pos.y < target->pos.y ? DIR_DOWN : DIR_UP;

    if ((dx <= DOG_BITE_REACH) && (dy > DOG_BITE_LANE)) {
        if (!try_step(vertical, DOG_STEP)) {
            dog_target = NULL;
            dog_retreat_rest_time = 20u;
            actor_set_anim_idle(dog_actor);
        }
    } else if ((dy <= DOG_BITE_REACH) && (dx > DOG_BITE_LANE)) {
        if (!try_step(horizontal, DOG_STEP)) {
            dog_target = NULL;
            dog_retreat_rest_time = 20u;
            actor_set_anim_idle(dog_actor);
        }
    } else {
        move_relative_to(target, FALSE, DOG_STEP);
    }
}

static UBYTE try_place_near_player(direction_e dir, UBYTE distance) {
    upoint16_t next = PLAYER.pos;
    actor_t *other;

    point_translate_dir(&next, dir, PX_TO_SUBPX(distance));
    if (blocked_by_wall(dog_actor, &next, dir)) return FALSE;
    if (bb_intersects(&dog_actor->bounds, &next, &PLAYER.bounds, &PLAYER.pos)) return FALSE;
    other = actor_overlapping_bb(&dog_actor->bounds, &next, dog_actor);
    if (other && other != &PLAYER) return FALSE;
    dog_actor->pos = next;
    actor_set_anim_idle(dog_actor);
    return TRUE;
}

static UBYTE keep_dog_near_player(void) {
    if (dog_distance(dog_actor, &PLAYER) <= DOG_WARP_DISTANCE) return FALSE;
    if (try_place_near_player(DIR_LEFT, 24u)) return TRUE;
    if (try_place_near_player(DIR_RIGHT, 24u)) return TRUE;
    if (try_place_near_player(DIR_UP, 24u)) return TRUE;
    if (try_place_near_player(DIR_DOWN, 24u)) return TRUE;
    if (try_place_near_player(DIR_LEFT, 16u)) return TRUE;
    if (try_place_near_player(DIR_RIGHT, 16u)) return TRUE;
    if (try_place_near_player(DIR_UP, 16u)) return TRUE;
    return try_place_near_player(DIR_DOWN, 16u);
}

static actor_t *pick_target(void) {
    actor_t *candidate;
    actor_t *picked = NULL;
    UBYTE choices = 0u;

    for (candidate = actors_active_head; candidate; candidate = candidate->next) {
        if (!is_enemy(candidate)) continue;
        if (dog_distance(dog_actor, candidate) > DOG_NOTICE_DISTANCE) continue;
        if (dog_distance(&PLAYER, candidate) > DOG_LEASH_DISTANCE) continue;
        choices++;
        if ((rand() % choices) == 0u) picked = candidate;
    }
    return picked;
}

static void take_damage_from(actor_t *attacker) {
    if (VM_GLOBAL(VAR_DOGHP) > 0u) VM_GLOBAL(VAR_DOGHP)--;
    dog_hurt_cooldown = DOG_HURT_COOLDOWN;
    dog_retreat_time = DOG_RETREAT_TIME;
    dog_windup_time = 0u;
    dog_target = attacker;
}

static void check_contact_damage(void) {
    actor_t *enemy;
    projectile_t *projectile;

    if (dog_hurt_cooldown) {
        dog_hurt_cooldown--;
        return;
    }
    for (enemy = actors_active_head; enemy; enemy = enemy->next) {
        if (is_enemy(enemy) && dog_distance(dog_actor, enemy) <= DOG_CONTACT_DISTANCE) {
            take_damage_from(enemy);
            return;
        }
    }
    for (projectile = projectiles_active_head; projectile; projectile = projectile->next) {
        if (projectile->def.collision_mask == COLLISION_GROUP_PLAYER &&
            bb_intersects(&projectile->def.bounds, &projectile->pos, &dog_actor->bounds, &dog_actor->pos)) {
            take_damage_from(NULL);
            return;
        }
    }
}

void dog_companion_update(SCRIPT_CTX *THIS) OLDCALL BANKED {
    UBYTE actor_index = *(UBYTE *)VM_REF_TO_PTR(FN_ARG0);
    UBYTE attack_script_bank = *(UBYTE *)VM_REF_TO_PTR(FN_ARG1);
    UBYTE *attack_script_ptr = *(UBYTE **)VM_REF_TO_PTR(FN_ARG2);
    actor_t *current_dog = actors + actor_index;
    UBYTE new_dog_actor = current_dog != dog_actor;

    dog_attack_script_bank = attack_script_bank;
    dog_attack_script_ptr = attack_script_ptr;
    if (new_dog_actor) {
        dog_actor = current_dog;
        dog_target = NULL;
        dog_bite_cooldown = dog_hurt_cooldown = dog_retreat_time = dog_retreat_rest_time = dog_decision_delay = dog_windup_time = 0u;
        dog_follow_axis = dog_follow_axis_time = 0u;
    }
    if (!VM_GLOBAL(VAR_DOGJOINED) || CHK_FLAG(dog_actor->flags, ACTOR_FLAG_HIDDEN | ACTOR_FLAG_DISABLED)) return;

    if (VM_GLOBAL(VAR_DOGHP) == 0u) {
        dog_target = NULL;
        dog_retreat_time = dog_retreat_rest_time = dog_windup_time = 0u;
        if (!keep_dog_near_player() && dog_distance(dog_actor, &PLAYER) > DOG_FOLLOW_DISTANCE) {
            move_toward_player(DOG_KO_STEP);
        } else {
            actor_set_anim_idle(dog_actor);
        }
        return;
    }

    if (keep_dog_near_player()) return;
    if (dog_bite_cooldown) dog_bite_cooldown--;
    if (dog_decision_delay) dog_decision_delay--;
    check_contact_damage();
    if (VM_GLOBAL(VAR_DOGHP) == 0u) {
        dog_target = NULL;
        actor_set_anim_idle(dog_actor);
        return;
    }

    if (dog_retreat_time && dog_target && is_enemy(dog_target)) {
        dog_retreat_time--;
        move_relative_to(dog_target, TRUE, DOG_STEP);
        if (!dog_retreat_time) {
            dog_retreat_rest_time = DOG_RETREAT_REST_TIME;
            dog_target = NULL;
        }
        return;
    }
    dog_retreat_time = 0u;
    if (dog_retreat_rest_time) {
        dog_retreat_rest_time--;
        actor_set_anim_idle(dog_actor);
        return;
    }

    if (!is_enemy(dog_target) ||
        dog_distance(&PLAYER, dog_target) > DOG_LEASH_DISTANCE ||
        dog_distance(dog_actor, dog_target) > DOG_NOTICE_DISTANCE) {
        dog_target = NULL;
    }

    if (!dog_target && !dog_decision_delay) {
        dog_decision_delay = DOG_DECISION_DELAY;
        if ((rand() & 0x03u) != 0u) dog_target = pick_target();
    }

    if (dog_target) {
        if (dog_has_bite_line(dog_target)) {
            face_target(dog_target);
            if (!dog_bite_cooldown && dog_target->script.bank &&
                (dog_target->hscript_hit & SCRIPT_TERMINATED)) {
                if (!dog_windup_time) {
                    dog_windup_time = DOG_WINDUP_TIME;
                    if (dog_attack_script_bank && dog_attack_script_ptr) {
                        script_execute(dog_attack_script_bank, dog_attack_script_ptr, 0, 0);
                    }
                } else if (!--dog_windup_time) {
                    script_execute(dog_target->script.bank, dog_target->script.ptr,
                                   &(dog_target->hscript_hit), 1, (UWORD)COLLISION_GROUP_3);
                    dog_bite_cooldown = DOG_BITE_COOLDOWN;
                    dog_retreat_time = DOG_RETREAT_TIME;
                }
            }
            actor_set_anim_idle(dog_actor);
        } else {
            dog_windup_time = 0u;
            approach_bite_line(dog_target);
        }
        return;
    }

    if (dog_distance(dog_actor, &PLAYER) > DOG_FOLLOW_DISTANCE) {
        move_toward_player(DOG_STEP);
    } else {
        actor_set_anim_idle(dog_actor);
    }
}

void dog_enemy_aggro_update(SCRIPT_CTX *THIS) OLDCALL BANKED {
    UBYTE actor_index = *(UBYTE *)VM_REF_TO_PTR(FN_ARG0);
    actor_t *enemy = actors + actor_index;
    actor_t *target = &PLAYER;
    UWORD player_distance;
    UWORD companion_distance;

    if (!is_enemy(enemy)) return;
    player_distance = dog_distance(enemy, &PLAYER);
    companion_distance = SKEETO_ALERT_DISTANCE + 1u;
    if (dog_actor && VM_GLOBAL(VAR_DOGJOINED) && VM_GLOBAL(VAR_DOGHP)) {
        companion_distance = dog_distance(enemy, dog_actor);
    }
    if (player_distance > SKEETO_ALERT_DISTANCE && companion_distance > SKEETO_ALERT_DISTANCE) return;
    if (companion_distance < player_distance) target = dog_actor;
    if ((game_time & 0x01u) == (actor_index & 0x01u)) {
        move_enemy_toward(enemy, target);
    }
    THIS;
}

void new_game_reset_state(SCRIPT_CTX *THIS) OLDCALL BANKED {
    UBYTE i;

    for (i = 0u; i != MAX_GLOBAL_VARS; ++i) {
        VM_GLOBAL(i) = 0;
    }
    VM_GLOBAL(VAR_PLAYERHP) = 10u;
    VM_GLOBAL(VAR_PLAYERMAXHP) = 10u;
    VM_GLOBAL(VAR_CURRENTDAMAGE) = 1u;
    VM_GLOBAL(VAR_PITRAPTORHP) = 4u;
    VM_GLOBAL(VAR_DOGHP) = 10u;
    VM_GLOBAL(VAR_DOGMAXHP) = 10u;

    dog_actor = NULL;
    dog_target = NULL;
    dog_bite_cooldown = dog_hurt_cooldown = dog_retreat_time = 0u;
    dog_retreat_rest_time = dog_decision_delay = dog_windup_time = 0u;
    dog_follow_axis = dog_follow_axis_time = 0u;
    THIS;
}
