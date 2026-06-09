/*
 * =====================================================================
 * BASIC++ Interpreter - module.c
 * =====================================================================
 *
 * Module System implementation (Phase 14).
 *
 * IMPLEMENTATION:
 *   Static module table with MAX_MODULES slots. Each slot stores
 *   a ModuleInfo descriptor and an active flag. Modules are
 *   registered at boot and activated on demand.
 *
 *   Name lookups are case-insensitive (BASIC convention).
 *
 * ANSI C89/C90 COMPLIANT
 * =====================================================================
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "module.h"
#include "security.h"

/* =====================================================================
 * Module Table
 * =====================================================================
 */
typedef struct ModuleSlot {
    ModuleInfo info;
    int        active;     /* 1 = active, 0 = inactive */
    int        occupied;   /* 1 = slot used, 0 = empty */
} ModuleSlot;

static ModuleSlot module_table[MAX_MODULES];
static int module_table_count = 0;

/* =====================================================================
 * Case-insensitive string compare (portable C89)
 * =====================================================================
 */
static int str_iequal(const char *a, const char *b)
{
    if (!a || !b) return 0;
    while (*a && *b) {
        if (toupper((unsigned char)*a) !=
            toupper((unsigned char)*b)) {
            return 0;
        }
        a++;
        b++;
    }
    return (*a == '\0' && *b == '\0');
}

/* =====================================================================
 * module_system_init
 * =====================================================================
 */
void module_system_init(void)
{
    memset(module_table, 0, sizeof(module_table));
    module_table_count = 0;
}

/* =====================================================================
 * module_register
 * =====================================================================
 */
int module_register(const ModuleInfo *info)
{
    if (!info || !info->name) return -1;

    if (module_table_count >= MAX_MODULES) {
        printf("Module table full.\n");
        return -1;
    }

    /* Check for duplicate name */
    {
        int i;
        for (i = 0; i < module_table_count; i++) {
            if (module_table[i].occupied &&
                str_iequal(module_table[i].info.name,
                           info->name)) {
                /* Already registered - skip silently */
                return 0;
            }
        }
    }

    module_table[module_table_count].info = *info;
    module_table[module_table_count].active = 0;
    module_table[module_table_count].occupied = 1;
    module_table_count++;

    return 0;
}

/* =====================================================================
 * module_activate
 * =====================================================================
 */
int module_activate(const char *name, void *rt)
{
    int i;

    for (i = 0; i < module_table_count; i++) {
        if (module_table[i].occupied &&
            str_iequal(module_table[i].info.name, name)) {
            /* Already active - idempotent */
            if (module_table[i].active) {
                return 0;
            }

            /* Check security before activation */
            if (!security_module_allowed(
                    module_table[i].info.capabilities)) {
                printf("Module '%s' blocked by "
                       "security level %s.\n",
                       module_table[i].info.name,
                       security_level_name(
                           security_get_level()));
                return -1;
            }

            /* Call init callback */
            if (module_table[i].info.init) {
                int result = module_table[i].info.init(rt);
                if (result != 0) {
                    printf("Module '%s' init failed.\n",
                           module_table[i].info.name);
                    return -1;
                }
            }

            module_table[i].active = 1;
            return 0;
        }
    }

    printf("Module '%s' not found.\n", name);
    return -1;
}

/* =====================================================================
 * module_deactivate
 * =====================================================================
 */
int module_deactivate(const char *name)
{
    int i;

    for (i = 0; i < module_table_count; i++) {
        if (module_table[i].occupied &&
            str_iequal(module_table[i].info.name, name)) {
            if (!module_table[i].active) {
                return -1;  /* not active */
            }

            /* Call cleanup callback */
            if (module_table[i].info.cleanup) {
                module_table[i].info.cleanup();
            }

            module_table[i].active = 0;
            return 0;
        }
    }

    return -1;
}

/* =====================================================================
 * module_is_active
 * =====================================================================
 */
int module_is_active(const char *name)
{
    int i;

    for (i = 0; i < module_table_count; i++) {
        if (module_table[i].occupied &&
            str_iequal(module_table[i].info.name, name)) {
            return module_table[i].active;
        }
    }

    return 0;
}

/* =====================================================================
 * module_find
 * =====================================================================
 */
const ModuleInfo *module_find(const char *name)
{
    int i;

    for (i = 0; i < module_table_count; i++) {
        if (module_table[i].occupied &&
            str_iequal(module_table[i].info.name, name)) {
            return &module_table[i].info;
        }
    }

    return NULL;
}

/* =====================================================================
 * module_count
 * =====================================================================
 */
int module_count(void)
{
    return module_table_count;
}

/* =====================================================================
 * module_get
 * =====================================================================
 */
const ModuleInfo *module_get(int index)
{
    if (index < 0 || index >= module_table_count) return NULL;
    if (!module_table[index].occupied) return NULL;
    return &module_table[index].info;
}

/* =====================================================================
 * module_is_loaded
 * =====================================================================
 */
int module_is_loaded(int index)
{
    if (index < 0 || index >= module_table_count) return 0;
    return module_table[index].active;
}

/* =====================================================================
 * module_class_name
 * =====================================================================
 */
const char *module_class_name(ModuleClass cls)
{
    switch (cls) {
        case MOD_LIBRARY:   return "Library";
        case MOD_DIALECT:   return "Dialect";
        case MOD_DEVICE:    return "Device";
        case MOD_EXTENSION: return "Extension";
        default:            return "Unknown";
    }
}

/* =====================================================================
 * module_caps_string
 * =====================================================================
 * Formats capability flags as abbreviated letters:
 *   M=Math S=String I=IO F=File Y=System G=Graphics A=Sound N=Net
 */
void module_caps_string(unsigned int caps, char *buf, int buf_len)
{
    int pos = 0;

    if (buf_len <= 0) return;

    if ((caps & CAP_MATH)     && pos < buf_len - 1) buf[pos++] = 'M';
    if ((caps & CAP_STRING)   && pos < buf_len - 1) buf[pos++] = 'S';
    if ((caps & CAP_IO)       && pos < buf_len - 1) buf[pos++] = 'I';
    if ((caps & CAP_FILE)     && pos < buf_len - 1) buf[pos++] = 'F';
    if ((caps & CAP_SYSTEM)   && pos < buf_len - 1) buf[pos++] = 'Y';
    if ((caps & CAP_GRAPHICS) && pos < buf_len - 1) buf[pos++] = 'G';
    if ((caps & CAP_SOUND)    && pos < buf_len - 1) buf[pos++] = 'A';
    if ((caps & CAP_NETWORK)  && pos < buf_len - 1) buf[pos++] = 'N';

    if (pos == 0 && buf_len > 1) {
        buf[pos++] = '-';
    }

    buf[pos] = '\0';
}
