#ifndef BASICPP_SPEC_H
#define BASICPP_SPEC_H

#include "lexer.h"
#include "security.h"

/* --- Specification Categories --- */
typedef enum {
    SPEC_CAT_UNKNOWN = 0,
    SPEC_CAT_STATEMENT,
    SPEC_CAT_FUNCTION,
    SPEC_CAT_DIALECT,
    SPEC_CAT_MODULE,
    SPEC_CAT_LIBRARY,
    SPEC_CAT_DEVICE
} SpecCategory;

/* --- Specification Object --- */
typedef struct SpecObject {
    char name[64];
    char version[16];
    SpecCategory category;
    char depends[256];
    char lib_path[256]; /* Path to the external .LIB file for behavior */
    KeywordId kw_id;    /* If it defines a new statement/function */
    SecLevel required_level; /* Pinned security level (SEC_COUNT = unpinned) */
} SpecObject;

/* --- Specification Registry API --- */
#define MAX_SPECS 128

void spec_registry_init(void);
int spec_load_file(const char *filename);
SpecObject* spec_find(const char *name);
void spec_list_all(void);

#endif /* BASICPP_SPEC_H */
