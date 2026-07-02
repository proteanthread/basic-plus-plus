/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: ext_plugin.c
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
 // BASIC++ Interpreter - ext_plugin.c
 // ---
 //
 // External Plugin loader implementation.
 //
 // IMPLEMENTATION:
 // Static table of MAX_EXT_PLUGINS slots. A plugin is a
 // directory containing a plugin.yaml manifest and any
 // combination of modules, libraries, features, and specs.
 //
 // YAML MANIFEST PARSER:
 // Minimal YAML subset parser for C89. Handles flat
 // key-value pairs and simple list entries with "- type:"
 // and "  file:" structure. Does NOT implement full YAML.
 //
 // LOAD ORDER (dependency-safe):
 //   1. Specs (.spec)         - keyword definitions
 //   2. Features (.spec+.lib) - language extensions
 //   3. Modules (.dll/.so)    - native code
 //   4. Libraries (.lib/.bas) - BASIC++ source
 //
 // UNLOAD ORDER (reverse):
 //   4. Libraries
 //   3. Modules
 //   2. Features
 //   1. Specs
 //
 // C89/C90 COMPLIANT.
 //
 // ---

#include <stdio.h>
#include <string.h>
#include "../compat.h"
#include <ctype.h>
#include "ext_plugin_api.h"
#include "ext_func_api.h"
#include <stdbool.h>
#include "ext_lib_api.h"
#include "ext_feature_api.h"
#include "../module.h"
#include "../spec.h"
#include "../security.h"
#include "../console.h"

// --- Slot ---
static BppPlugin plug_table[MAX_EXT_PLUGINS];
static int plug_count = 0;

// --- Case-insensitive compare (C89) ---
static int plug_str_iequal(const char *a, const char *b)
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

// --- Trim trailing whitespace/newlines ---
static void trim_trailing(char *s)
{
    int len = (int)strlen(s);
    while (len > 0 &&
           (s[len-1] == '\n' || s[len-1] == '\r' ||
            s[len-1] == ' '  || s[len-1] == '\t'))
        s[--len] = '\0';
}

// --- Strip surrounding quotes ---
static void strip_quotes(char *s)
{
    int len = (int)strlen(s);
    if (len >= 2 && s[0] == '"' && s[len-1] == '"') {
        memmove(s, s + 1, (size_t)(len - 2));
        s[len - 2] = '\0';
    }
}

// --- Parse YAML key-value pair ---
 // Given a line like "name: GAMEDEV", extracts
 // key into key_buf and value into val_buf.
 // Returns 1 if a key-value pair was found, 0 otherwise.
static int parse_yaml_kv(const char *line,
                          char *key_buf, int key_len,
                          char *val_buf, int val_len)
{
    const char *colon;
    const char *p;
    int klen;

    // Skip leading whitespace
    p = line;
    while (*p == ' ' || *p == '\t') p++;

    // Skip comments and empty lines
    if (*p == '#' || *p == '\0' || *p == '\n') return 0;

    // Find colon
    colon = strchr(p, ':');
    if (!colon) return 0;

    // Extract key
    klen = (int)(colon - p);
    if (klen >= key_len) klen = key_len - 1;
    strncpy(key_buf, p, (size_t)klen);
    key_buf[klen] = '\0';
    trim_trailing(key_buf);

    // Extract value
    p = colon + 1;
    while (*p == ' ' || *p == '\t') p++;
    strncpy(val_buf, p, (size_t)(val_len - 1));
    val_buf[val_len - 1] = '\0';
    trim_trailing(val_buf);
    strip_quotes(val_buf);

    return 1;
}

// --- parse_yaml_manifest ---
 // Minimal YAML parser for plugin.yaml.
 //
 // Handles:
 //   name: VALUE
 //   version: VALUE
 //   author: VALUE
 //   description: VALUE
 //   security: VALUE
 //   entries:
 //     - type: module
 //       file: path.dll
static int parse_yaml_manifest(const char *manifest_path,
                                BppPlugin *plug)
{
    FILE *fp;
    char line[512];
    char key[64];
    char val[256];
    bool in_entries = false;
    int cur_entry = -1;

    fp = fopen(manifest_path, "r");
    if (!fp) return -1;

    while (fgets(line, sizeof(line), fp)) {
        // Check for list item "- type:"
        {
            const char *p = line;
            while (*p == ' ' || *p == '\t') p++;
            if (*p == '-') {
                // New list entry
                p++;
                while (*p == ' ' || *p == '\t') p++;
                if (in_entries &&
                    plug->entry_count < MAX_PLUGIN_ENTRIES) {
                    cur_entry = plug->entry_count;
                    plug->entry_count++;
                    memset(&plug->entries[cur_entry], 0,
                           sizeof(BppPluginEntry));
                    // Parse inline "type: value"
                    if (parse_yaml_kv(p, key, sizeof(key),
                                      val, sizeof(val))) {
                        if (plug_str_iequal(key, "type")) {
                            if (plug_str_iequal(val,
                                    "module"))
                                plug->entries[cur_entry]
                                    .type =
                                    BPP_PLUG_MODULE;
                            else if (plug_str_iequal(val,
                                         "library"))
                                plug->entries[cur_entry]
                                    .type =
                                    BPP_PLUG_LIBRARY;
                            else if (plug_str_iequal(val,
                                         "feature"))
                                plug->entries[cur_entry]
                                    .type =
                                    BPP_PLUG_FEATURE;
                            else if (plug_str_iequal(val,
                                         "spec"))
                                plug->entries[cur_entry]
                                    .type =
                                    BPP_PLUG_SPEC;
                        }
                    }
                }
                continue;
            }
        }

        if (!parse_yaml_kv(line, key, sizeof(key),
                           val, sizeof(val)))
            continue;

        // Top-level keys
        if (plug_str_iequal(key, "name")) {
            strncpy(plug->name, val, 63);
            in_entries = 0;
        } else if (plug_str_iequal(key, "version")) {
            strncpy(plug->version, val, 15);
        } else if (plug_str_iequal(key, "author")) {
            strncpy(plug->author, val, 63);
        } else if (plug_str_iequal(key,
                       "description")) {
            strncpy(plug->description, val, 255);
        } else if (plug_str_iequal(key, "security")) {
            int lvl = security_find_level_by_name(val);
            if (lvl >= 0) {
                plug->required_level = (SecLevel)lvl;
            }
        } else if (plug_str_iequal(key, "entries")) {
            in_entries = 1;
        } else if (plug_str_iequal(key, "file")) {
            // File for current list entry
            if (cur_entry >= 0 &&
                cur_entry < plug->entry_count) {
                strncpy(
                    plug->entries[cur_entry].filename,
                    val, 255);
            }
        }
    }

    fclose(fp);
    return (plug->name[0] != '\0') ? 0 : -1;
}

// --- Build full entry path ---
 // Combines plugin directory + entry filename.
static void build_entry_path(char *buf, int buf_len,
                              const char *dir,
                              const char *filename)
{
    int dlen = (int)strlen(dir);
    if (dlen > 0 && dir[dlen-1] != '/' &&
        dir[dlen-1] != '\\') {
        bpp_snprintf(buf, (size_t)buf_len, "%s/%s",
                 dir, filename);
    } else {
        bpp_snprintf(buf, (size_t)buf_len, "%s%s",
                 dir, filename);
    }
}

// --- ext_plugin_init ---
void ext_plugin_init(void)
{
    memset(plug_table, 0, sizeof(plug_table));
    plug_count = 0;
}

// --- ext_plugin_load ---
 // Load a plugin from a directory.
 //
 // Steps:
 //   1. Security check (SECOP_EXT_LOAD)
 //   2. Parse plugin.yaml manifest
 //   3. Validate pinned level
 //   4. Validate all entry paths (within plugin dir)
 //   5. Load entries in dependency order
int ext_plugin_load(const char *dir_path, void *rt)
{
    int slot = -1;
    BppPlugin temp;
    char manifest[512];
    char entry_path[512];

    // Security gate
    if (security_check(SECOP_EXT_LOAD, 0) != 0)
        return -1;

    // Find a free slot
    for (int i = 0; i < plug_count; i++) {
        if (!plug_table[i].loaded) {
            slot = i;
            break;
        }
    }
    if (slot < 0) {
        if (plug_count >= MAX_EXT_PLUGINS) {
            printf("Plugin table full.\n");
            return -1;
        }
        slot = plug_count;
    }

    // Build manifest path
    build_entry_path(manifest, sizeof(manifest),
                     dir_path, "plugin.yaml");

    // Initialize descriptor
    memset(&temp, 0, sizeof(temp));
    strncpy(temp.path, dir_path, 255);
    temp.required_level = SEC_COUNT;

    // Parse YAML manifest
    if (parse_yaml_manifest(manifest, &temp) != 0) {
        printf("Failed to parse plugin manifest: "
               "%s\n", manifest);
        return -1;
    }

    // Check security pinning
    if (!security_check_pinned_level(
            temp.required_level)) {
        printf("Plugin '%s' requires security level "
               "%s (current: %s).\n",
               temp.name,
               security_level_name(temp.required_level),
               security_level_name(
                   security_get_level()));
        return -1;
    }

    // Validate all entry paths are within plugin dir
    int i;
    for (i = 0; i < temp.entry_count; i++) {
        if (strstr(temp.entries[i].filename, "..")) {
            printf("SORRY? Plugin '%s' entry '%s' "
                   "contains path traversal.\n",
                   temp.name,
                   temp.entries[i].filename);
            return -1;
        }
    }

    // Load entries in dependency order
    // Pass 1: Specs

    for (i = 0; i < temp.entry_count; i++) {
        if (temp.entries[i].type == BPP_PLUG_SPEC) {
            build_entry_path(entry_path,
                sizeof(entry_path), dir_path,
                temp.entries[i].filename);
            spec_load_file(entry_path);
        }
    }

    // Pass 2: Features

    for (i = 0; i < temp.entry_count; i++) {
        if (temp.entries[i].type == BPP_PLUG_FEATURE) {
            build_entry_path(entry_path,
                sizeof(entry_path), dir_path,
                temp.entries[i].filename);
            ext_feature_load(entry_path, rt);
        }
    }

    // Pass 3: Modules

    for (i = 0; i < temp.entry_count; i++) {
        if (temp.entries[i].type == BPP_PLUG_MODULE) {
            build_entry_path(entry_path,
                sizeof(entry_path), dir_path,
                temp.entries[i].filename);
            module_load_dynamic(entry_path);
        }
    }

    // Pass 4: Libraries
    for (int i = 0; i < temp.entry_count; i++) {
        if (temp.entries[i].type == BPP_PLUG_LIBRARY) {
            build_entry_path(entry_path,
                sizeof(entry_path), dir_path,
                temp.entries[i].filename);
            ext_lib_load(entry_path, rt);
        }
    }

    // Store in table
    temp.loaded = 1;
    plug_table[slot] = temp;
    if (slot >= plug_count) plug_count = slot + 1;

    printf("Loaded plugin: %s v%s",
           temp.name, temp.version);
    if (temp.author[0])
        printf(" by %s", temp.author);
    if (temp.required_level != SEC_COUNT)
        printf(" [%s]",
               security_level_name(temp.required_level));
    printf(" (%d entries)\n", temp.entry_count);

    return 0;
}

// --- ext_plugin_unload ---
int ext_plugin_unload(const char *name)
{
    int j;
    for (int i = 0; i < plug_count; i++) {
        if (plug_table[i].loaded &&
            plug_str_iequal(plug_table[i].name, name)) {
            // Unload in reverse order
            // Libraries first

            for (j = plug_table[i].entry_count - 1;
                 j >= 0; j--) {
                if (plug_table[i].entries[j].type ==
                    BPP_PLUG_LIBRARY) {
                    // Extract name from filename
                    char lib_name[64];
                    const char *base =
                        plug_table[i].entries[j].filename;
                    const char *p = base;
                    char *dot;
                    while (*p) {
                        if (*p == '/' || *p == '\\')
                            base = p + 1;
                        p++;
                    }
                    strncpy(lib_name, base, 63);
                    lib_name[63] = '\0';
                    dot = strrchr(lib_name, '.');
                    if (dot) *dot = '\0';
                    ext_lib_unload(lib_name);
                }
            }
            // Then modules (deactivate)

            for (j = plug_table[i].entry_count - 1;
                 j >= 0; j--) {
                if (plug_table[i].entries[j].type ==
                    BPP_PLUG_MODULE) {
                    // TODO: module_unload_dynamic()
                }
            }
            // Then features
            for (int j = plug_table[i].entry_count - 1;
                 j >= 0; j--) {
                if (plug_table[i].entries[j].type ==
                    BPP_PLUG_FEATURE) {
                    char feat_name[64];
                    const char *base =
                        plug_table[i].entries[j].filename;
                    const char *p = base;
                    char *dot;
                    while (*p) {
                        if (*p == '/' || *p == '\\')
                            base = p + 1;
                        p++;
                    }
                    strncpy(feat_name, base, 63);
                    feat_name[63] = '\0';
                    dot = strrchr(feat_name, '.');
                    if (dot) *dot = '\0';
                    ext_feature_unload(feat_name);
                }
            }

            plug_table[i].loaded = 0;
            printf("Unloaded plugin: %s\n", name);
            return 0;
        }
    }
    printf("Plugin '%s' not found.\n", name);
    return -1;
}

// --- ext_plugin_is_loaded ---
int ext_plugin_is_loaded(const char *name)
{
    for (int i = 0; i < plug_count; i++) {
        if (plug_table[i].loaded &&
            plug_str_iequal(plug_table[i].name, name))
            return 1;
    }
    return 0;
}

// --- ext_plugin_find ---
const BppPlugin *ext_plugin_find(const char *name)
{
    for (int i = 0; i < plug_count; i++) {
        if (plug_table[i].loaded &&
            plug_str_iequal(plug_table[i].name, name))
            return &plug_table[i];
    }
    return NULL;
}

// --- ext_plugin_list ---
void ext_plugin_list(void)
{
    static const char *type_names[] = {
        "module", "library", "feature", "spec"
    };
    int found = 0;

    printf("--- Loaded Plugins ---\n");
    for (int i = 0; i < plug_count; i++) {
        if (plug_table[i].loaded) {
            printf("  %s v%s [%s] %s\n",
                plug_table[i].name,
                plug_table[i].version,
                plug_table[i].required_level != SEC_COUNT
                    ? security_level_name(
                          plug_table[i].required_level)
                    : "any",
                plug_table[i].description);
            for (int j = 0;
                 j < plug_table[i].entry_count; j++) {
                int t = plug_table[i].entries[j].type;
                printf("    %s: %s\n",
                    (t >= 0 && t <= 3)
                        ? type_names[t] : "?",
                    plug_table[i].entries[j].filename);
            }
            found++;
        }
    }
    if (!found) printf("  (none)\n");
}

// --- ext_plugin_count ---
int ext_plugin_count(void)
{
    int n = 0;
    for (int i = 0; i < plug_count; i++) {
        if (plug_table[i].loaded) n++;
    }
    return n;
}
