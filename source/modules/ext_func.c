/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: ext_func.c
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
 // BASIC++ Interpreter - ext_func.c
 // ---
 //
 // External Function loader implementation.
 //
 // IMPLEMENTATION:
 // Static table of MAX_EXT_FUNCS slots. Each slot stores
 // a BppExtFunc descriptor and a native library handle.
 // Functions are loaded from .dll/.so files and registered
 // in the function registry (funcreg).
 //
 // Hot-reload is supported: loading a function with the
 // same name as an existing one replaces it. Best practice
 // is to UNLOAD first.
 //
 // C89/C90 COMPLIANT.
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
#include "ext_func_api.h"
#include "../funcreg.h"
#include "../security.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define TokenType WinTokenType // avoid winnt.h collision
#include <windows.h>
#undef TokenType
#elif !defined(BPP_FREEDOS) && !defined(BPP_EMBEDDED)
#include <dlfcn.h>
#include <stdbool.h>
#include "../console.h"
#endif

// --- Slot ---
typedef struct ExtFuncSlot {
    BppExtFunc   desc; // function descriptor
    void        *handle; // native library handle
    int          occupied; // 1 = slot used
} ExtFuncSlot;

static ExtFuncSlot func_table[MAX_EXT_FUNCS];
static int func_count = 0;

// --- Case-insensitive compare (C89) ---
static int ext_str_iequal(const char *a, const char *b)
{
    if (!a || !b) return 0;
    while (*a && *b) {
        if (toupper((unsigned char)*a) !=
            toupper((unsigned char)*b))
            return 0;
        a++; b++;
    }
    return (*a == '\0' && *b == '\0');
}

// --- ext_func_init ---
void ext_func_init(void)
{
    memset(func_table, 0, sizeof(func_table));
    func_count = 0;
}

// --- ext_func_load ---
 // Load an external function from a .dll/.so.
 //
 // Steps:
 //   1. Security check (SECOP_EXT_LOAD)
 //   2. Path validation
 //   3. LoadLibrary / dlopen
 //   4. Resolve bpp_func_register symbol
 //   5. Validate descriptor and pinned level
 //   6. Register in funcreg (hot-reload if exists)
int ext_func_load(const char *path, void *rt)
{
#if defined(BPP_FREEDOS) || defined(BPP_EMBEDDED)
    (void)path; (void)rt;
    printf("External functions not supported on "
           "this platform.\n");
    return -1;
#else
    typedef const BppExtFunc *(*RegFunc)(void);
    RegFunc reg_fn;
    const BppExtFunc *desc;
    FunctionEntry entry;
    int slot;
    (void)rt;

    // Security gate
    if (security_check(SECOP_EXT_LOAD, 0) != 0)
        return -1;
    if (security_check_path(path, 0) != 0)
        return -1;

    // Find a free slot (or existing for hot-reload)
    slot = -1;
    for (int i = 0; i < func_count; i++) {
        if (!func_table[i].occupied) {
            if (slot < 0) slot = i;
        }
    }
    if (slot < 0) {
        if (func_count >= MAX_EXT_FUNCS) {
            printf("External function table full.\n");
            return -1;
        }
        slot = func_count;
    }

    // Load the library
    {
#ifdef _WIN32
        HMODULE handle = LoadLibraryA(path);
        if (!handle) {
            printf("Failed to load: %s\n", path);
            return -1;
        }
        reg_fn = (RegFunc)GetProcAddress(handle,
                     "bpp_func_register");
        if (!reg_fn) {
            printf("No bpp_func_register in %s\n",
                   path);
            FreeLibrary(handle);
            return -1;
        }
#else
        void *handle = dlopen(path, RTLD_LAZY);
        if (!handle) {
            printf("Failed to load: %s\n", dlerror());
            return -1;
        }
        reg_fn = (RegFunc)dlsym(handle,
                     "bpp_func_register");
        if (!reg_fn) {
            printf("No bpp_func_register in %s\n",
                   path);
            dlclose(handle);
            return -1;
        }
#endif

        // Get the descriptor
        desc = reg_fn();
        if (!desc || !desc->name || !desc->handler) {
            printf("Invalid function descriptor "
                   "in %s\n", path);
#ifdef _WIN32
            FreeLibrary(handle);
#else
            dlclose(handle);
#endif
            return -1;
        }

        // Check security pinning
        if (!security_check_pinned_level(
                desc->required_level)) {
            printf("Function '%s' requires security "
                   "level %s (current: %s).\n",
                   desc->name,
                   security_level_name(
                       desc->required_level),
                   security_level_name(
                       security_get_level()));
#ifdef _WIN32
            FreeLibrary(handle);
#else
            dlclose(handle);
#endif
            return -1;
        }

        // Hot-reload: check if name already loaded
        for (int i = 0; i < func_count; i++) {
            if (func_table[i].occupied &&
                ext_str_iequal(
                    func_table[i].desc.name,
                    desc->name)) {
                // Replace existing
#ifdef _WIN32
                if (func_table[i].handle)
                    FreeLibrary(
                        (HMODULE)func_table[i].handle);
#else
                if (func_table[i].handle)
                    dlclose(func_table[i].handle);
#endif
                slot = i;
                break;
            }
        }

        // Store in table
        func_table[slot].desc = *desc;
        func_table[slot].handle = handle;
        func_table[slot].occupied = 1;
        if (slot >= func_count)
            func_count = slot + 1;

        // Register in funcreg
        memset(&entry, 0, sizeof(entry));
        entry.name = desc->name;
        entry.keyword = KW_COUNT;
        entry.category = FCAT_USER;
        entry.ret_type = (FuncReturnType)desc->ret_type;
        entry.min_args = desc->min_args;
        entry.max_args = desc->max_args;
        entry.safety = (FuncSafety)desc->safety;
        entry.overridable = 0;
        entry.handler = desc->handler;
        entry.help_text = desc->help_text;
        funcreg_register(&entry);

        printf("Loaded external function: %s\n",
               desc->name);
        return 0;
    }
#endif // !(BPP_FREEDOS || BPP_EMBEDDED)
}

// --- ext_func_unload ---
int ext_func_unload(const char *name)
{
    for (int i = 0; i < func_count; i++) {
        if (func_table[i].occupied &&
            ext_str_iequal(func_table[i].desc.name,
                           name)) {
#if !defined(BPP_FREEDOS) && !defined(BPP_EMBEDDED)
#ifdef _WIN32
            if (func_table[i].handle)
                FreeLibrary(
                    (HMODULE)func_table[i].handle);
#else
            if (func_table[i].handle)
                dlclose(func_table[i].handle);
#endif
#endif
            // Note: funcreg doesn't support unregister,
             // but the handler pointer is now invalid.
             // The slot is cleared so find won't match. 
            func_table[i].occupied = 0;
            func_table[i].handle = NULL;
            printf("Unloaded function: %s\n", name);
            return 0;
        }
    }
    printf("Function '%s' not found.\n", name);
    return -1;
}

// --- ext_func_find ---
const BppExtFunc *ext_func_find(const char *name)
{
    for (int i = 0; i < func_count; i++) {
        if (func_table[i].occupied &&
            ext_str_iequal(func_table[i].desc.name,
                           name)) {
            return &func_table[i].desc;
        }
    }
    return NULL;
}

// --- ext_func_list ---
void ext_func_list(void)
{
    bool found = false;
    printf("--- Loaded External Functions ---\n");
    for (int i = 0; i < func_count; i++) {
        if (func_table[i].occupied) {
            printf("  %s (%d-%d args) [%s] %s\n",
                func_table[i].desc.name,
                func_table[i].desc.min_args,
                func_table[i].desc.max_args,
                security_level_name(
                    func_table[i].desc.required_level),
                func_table[i].desc.help_text ?
                    func_table[i].desc.help_text : "");
            found = true;
        }
    }
    if (!found) printf("  (none)\n");
}

// --- ext_func_count ---
int ext_func_count(void)
{
    int n = 0;
    for (int i = 0; i < func_count; i++) {
        if (func_table[i].occupied) n++;
    }
    return n;
}
