// FILENAME: common_internal.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: 
// NEEDS: libboot, libcore, libengine, libkernel, libplatform
// Provides core logic and interface definitions for common_internal within BASIC++.
//
// ---- Includes ----

#ifndef BOOTSTRAP_COMMON_INTERNAL_H
#define BOOTSTRAP_COMMON_INTERNAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "runtime/ctype/ctype.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/memory/alloc.h"
#include "runtime/conv/num_parse.h"
#include "runtime/conv/float_parse.h"
#include "runtime/format/snprintf.h"
#include "hal/hal.h"

#include "core/boot.h"
#include "core/feature_reg.h"
#include "device/fujinet.h"
#include "device/vdev.h"
#include "device/vprinter.h"
#include "lexer/lexer.h"
#include "memory/memory.h"
#include "platform/platform.h"
#include "runtime/funcreg.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/spec.h"
#include "runtime/strings.h"
#include "runtime/variables.h"
#include "security/security.h"
#include "types/config.h"
#include "vm/vm.h"

//
// ---- Registration Interfaces ----

void boot_register_all_statements(void);
void boot_register_all_functions(void);

#endif // BOOTSTRAP_COMMON_INTERNAL_H
