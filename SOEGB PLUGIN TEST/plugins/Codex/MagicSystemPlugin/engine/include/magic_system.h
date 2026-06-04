#ifndef MAGIC_SYSTEM_H
#define MAGIC_SYSTEM_H

#include <gbdk/platform.h>
#include "vm.h"

void magic_configure_targeting(SCRIPT_CTX *THIS) OLDCALL BANKED;
void magic_configure_damage(SCRIPT_CTX *THIS) OLDCALL BANKED;
void magic_configure_cost(SCRIPT_CTX *THIS) OLDCALL BANKED;
void magic_configure_effect(SCRIPT_CTX *THIS) OLDCALL BANKED;
void magic_configure_output_vars(SCRIPT_CTX *THIS) OLDCALL BANKED;
void magic_cast_configured(SCRIPT_CTX *THIS) OLDCALL BANKED;
void magic_system_update(SCRIPT_CTX *THIS) OLDCALL BANKED;

#endif
