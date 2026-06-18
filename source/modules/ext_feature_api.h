 // ---
 // BASIC++ Interpreter - ext_feature_api.h
 // ---
 //
 // External Feature API.
 //
 // PURPOSE:
 // Defines the interface for spec-driven language extensions.
 // A "feature" extends the BASIC++ language with new keywords
 // and statements defined via the SPEC system and implemented
 // in a companion .lib behavior file.
 //
 // DESIGN:
 // A feature = .spec file (keyword definitions) +
 //             .lib file (BASIC++ behavior implementation)
 //
 // The .spec file registers new keywords/statements via the
 // specification system. The .lib file provides the actual
 // implementation in BASIC++ (loaded into the library program
 // space, NOT the user's line-number range).
 //
 // FORMAT SUPPORT (Hybrid):
 // The companion .lib file can be plain BASIC++ source
 // (portable) or tokenized/pre-compiled (faster on
 // constrained platforms). The format may also depend
 // on the security level: tokenized libraries may be
 // required at higher security levels for integrity
 // verification.
 //
 // EXAMPLE FEATURE (TURTLE GRAPHICS):
 //
 //   turtle.spec:
 //     DEFINE SPECIFICATION "TURTLE"
 //         CATEGORY "STATEMENT"
 //         VERSION "1.0"
 //         SECURITY "SAFE"
 //         LIB "turtle.lib"
 //     END SPECIFICATION
 //
 //   turtle.lib:
 //     REM @LIBRARY TURTLE
 //     REM @VERSION 1.0
 //     REM @SECURITY SAFE
 //     SUB FORWARD(D)
 //       REM Move turtle forward D pixels
 //     END SUB
 //     SUB TURNLEFT(A)
 //       REM Turn turtle left A degrees
 //     END SUB
 //
 // HOW TO USE FROM BASIC++:
 //   LOAD FEATURE "turtle.spec"
 //   FORWARD 100
 //   TURNLEFT 90
 //   FORWARD 50
 //   UNLOAD FEATURE "TURTLE"
 //
 // SECURITY:
 // Features declare a required security level in their
 // .spec file via SECURITY "SAFE". Both the spec and the
 // companion library must satisfy the pinning check.
 //
 // C89/C90 COMPLIANT.
 //
 // ---

#ifndef BASICPP_EXT_FEATURE_API_H
#define BASICPP_EXT_FEATURE_API_H

#include "../config.h"
#include "../security.h"

// --- External Feature Descriptor ---
 //
 // Tracks a loaded feature (spec + library pair).
typedef struct BppExtFeature {
    char         name[64]; // feature name ("TURTLE")
    char         version[16]; // version string
    char         spec_path[256]; // path to .spec file
    char         lib_path[256]; // path to .lib behavior
    SecLevel     required_level; // SEC_COUNT = unpinned
    int          spec_loaded; // 1=spec registered
    int          lib_loaded; // 1=library merged
    int          keyword_count; // keywords added by spec
} BppExtFeature;

// --- Feature API ---

 // ext_feature_init - Initialize the feature table.
 // Call once at boot.
void ext_feature_init(void);

 // ext_feature_load - Load a feature from a .spec file.
 //
 // Loads the spec file via spec_load_file(), extracts the
 // companion .lib path from the spec, and loads the library
 // via ext_lib_load(). Both files are validated for security.
 //
 // Security checks:
 //   1. SECOP_EXT_LOAD must be permitted
 //   2. Spec path must pass security_check_path()
 //   3. Spec's required_level must match current level
 //   4. Companion .lib path validated and pinning checked
 //
 // Returns 0 on success, -1 on error.
int ext_feature_load(const char *spec_path, void *rt);

 // ext_feature_unload - Unload a feature by name.
 //
 // Unloads the companion library and unregisters the
 // spec keywords.
 // Returns 0 on success, -1 if not found.
int ext_feature_unload(const char *name);

 // ext_feature_is_loaded - Check if a feature is loaded.
 // Returns 1 if loaded, 0 if not.
int ext_feature_is_loaded(const char *name);

 // ext_feature_find - Find a loaded feature by name.
 // Returns NULL if not found.
const BppExtFeature *ext_feature_find(const char *name);

 // ext_feature_list - Print all loaded features.
void ext_feature_list(void);

 // ext_feature_count - Return the number of loaded features.
int ext_feature_count(void);

#endif // BASICPP_EXT_FEATURE_API_H
