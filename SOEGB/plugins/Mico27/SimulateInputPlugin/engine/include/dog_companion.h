#ifndef DOG_COMPANION_H
#define DOG_COMPANION_H

#include "vm.h"

void dog_companion_update(SCRIPT_CTX *THIS) OLDCALL BANKED;
void dog_enemy_aggro_update(SCRIPT_CTX *THIS) OLDCALL BANKED;
void new_game_reset_state(SCRIPT_CTX *THIS) OLDCALL BANKED;

#endif
