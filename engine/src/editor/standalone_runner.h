// FILENAME: standalone_runner.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libstandard (edlin_internal.h)
// NEEDS: libcore (string.h), libplatform (platform.h)
// Implements visual text editor subsystem components for standalone_runner.
//
// ---- Includes ----

#ifndef STANDALONE_RUNNER_H
#define STANDALONE_RUNNER_H

#ifdef STANDALONE_EDITOR

#include "runtime/format/snprintf.h"
#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "platform/platform.h"

static inline bool runner_file_exists(const char *path) {
    void *f = platform_file_open(path, "rb");
    if (f) {
        platform_file_close(f);
        return true;
    }
    return false;
}

static const char* find_runner(void) {
    if (runner_file_exists("bs.exe")) return "bs.exe";
    if (runner_file_exists("bpp.exe")) return "bpp.exe";
    if (runner_file_exists("baspp.exe")) return "baspp.exe";
    if (runner_file_exists("bs")) return "./bs";
    if (runner_file_exists("bpp")) return "./bpp";
    if (runner_file_exists("baspp")) return "./baspp";
    return NULL;
}

static void execute_standalone(const char *target, int exec_mode) {
    const char *runner = find_runner();
    if (!runner) {
        return;
    }
    
    const char *args = "";
    if (runtime_strstr(runner, "bpp") || runtime_strstr(runner, "baspp")) {
        args = "-c ";
    }
    
    char cmd[2048];
    if (exec_mode == 1) { // Debug
        runtime_snprintf(cmd, sizeof(cmd), "%s %s\"%s\"", runner, args, target);
    } else if (exec_mode == 2) { // Trace
        runtime_snprintf(cmd, sizeof(cmd), "%s %s\"%s\"", runner, args, target);
    } else {
        runtime_snprintf(cmd, sizeof(cmd), "%s %s\"%s\"", runner, args, target);
    }
    
    platform_execute_system(cmd);
}

#endif // STANDALONE_EDITOR
#endif // STANDALONE_RUNNER_H
