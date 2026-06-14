/*
 * ---
 * BASIC++ Interpreter - config_file.h
 * ---
 *
 * INI-style configuration file loader.
 *
 * PURPOSE:
 * Reads user-configurable defaults from a platform-specific
 * configuration file at boot time. Settings in the config file
 * are overridden by command-line switches, which are in turn
 * overridden by runtime commands (OPTION, DIALECT, SECURITY).
 *
 * PRIORITY CHAIN (lowest to highest):
 *   1. Compiled defaults (in main.c)
 *   2. Config file (baspp.cfg / basicpp.cfg / bpp.cfg)
 *   3. Command-line switches (-d, -s, -S)
 *   4. Runtime commands (DIALECT, SECURITY, OPTION STRICT)
 *
 * CONFIG FILE NAMES (platform-specific):
 *   Linux:   baspp.cfg
 *   Windows: basicpp.cfg
 *   FreeDOS: bpp.cfg
 *
 * CONFIG FILE SEARCH ORDER:
 *   1. Current working directory
 *   2. Home directory (~/ on Linux, %USERPROFILE% on Windows)
 *
 * ---
 */

#ifndef BASICPP_CONFIG_FILE_H
#define BASICPP_CONFIG_FILE_H

/* --- Parsed Configuration ---
 * All fields use empty-string or -1 to indicate "not set."
 * The caller should apply defaults for any unset fields.
 */
typedef struct ConfigFile {
    char dialect[16];    /* dialect short name: "PATB", "GWBS", etc. */
    char security[16];   /* security level: "OPEN", "STANDARD", "RESTRICTED" */
    int strict;          /* strict mode: 0=off, 1=on, -1=unset */
    int quiet;           /* suppress banner: 0=no, 1=yes, -1=unset */
    int found;           /* 1 if a config file was found and loaded */
    char filepath[256];  /* path of the file that was loaded */
} ConfigFile;

/*
 * config_file_load - Load configuration from the INI file.
 *
 * Searches for the platform-appropriate config file in:
 *   1. Current directory
 *   2. Home directory
 *
 * Parses key=value pairs. Ignores unknown keys gracefully.
 * Returns 0 on success (file found), -1 if no config file exists.
 * The ConfigFile struct is always initialized to defaults.
 */
int config_file_load(ConfigFile *cfg, const char *exe_path);

/*
 * config_file_load_path - Load configuration from a specific file.
 *
 * Like config_file_load(), but reads from the given path instead
 * of searching the default locations. Used with the -f switch.
 *
 * Returns 0 on success, -1 if the file cannot be opened.
 */
int config_file_load_path(ConfigFile *cfg, const char *path);

/*
 * config_file_get_name - Return the platform-specific config filename.
 *
 * Returns "baspp.cfg" on Linux, "basicpp.cfg" on Windows,
 * "bpp.cfg" on FreeDOS.
 */
const char *config_file_get_name(const char *exe_path);

#endif /* BASICPP_CONFIG_FILE_H */
