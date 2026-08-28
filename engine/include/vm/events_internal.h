// FILENAME: events_internal.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (events.c, events_alarm.c, events_poll.c, events_trap.c)
// NEEDS: libcore, libengine, libkernel, libplatform, libserver
// Implements bytecode virtual machine execution and state for events_internal.
//
// ---- Includes ----

#ifndef EVENTS_INTERNAL_H
#define EVENTS_INTERNAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/ctype/ctype.h"
#include "runtime/format/snprintf.h"
#include "hal/hal.h"


#include "core/struct.h"
#include "debug/logger.h"
#include "device/bus.h"
#include "device/usb.h"
#include "device/vcon.h"
#include "device/vdev.h"
#include "eval/eval.h"
#include "memory/segmented_mem.h"
#include "platform/platform.h"
#include "runtime/file.h"
#include "runtime/metadata.h"
#include "runtime/spec.h"
#include "runtime/variables.h"
#include "runtime/vfs.h"
#include "runtime/vnet.h"
#include "security/security.h"
#include "stmt/stmt.h"
#include "types/config.h"
#include "types/errors.h"
#include "types/types.h"
#include "vm/vm.h"
#include "vm/vm_internal.h"

//
// ---- Internal Helpers ----

double platform_get_timer(void);
double platform_get_uptime(void);
int platform_inkey_char(void);
#ifndef BASIC_LITE_BUILD
int vdev_music_note_count(void);
#endif

bool parse_time_str(const char *time_str, int *out_secs);
bool grow_alarms(VMContext *vm);
bool grow_alarms_str(VMContext *vm);

#endif // EVENTS_INTERNAL_H
