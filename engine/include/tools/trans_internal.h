// FILENAME: trans_internal.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (trans.c)
// NEEDS: libcore (string.h)
// NEEDS: libengine (string.c, version.c)
// NEEDS: libkernel (version.h)
// Provides core logic and interface definitions for the BASIC++ transpiler (trans).
//
// ---- Includes ----

#ifndef TOOLS_TRANS_INTERNAL_H
#define TOOLS_TRANS_INTERNAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "runtime/format/snprintf.h"
#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

#include "types/version.h"

typedef struct {
    bool c17;
    bool inline_runtime;
    bool optimize;
    bool debug;
    const char *outfile;
    const char **infiles;
    int num_infiles;
} TranspilerOptions;

bool transpile_files_to_c(const char **infiles, int num_files, const char *outfile, bool inline_runtime, bool optimize, bool debug);
int trans_main_entry(int argc, char **argv);

#endif // TOOLS_TRANS_INTERNAL_H
