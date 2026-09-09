// FILENAME: builtin_descriptors.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore, libboot
// Provides interface for built-in/intrinsic statement and function language descriptors.

#ifndef RUNTIME_DESCRIPTORS_BUILTIN_DESCRIPTORS_H
#define RUNTIME_DESCRIPTORS_BUILTIN_DESCRIPTORS_H

#include "runtime/language_descriptor.h"

#ifdef __cplusplus
extern "C" {
#endif

// @brief Registers LanguageDescriptor records for all built-in/intrinsic statements and functions.
void builtin_descriptors_register(void);

#ifdef __cplusplus
}
#endif

#endif // RUNTIME_DESCRIPTORS_BUILTIN_DESCRIPTORS_H
