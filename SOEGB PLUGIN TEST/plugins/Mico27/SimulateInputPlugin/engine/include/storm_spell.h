#ifndef STORM_SPELL_H
#define STORM_SPELL_H

#include <gbdk/platform.h>

#include "vm.h"

void storm_cast_nearest(SCRIPT_CTX *THIS) OLDCALL BANKED;
void storm_spell_update(void) BANKED;

#endif
