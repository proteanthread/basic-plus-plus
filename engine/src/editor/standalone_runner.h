// FILENAME: standalone_runner.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libstandard (edlin_internal.h)
// NEEDS: libcore (string.h)
// NEEDS: libengine (string.c)
// Implements visual text editor subsystem components for standalone_runner.
//
// ---- Includes ----

#ifndef STANDALONE_RUNNER_H
#define STANDALONE_RUNNER_H

#ifdef STANDALONE_EDITOR

#if !defined(_XOPEN_SOURCE)
#define _XOPEN_SOURCE 500
#endif
#if !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <io.h>
#ifndef F_OK
#define F_OK 0
#endif
#ifndef access
#define access _access
#endif
#else
#include <unistd.h>
#endif

static const char* find_runner(void) {
    if (access("bs.exe", F_OK) == 0) return "bs.exe";
    if (access("bpp.exe", F_OK) == 0) return "bpp.exe";
    if (access("baspp.exe", F_OK) == 0) return "baspp.exe";
    if (access("bs", F_OK) == 0) return "./bs";
    if (access("bpp", F_OK) == 0) return "./bpp";
    if (access("baspp", F_OK) == 0) return "./baspp";
    return NULL;
}

static void execute_standalone(const char *target, int exec_mode) {
    const char *runner = find_runner();
    if (!runner) {
        printf("\n[Error: Execution engine not found. Ensure bs, bpp, or baspp is in the directory.]\n");
        return;
    }
    
    const char *args = "";
    if (strstr(runner, "bpp") || strstr(runner, "baspp")) {
        args = "-c ";
    }
    
    char cmd[2048];
    if (exec_mode == 1) { // Debug
        printf("\n[Standalone Debugging: Simulated Step Mode via %s]\n", runner);
        snprintf(cmd, sizeof(cmd), "%s %s\"%s\"", runner, args, target);
    } else if (exec_mode == 2) { // Trace
        printf("\n[Standalone Tracing: Simulated Trace Mode via %s]\n", runner);
        snprintf(cmd, sizeof(cmd), "%s %s\"%s\"", runner, args, target);
    } else {
        snprintf(cmd, sizeof(cmd), "%s %s\"%s\"", runner, args, target);
    }
    
    system(cmd);
}

#endif // STANDALONE_EDITOR
#endif // STANDALONE_RUNNER_H
