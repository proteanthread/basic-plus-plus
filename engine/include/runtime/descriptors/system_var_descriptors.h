// FILENAME: system_var_descriptors.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore, libengine, libflex
// NEEDS: libcore (language_descriptor.h)
// Declares registration interface for built-in system variable descriptors.

#ifndef SYSTEM_VAR_DESCRIPTORS_H
#define SYSTEM_VAR_DESCRIPTORS_H

#include "runtime/language_descriptor.h"

#ifdef __cplusplus
extern "C" {
#endif

// Registers all built-in OS, environment, clock, and diagnostic system variable descriptors.
void system_var_descriptors_register(void);

#ifdef __cplusplus
}
#endif

#endif // SYSTEM_VAR_DESCRIPTORS_H
