/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: ext_feature.c
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
 // BASIC++ Interpreter - ext_feature.c
 // ---
 //
 // External Feature loader implementation.
 //
 // IMPLEMENTATION:
 // Static table of MAX_EXT_FEATURES slots. A feature is
 // a pair: .spec file (keyword definitions) + .lib file
 // (BASIC++ behavior). The spec is loaded via the spec
 // system; the library is loaded via ext_lib.
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
#include "ext_feature_api.h"
#include "ext_lib_api.h"
#include "../spec.h"
#include "../security.h"
#include "../console.h"

// --- Slot ---
static BppExtFeature feat_table[MAX_EXT_FEATURES];
static int feat_count = 0;

// --- Case-insensitive compare (C89) ---
static int feat_str_iequal(const char *a, const char *b)
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

// --- ext_feature_init ---
void ext_feature_init(void)
{
    memset(feat_table, 0, sizeof(feat_table));
    feat_count = 0;
}

// --- ext_feature_load ---
 // Load a feature from a .spec file.
 //
 // Steps:
 //   1. Security check (SECOP_EXT_LOAD)
 //   2. Path validation for .spec
 //   3. Load spec via spec_load_file()
 //   4. Find the loaded spec to get metadata
 //   5. Validate pinned level from spec
 //   6. Load companion .lib via ext_lib_load()
 //   7. Store feature descriptor
int ext_feature_load(const char *spec_path, void *rt)
{
    int slot = -1;
    SpecObject *spec;
    BppExtFeature temp;

    // Security gate
    if (security_check(SECOP_EXT_LOAD, 0) != 0)
        return -1;
    if (security_check_path(spec_path, 0) != 0)
        return -1;

    // Find a free slot
    for (int i = 0; i < feat_count; i++) {
        if (!feat_table[i].spec_loaded &&
            !feat_table[i].lib_loaded) {
            slot = i;
            break;
        }
    }
    if (slot < 0) {
        if (feat_count >= MAX_EXT_FEATURES) {
            printf("Feature table full.\n");
            return -1;
        }
        slot = feat_count;
    }

    // Load the spec file
    if (spec_load_file(spec_path) != 0) {
        printf("Failed to load spec: %s\n", spec_path);
        return -1;
    }

    // Initialize temp descriptor
    memset(&temp, 0, sizeof(temp));
    strncpy(temp.spec_path, spec_path, 255);

    // Find the spec that was just loaded.
     // We scan for the spec with a matching lib_path
     // (since spec_load_file may have loaded it). We
     // look for the most recently added spec. 
    spec = NULL;
    {
        // Try to extract name from filename
        const char *base = spec_path;
        const char *p = spec_path;
        char probe_name[64];
        while (*p) {
            if (*p == '/' || *p == '\\') base = p + 1;
            p++;
        }
        strncpy(probe_name, base, 63);
        probe_name[63] = '\0';
        {
            char *dot = strrchr(probe_name, '.');
            if (dot) *dot = '\0';
        }
        // Uppercase
        for (int i = 0; probe_name[i]; i++)
            probe_name[i] = (char)toupper(
                (unsigned char)probe_name[i]);

        spec = spec_find(probe_name);
    }

    if (spec) {
        snprintf(temp.name, 64, "%s", spec->name);
        snprintf(temp.version, 16, "%s", spec->version);
        temp.required_level = spec->required_level;
        if (spec->lib_path[0])
            snprintf(temp.lib_path, 256, "%s", spec->lib_path);
    } else {
        // Derive name from filename
        const char *base = spec_path;
        const char *p = spec_path;
        while (*p) {
            if (*p == '/' || *p == '\\') base = p + 1;
            p++;
        }
        strncpy(temp.name, base, 63);
        temp.name[63] = '\0';
        {
            char *dot = strrchr(temp.name, '.');
            if (dot) *dot = '\0';
        }
        for (int i = 0; temp.name[i]; i++)
            temp.name[i] = (char)toupper(
                (unsigned char)temp.name[i]);
        temp.required_level = SEC_COUNT;
    }

    // Check security pinning
    if (!security_check_pinned_level(
            temp.required_level)) {
        printf("Feature '%s' requires security level "
               "%s (current: %s).\n",
               temp.name,
               security_level_name(temp.required_level),
               security_level_name(
                   security_get_level()));
        return -1;
    }

    temp.spec_loaded = 1;

    // Load companion library if specified
    if (temp.lib_path[0]) {
        if (ext_lib_load(temp.lib_path, rt) == 0) {
            temp.lib_loaded = 1;
        } else {
            printf("Warning: companion library '%s' "
                   "for feature '%s' failed to load.\n",
                   temp.lib_path, temp.name);
            // Feature still usable without library
             // if spec defines all behavior 
        }
    }

    // Store in table
    feat_table[slot] = temp;
    if (slot >= feat_count) feat_count = slot + 1;

    printf("Loaded feature: %s", temp.name);
    if (temp.version[0])
        printf(" v%s", temp.version);
    if (temp.required_level != SEC_COUNT)
        printf(" [%s]",
               security_level_name(temp.required_level));
    printf("\n");

    return 0;
}

// --- ext_feature_unload ---
int ext_feature_unload(const char *name)
{
    for (int i = 0; i < feat_count; i++) {
        if ((feat_table[i].spec_loaded ||
             feat_table[i].lib_loaded) &&
            feat_str_iequal(feat_table[i].name, name)) {
            // Unload companion library first
            if (feat_table[i].lib_loaded) {
                ext_lib_unload(feat_table[i].name);
                feat_table[i].lib_loaded = 0;
            }
            // Spec keywords remain registered but
             // inactive (no behavior backing) 
            feat_table[i].spec_loaded = 0;
            printf("Unloaded feature: %s\n", name);
            return 0;
        }
    }
    printf("Feature '%s' not found.\n", name);
    return -1;
}

// --- ext_feature_is_loaded ---
int ext_feature_is_loaded(const char *name)
{
    for (int i = 0; i < feat_count; i++) {
        if (feat_table[i].spec_loaded &&
            feat_str_iequal(feat_table[i].name, name))
            return 1;
    }
    return 0;
}

// --- ext_feature_find ---
const BppExtFeature *ext_feature_find(const char *name)
{
    for (int i = 0; i < feat_count; i++) {
        if (feat_table[i].spec_loaded &&
            feat_str_iequal(feat_table[i].name, name))
            return &feat_table[i];
    }
    return NULL;
}

// --- ext_feature_list ---
void ext_feature_list(void)
{
    int found = 0;
    printf("--- Loaded Features ---\n");
    for (int i = 0; i < feat_count; i++) {
        if (feat_table[i].spec_loaded) {
            printf("  %s v%s [%s] spec:%s lib:%s\n",
                feat_table[i].name,
                feat_table[i].version,
                feat_table[i].required_level != SEC_COUNT
                    ? security_level_name(
                          feat_table[i].required_level)
                    : "any",
                feat_table[i].spec_loaded ? "yes" : "no",
                feat_table[i].lib_loaded ? "yes" : "no");
            found++;
        }
    }
    if (!found) printf("  (none)\n");
}

// --- ext_feature_count ---
int ext_feature_count(void)
{
    int n = 0;
    for (int i = 0; i < feat_count; i++) {
        if (feat_table[i].spec_loaded) n++;
    }
    return n;
}
