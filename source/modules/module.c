/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: module.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    C-level modular expansions providing hardware wrappers and runtime libraries.
 *
 * 2. WHAT TO EXPECT:
 *    Modules register customized functions at boot to dynamically extend vocabulary.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Module naming, registered commands list, setup/shutdown details.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Module lifecycle dispatcher, keyword override bindings.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Ensure mod_init does not fail. Verify that linkage matches build profiles.
 * ===================================================================== */

 // ---
 // BASIC++ Interpreter - module.c
 // ---
 //
 // Module System implementation.
 //
 // IMPLEMENTATION:
 // Static module table with MAX_MODULES slots. Each slot stores
 // a ModuleInfo descriptor and an active flag. Modules are
 // registered at boot and activated on demand.
 //
 // Name lookups are case-insensitive (BASIC convention).
 //
//
// HOW TO EXTEND:
//   To add new functions to this module:
//   1. Add the function implementation in this file.
//   2. Register it in the module's init function using
//      module_register_function().
//   3. Update the module's header with the new declaration.
//
// TROUBLESHOOTING:
//   - Module not loading: check module_init() registration.
//   - Function not found: verify registration name matches
//     the BASIC keyword exactly (case-insensitive).
 // ---

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "module.h"
#include "security.h"

#ifdef _WIN32
#include <windows.h>
#elif !defined(BPP_FREEDOS)
#include <dlfcn.h>
#endif

// --- Module Table ---
typedef struct ModuleSlot {
 ModuleInfo info;
 int active; // 1 = active, 0 = inactive
 int occupied; // 1 = slot used, 0 = empty
} ModuleSlot;

static ModuleSlot module_table[MAX_MODULES];
static int module_table_count = 0;

// --- Case-insensitive string compare (portable C89) ---
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

// --- module_system_init ---
void module_system_init(void)
{
 memset(module_table, 0, sizeof(module_table));
 module_table_count = 0;
}

// --- module_register ---
int module_register(const ModuleInfo *info)
{
 if (!info || !info->name) return -1;

 if (module_table_count >= MAX_MODULES) {
 printf("Module table full.\n");
 return -1;
 }

 // Check for duplicate name
 {
 int i;
 for (i = 0; i < module_table_count; i++) {
 if (module_table[i].occupied &&
 str_iequal(module_table[i].info.name,
 info->name)) {
 // Already registered - skip silently
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

// --- module_activate ---
int module_activate(const char *name, void *rt)
{
 int i;

 for (i = 0; i < module_table_count; i++) {
 if (module_table[i].occupied &&
 str_iequal(module_table[i].info.name, name)) {
 // Already active - idempotent
 if (module_table[i].active) {
 return 0;
 }

 // Check security before activation
 if (!security_module_allowed(
 module_table[i].info.capabilities)) {
 printf("Module '%s' blocked by "
 "security level %s.\n",
 module_table[i].info.name,
 security_level_name(
 security_get_level()));
 return -1;
 }

 // Call init callback
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

// --- module_deactivate ---
int module_deactivate(const char *name)
{
 int i;

 for (i = 0; i < module_table_count; i++) {
 if (module_table[i].occupied &&
 str_iequal(module_table[i].info.name, name)) {
 if (!module_table[i].active) {
 return -1; // not active
 }

 // Call cleanup callback
 if (module_table[i].info.cleanup) {
 module_table[i].info.cleanup();
 }

 module_table[i].active = 0;
 return 0;
 }
 }

 return -1;
}

// --- module_is_active ---
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

// --- module_find ---
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

// --- module_count ---
int module_count(void)
{
 return module_table_count;
}

// --- module_get ---
const ModuleInfo *module_get(int index)
{
 if (index < 0 || index >= module_table_count) return NULL;
 if (!module_table[index].occupied) return NULL;
 return &module_table[index].info;
}

// --- module_is_loaded ---
int module_is_loaded(int index)
{
 if (index < 0 || index >= module_table_count) return 0;
 return module_table[index].active;
}

// --- module_class_name ---
const char *module_class_name(ModuleClass cls)
{
 switch (cls) {
 case MOD_LIBRARY: return "Library";
 case MOD_DIALECT: return "Dialect";
 case MOD_DEVICE: return "Device";
 case MOD_EXTENSION: return "Extension";
 default: return "Unknown";
 }
}

// --- module_caps_string ---
 // Formats capability flags as abbreviated letters:
 // M=Math S=String I=IO F=File Y=System G=Graphics A=Sound N=Net
void module_caps_string(unsigned int caps, char *buf, int buf_len)
{
 int pos = 0;

 if (buf_len <= 0) return;

 if ((caps & CAP_MATH) && pos < buf_len - 1) buf[pos++] = 'M';
 if ((caps & CAP_STRING) && pos < buf_len - 1) buf[pos++] = 'S';
 if ((caps & CAP_IO) && pos < buf_len - 1) buf[pos++] = 'I';
 if ((caps & CAP_FILE) && pos < buf_len - 1) buf[pos++] = 'F';
 if ((caps & CAP_SYSTEM) && pos < buf_len - 1) buf[pos++] = 'Y';
 if ((caps & CAP_GRAPHICS) && pos < buf_len - 1) buf[pos++] = 'G';
 if ((caps & CAP_SOUND) && pos < buf_len - 1) buf[pos++] = 'A';
 if ((caps & CAP_NETWORK) && pos < buf_len - 1) buf[pos++] = 'N';

    if (pos == 0 && buf_len > 1) {
        buf[pos++] = '-';
    }

    buf[pos] = '\0';
}

// --- module_load_dynamic ---
int module_load_dynamic(const char *path)
{
#ifdef BPP_FREEDOS
    printf("Dynamic modules not supported on FreeDOS.\n");
    return -1;
#else
    typedef void (*InitFunc)(void);
    InitFunc init;
#ifdef _WIN32
    HMODULE handle = LoadLibraryA(path);
    if (!handle) {
        printf("Failed to load library: %s\n", path);
        return -1;
    }
    
    init = (InitFunc)GetProcAddress(handle, "bpp_module_init");
    if (!init) {
        printf("No bpp_module_init found in %s\n", path);
        FreeLibrary(handle);
        return -1;
    }
    init();
    return 0;
#else
    void *handle = dlopen(path, RTLD_LAZY);
    if (!handle) {
        printf("Failed to load library: %s\n", dlerror());
        return -1;
    }
    
    init = (InitFunc)dlsym(handle, "bpp_module_init");
    if (!init) {
        printf("No bpp_module_init found in %s\n", path);
        dlclose(handle);
        return -1;
    }
    init();
    return 0;
#endif
#endif
}
