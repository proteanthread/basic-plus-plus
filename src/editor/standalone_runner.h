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
    if (access("bscript.exe", F_OK) == 0) return "bscript.exe";
    if (access("blite.exe", F_OK) == 0) return "blite.exe";
    if (access("baspp.exe", F_OK) == 0) return "baspp.exe";
    if (access("bscript", F_OK) == 0) return "./bscript";
    if (access("blite", F_OK) == 0) return "./blite";
    if (access("baspp", F_OK) == 0) return "./baspp";
    return NULL;
}

static void execute_standalone(const char *target, int exec_mode) {
    const char *runner = find_runner();
    if (!runner) {
        printf("\n[Error: Execution engine not found. Ensure bscript, blite, or baspp is in the directory.]\n");
        return;
    }
    
    const char *args = "";
    if (strstr(runner, "blite") || strstr(runner, "baspp")) {
        args = "-c ";
    }
    
    char cmd[2048];
    if (exec_mode == 1) { /* Debug */
        printf("\n[Standalone Debugging: Simulated Step Mode via %s]\n", runner);
        snprintf(cmd, sizeof(cmd), "%s %s\"%s\"", runner, args, target);
    } else if (exec_mode == 2) { /* Trace */
        printf("\n[Standalone Tracing: Simulated Trace Mode via %s]\n", runner);
        snprintf(cmd, sizeof(cmd), "%s %s\"%s\"", runner, args, target);
    } else {
        snprintf(cmd, sizeof(cmd), "%s %s\"%s\"", runner, args, target);
    }
    
    system(cmd);
}

#endif /* STANDALONE_EDITOR */
#endif /* STANDALONE_RUNNER_H */
