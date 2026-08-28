// FILENAME: module.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_internal.h, help.c, module.c, system.c)
// NEEDS: libkernel (errors.h, security.h, security.c, types.h)
// Provides core logic and interface definitions for module within BASIC++.
//
// ---- Includes ----

#ifndef MODULE_H
#define MODULE_H

#include "security/security.h"
#include "types/errors.h"
#include "types/types.h"

#define MAX_MODULES 16

typedef enum {
    MOD_LIBRARY = 0,
    MOD_DIALECT = 1,
    MOD_DEVICE = 2,
    MOD_EXTENSION = 3
} BppModuleClass;

#define CAP_NONE      0x0000u
#define CAP_MATH      0x0001u
#define CAP_STRING    0x0002u
#define CAP_IO        0x0004u
#define CAP_FILE      0x0008u
#define CAP_SYSTEM    0x0010u
#define CAP_GRAPHICS  0x0020u
#define CAP_SOUND     0x0040u
#define CAP_NETWORK   0x0080u
#define CAP_GPIO      0x0100u
#define CAP_I2C       0x0200u
#define CAP_SPI       0x0400u
#define CAP_SENSOR    0x0800u
#define CAP_CAMERA    0x1000u
#define CAP_BLUETOOTH 0x2000u
#define CAP_USB       0x4000u

typedef struct {
    const char *name;
    const char *version;
    const char *description;
    BppModuleClass mod_class;
    unsigned int capabilities;
    BppSecLevel required_level;
    int (*init)(void *);
    void (*cleanup)(void);
} BppModuleInfo;

void                 module_system_init(void);
int                  module_count(void);
int                  module_register(const BppModuleInfo *info);
int                  module_activate(const char *name, void *rt);
int                  module_deactivate(const char *name);
int                  module_is_active(const char *name);
const BppModuleInfo *module_find(const char *name);
const BppModuleInfo *module_get(int index);
int                  module_is_loaded(int index);
const char          *module_class_name(BppModuleClass cls);
void                 module_caps_string(unsigned int caps, char *buf, int buf_len);
struct VMContext;
int                  module_load_dynamic(struct VMContext *vm, const char *path);
BppError             vm_load_library_file(struct VMContext *vm, const char *filename);

#endif // MODULE_H
