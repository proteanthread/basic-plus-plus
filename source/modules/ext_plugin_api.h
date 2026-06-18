 // ---
 // BASIC++ Interpreter - ext_plugin_api.h
 // ---
 //
 // External Plugin API.
 //
 // PURPOSE:
 // Defines the interface for plugin packages -- the
 // highest-level extension mechanism. A plugin bundles
 // modules, libraries, features, and specs into a single
 // distributable directory with a YAML manifest.
 //
 // DESIGN:
 // A plugin is a directory containing:
 //   plugin.yaml       - YAML manifest (descriptor)
 //   *.dll / *.so      - C modules
 //   *.lib / *.bas     - BASIC++ libraries
 //   *.spec            - Spec files
 //
 // The manifest declares metadata, security level, and
 // lists all entries with their types. Entries are loaded
 // in dependency order: specs first, then features, then
 // modules, then libraries.
 //
 // MANIFEST FORMAT (YAML):
 //
 //   name: GAMEDEV
 //   version: "1.0"
 //   author: "BASIC++ Community"
 //   description: "Game development toolkit"
 //   security: SAFE
 //   entries:
 //     - type: spec
 //       file: collision.spec
 //     - type: feature
 //       file: sprite.spec
 //     - type: module
 //       file: mod_sprites.dll
 //     - type: library
 //       file: tilemap.lib
 //
 // HOW TO USE FROM BASIC++:
 //   LOAD PLUGIN "gamedev"
 //   ' MODULE "SPRITES", LIBRARY "tilemap",
 //   ' FEATURE "SPRITE", SPEC "COLLISION" now available
 //   UNLOAD PLUGIN "gamedev"
 //
 // UNLOAD ORDER:
 // Entries are unloaded in reverse order (libraries first,
 // then modules, then features, then specs) to respect
 // dependencies.
 //
 // SECURITY:
 // The manifest declares a required security level. All
 // entries within the plugin inherit this level. Entry
 // paths are validated to be within the plugin directory
 // (no path traversal allowed).
 //
 // C89/C90 COMPLIANT.
 //
 // ---

#ifndef BASICPP_EXT_PLUGIN_API_H
#define BASICPP_EXT_PLUGIN_API_H

#include "../config.h"
#include "../security.h"

// --- Plugin Entry Types ---
#define BPP_PLUG_MODULE   0 // C module (.dll/.so)
#define BPP_PLUG_LIBRARY  1 // BASIC++ library (.lib)
#define BPP_PLUG_FEATURE  2 // Feature (.spec + .lib)
#define BPP_PLUG_SPEC     3 // Standalone spec (.spec)

// --- Plugin Entry ---
 //
 // A single entry in the plugin manifest.
 // All paths are relative to the plugin directory.
typedef struct BppPluginEntry {
    int          type; // BPP_PLUG_*
    char         filename[256]; // relative path
} BppPluginEntry;

// Maximum entries per plugin
#define MAX_PLUGIN_ENTRIES 16

// --- Plugin Descriptor ---
 //
 // Tracks a loaded plugin and all its entries.
typedef struct BppPlugin {
    char         name[64]; // plugin name
    char         version[16]; // version string
    char         author[64]; // author / organization
    char         description[256]; // one-line description
    char         path[256]; // plugin directory path
    SecLevel     required_level; // SEC_COUNT = unpinned
    int          entry_count; // number of entries
    BppPluginEntry entries[MAX_PLUGIN_ENTRIES];
    int          loaded; // 1=loaded, 0=not
} BppPlugin;

// --- Plugin API ---

 // ext_plugin_init - Initialize the plugin table.
 // Call once at boot.
void ext_plugin_init(void);

 // ext_plugin_load - Load a plugin from a directory.
 //
 // Parses plugin.yaml, validates security, and loads
 // all entries in dependency order:
 //   1. Specs (.spec)
 //   2. Features (.spec + .lib)
 //   3. Modules (.dll/.so)
 //   4. Libraries (.lib/.bas)
 //
 // Security checks:
 //   1. SECOP_EXT_LOAD must be permitted
 //   2. Plugin directory path validated
 //   3. Manifest's required_level must match
 //   4. All entry paths within plugin directory
 //
 // Returns 0 on success, -1 on error.
int ext_plugin_load(const char *dir_path, void *rt);

 // ext_plugin_unload - Unload a plugin by name.
 //
 // Unloads entries in reverse order (libraries, modules,
 // features, specs) to respect dependencies.
 // Returns 0 on success, -1 if not found.
int ext_plugin_unload(const char *name);

 // ext_plugin_is_loaded - Check if a plugin is loaded.
 // Returns 1 if loaded, 0 if not.
int ext_plugin_is_loaded(const char *name);

 // ext_plugin_find - Find a loaded plugin by name.
 // Returns NULL if not found.
const BppPlugin *ext_plugin_find(const char *name);

 // ext_plugin_list - Print all loaded plugins with their
 // entries.
void ext_plugin_list(void);

 // ext_plugin_count - Return the number of loaded plugins.
int ext_plugin_count(void);

#endif // BASICPP_EXT_PLUGIN_API_H
