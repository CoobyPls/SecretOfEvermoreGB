#ifndef MENU_PAUSE_SNAPSHOT_H
#define MENU_PAUSE_SNAPSHOT_H

#include <gbdk/platform.h>
#include "vm.h"

extern UBYTE menu_pause_restore_pending;

void menu_pause_capture(SCRIPT_CTX * THIS) OLDCALL BANKED;
void menu_pause_request_restore(SCRIPT_CTX * THIS) OLDCALL BANKED;
void menu_pause_restore(void) BANKED;

#endif
