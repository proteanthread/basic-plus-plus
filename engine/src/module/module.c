/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file module.c
 * @brief Module System manager implementation.
 *
 * SECTION 1: WHAT IT DOES
 * - Manages module slots, activations, deactivations, capability checks,
 *   and dynamic library linking.
 *
 * SECTION 2: WHY IT EXISTS
 * - Provides modular vocabulary extensions securely, checking them against
 *   the security sandbox configuration before initialization.
 *
 * SECTION 3: WHY IT WORKS THIS WAY
 * - It manages a static module slots registry. Activations verify capabilities
 *   against active security settings, and dynamic loaders bind target libraries.
 *
 * SECTION 4: WHAT CAN BE CHANGED
 * - Max modules limit, slot mapping structures, or dynamic library suffix formats.
 *
 * SECTION 5: WHAT CANNOT BE CHANGED
 * - Sandbox verification rules or symbol init signatures.
 *
 * SECTION 6: WHAT TO EXPECT
 * - O(N) slot scanning during lookup, activation, and loading.
 *
 * SECTION 7: WHAT TO DO IF SOMETHING BREAKS
 * - Verify security sandbox levels and check dynamic loader path formats.
 *
 * SECTION 8: ASSUMPTIONS
 * - The runtime target is passed correctly as a VMContext pointer.
 *
 * SECTION 9: PORTABILITY CONCERNS
 * - Platform-specific dynamic linking is isolated. Standard C17.
 *
 * SECTION 10: FUTURE EXPANSIONS
 * - Adding hot-reloaded modules or metadata-driven capabilities verification.
 *
 * SECTION 11: EXTERNAL EXTENSION HOOKS
 * - Dynamic modules expose module_init entry points to hook extensions.
 */

#include "module/module.h"
#include "security/security.h"
#include "runtime/funcreg.h"
#include "vm/vm.h"
#include "device/vdev.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "platform/platform.h"

typedef struct {
    BppModuleInfo info;
    int           active;
    int           occupied;
} ModuleSlot;

static ModuleSlot module_table[MAX_MODULES];
static int        module_table_count = 0;

static int str_iequal(const char *a, const char *b) {
    if (!a || !b) return 0;
    while (*a && *b) {
        if (toupper((unsigned char)*a) != toupper((unsigned char)*b)) {
            return 0;
        }
        a++;
        b++;
    }
    return (*a == '\0' && *b == '\0');
}

void module_system_init(void) {
    memset(module_table, 0, sizeof(module_table));
    module_table_count = 0;
}

int module_count(void) {
    return module_table_count;
}

int module_register(const BppModuleInfo *info) {
    if (!info || !info->name) return -1;

    if (module_table_count >= MAX_MODULES) {
        return -1;
    }

    /* Check for duplicate name */
    for (int i = 0; i < module_table_count; ++i) {
        if (module_table[i].occupied && str_iequal(module_table[i].info.name, info->name)) {
            return 0; /* Already registered */
        }
    }

    module_table[module_table_count].info = *info;
    module_table[module_table_count].active = 0;
    module_table[module_table_count].occupied = 1;
    module_table_count++;

    return 0;
}

int module_activate(const char *name, void *rt) {
    if (!name) return -1;

    for (int i = 0; i < module_table_count; ++i) {
        if (module_table[i].occupied && str_iequal(module_table[i].info.name, name)) {
            if (module_table[i].active) {
                return 0; /* Idempotent */
            }

            /* Verify required pinned security level */
            if (!security_check_pinned_level(module_table[i].info.required_level)) {
                if (rt) {
                    vdev_printf(vm_get_vdev((VMContext *)rt), "?Error: Module '%s' blocked by required security level pinning %s\n",
                                module_table[i].info.name,
                                security_level_name(module_table[i].info.required_level));
                } else {
                    printf("?Error: Module '%s' blocked by required security level pinning %s\n",
                           module_table[i].info.name,
                           security_level_name(module_table[i].info.required_level));
                }
                return -1;
            }

            /* Verify capabilities against active security settings */
            if (!security_module_allowed(module_table[i].info.capabilities)) {
                if (rt) {
                    vdev_printf(vm_get_vdev((VMContext *)rt), "?Error: Module '%s' blocked by security level %s\n",
                                module_table[i].info.name,
                                security_level_name(security_get_level()));
                } else {
                    printf("?Error: Module '%s' blocked by security level %s\n",
                           module_table[i].info.name,
                           security_level_name(security_get_level()));
                }
                return -1;
            }

            if (module_table[i].info.init) {
                funcreg_set_registering_module(module_table[i].info.name);
                int res = module_table[i].info.init(rt);
                funcreg_set_registering_module(NULL);
                if (res != 0) {
                    if (rt) {
                        vdev_printf(vm_get_vdev((VMContext *)rt), "?Error: Module '%s' initialization failed\n", module_table[i].info.name);
                    } else {
                        printf("?Error: Module '%s' initialization failed\n", module_table[i].info.name);
                    }
                    return -1;
                }
            }

            module_table[i].active = 1;
            return 0;
        }
    }

    if (rt) {
        vdev_printf(vm_get_vdev((VMContext *)rt), "?Error: Module '%s' not found\n", name);
    } else {
        printf("?Error: Module '%s' not found\n", name);
    }
    return -1;
}

int module_deactivate(const char *name) {
    if (!name) return -1;

    for (int i = 0; i < module_table_count; ++i) {
        if (module_table[i].occupied && str_iequal(module_table[i].info.name, name)) {
            if (!module_table[i].active) {
                return -1;
            }

            if (module_table[i].info.cleanup) {
                module_table[i].info.cleanup();
            }

            module_table[i].active = 0;
            return 0;
        }
    }

    return -1;
}

int module_is_active(const char *name) {
    if (!name) return 0;
    for (int i = 0; i < module_table_count; ++i) {
        if (module_table[i].occupied && str_iequal(module_table[i].info.name, name)) {
            return module_table[i].active;
        }
    }
    return 0;
}

const BppModuleInfo *module_find(const char *name) {
    if (!name) return NULL;
    for (int i = 0; i < module_table_count; ++i) {
        if (module_table[i].occupied && str_iequal(module_table[i].info.name, name)) {
            return &module_table[i].info;
        }
    }
    return NULL;
}

const BppModuleInfo *module_get(int index) {
    if (index < 0 || index >= module_table_count) return NULL;
    if (!module_table[index].occupied) return NULL;
    return &module_table[index].info;
}

int module_is_loaded(int index) {
    if (index < 0 || index >= module_table_count) return 0;
    return module_table[index].active;
}

const char *module_class_name(BppModuleClass cls) {
    switch (cls) {
        case MOD_LIBRARY:   return "Library";
        case MOD_DIALECT:   return "Dialect";
        case MOD_DEVICE:    return "Device";
        case MOD_EXTENSION: return "Extension";
        default:            return "Unknown";
    }
}

void module_caps_string(unsigned int caps, char *buf, int buf_len) {
    if (buf_len <= 0) return;
    int pos = 0;

    if ((caps & CAP_MATH) && pos < buf_len - 1) buf[pos++] = 'M';
    if ((caps & CAP_STRING) && pos < buf_len - 1) buf[pos++] = 'S';
    if ((caps & CAP_IO) && pos < buf_len - 1) buf[pos++] = 'I';
    if ((caps & CAP_FILE) && pos < buf_len - 1) buf[pos++] = 'F';
    if ((caps & CAP_SYSTEM) && pos < buf_len - 1) buf[pos++] = 'Y';
    if ((caps & CAP_GRAPHICS) && pos < buf_len - 1) buf[pos++] = 'G';
    if ((caps & CAP_SOUND) && pos < buf_len - 1) buf[pos++] = 'A';
    if ((caps & CAP_NETWORK) && pos < buf_len - 1) buf[pos++] = 'N';
    if ((caps & CAP_GPIO) && pos < buf_len - 1) buf[pos++] = 'P';
    if ((caps & CAP_I2C) && pos < buf_len - 1) buf[pos++] = '2';
    if ((caps & CAP_SPI) && pos < buf_len - 1) buf[pos++] = '3';
    if ((caps & CAP_SENSOR) && pos < buf_len - 1) buf[pos++] = 'R';
    if ((caps & CAP_CAMERA) && pos < buf_len - 1) buf[pos++] = 'C';
    if ((caps & CAP_BLUETOOTH) && pos < buf_len - 1) buf[pos++] = 'B';
    if ((caps & CAP_USB) && pos < buf_len - 1) buf[pos++] = 'U';

    if (pos == 0 && buf_len > 1) {
        buf[pos++] = '-';
    }
    buf[pos] = '\0';
}

int module_load_dynamic(VMContext *vm, const char *path) {
#if defined(BASIC_FREEDOS) || defined(__ESP32__)
    if (vm) {
        vdev_printf(vm_get_vdev(vm), "Dynamic modules not supported on this platform.\n");
    } else {
        printf("Dynamic modules not supported on this platform.\n");
    }
    return -1;
#else
    typedef int (*InitFunc)(void);
    InitFunc init_fn = NULL;

    void *handle = platform_load_library(path);
    if (!handle) {
        if (vm) {
            vdev_printf(vm_get_vdev(vm), "Failed to load library: %s\n", platform_library_last_error());
        } else {
            printf("Failed to load library: %s\n", platform_library_last_error());
        }
        return -1;
    }
    init_fn = (InitFunc)platform_get_proc_address(handle, "module_init");
    if (!init_fn) {
        if (vm) {
            vdev_printf(vm_get_vdev(vm), "No module_init found in %s\n", path);
        } else {
            printf("No module_init found in %s\n", path);
        }
        platform_free_library(handle);
        return -1;
    }
    if (init_fn() != 0) {
        platform_free_library(handle);
        return -1;
    }
    return 0;
#endif
}
