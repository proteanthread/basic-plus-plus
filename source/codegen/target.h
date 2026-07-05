#ifndef BASICPP_TARGET_H
#define BASICPP_TARGET_H

#include <stdbool.h>

typedef enum {
    TARGET_WINDOWS,
    TARGET_LINUX,
    TARGET_FREEDOS,
    TARGET_EMBEDDED
} TargetPlatform;

typedef struct {
    const char *name;            /* "windows", "linux", "freedos", "embedded" */
    TargetPlatform platform;
    const char *target_macro;    /* e.g., "BPP_TARGET_WINDOWS" */
    bool has_multitasking;       /* Does target support async tasks? */
    bool has_graphics;           /* Does target support SDL2/graphics? */
    bool has_filesystem;         /* Does target support standard VFS/disk I/O? */
    long default_string_limit;   /* Target string size bounds */
    long default_array_limit;    /* Target array size bounds */
} TargetConfig;

/* Retrieve a target config by its name. Returns NULL if not found. */
const TargetConfig* target_find(const char *name);

/* Retrieve the default target config for the current host environment. */
const TargetConfig* target_get_default(void);

#endif
