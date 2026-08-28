// FILENAME: analyzer.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (analyzer.c)
// NEEDED BY: libengine (check.c, verify.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libkernel (types.h, vdev.h, vdev.c)
// Provides core logic and interface definitions for analyzer within BASIC++.
//
// ---- Includes ----

#ifndef DEBUG_ANALYZER_H
#define DEBUG_ANALYZER_H

#include <stddef.h>
#include <stdbool.h>
#include "types/types.h"
#include "memory/memory.h"
#include "device/vdev.h"

#define ANALYZER_MAX_DIAGNOSTICS 64

typedef struct {
    BppLineNumber line;
    char severity[12];  // "ERROR" or "WARNING"
    char code[32];      // "ERR_BLOCK", "WARN_DATA_PARITY", etc.
    char message[128];
} AnalyzerDiagnostic;

typedef struct {
    size_t total_lines;
    size_t code_lines;
    size_t comment_lines;
    int for_count;
    int next_count;
    int while_count;
    int wend_count;
    int do_count;
    int loop_count;
    int if_count;
    int sub_count;
    int func_count;
    int data_items;
    int read_vars;
    int gosub_calls;
    int var_count;
    int unreachable_lines;
    int cyclomatic_complexity;
    char program_version[32];
    
    size_t diagnostic_count;
    AnalyzerDiagnostic diagnostics[ANALYZER_MAX_DIAGNOSTICS];
} StaticAnalysisReport;

// @brief Run static analysis against the specified memory program store.
void analyzer_run(MemoryContext *mem, StaticAnalysisReport *out_report);

// @brief Render high-level SUMMARY report to virtual device.
void analyzer_render_summary(VDevContext *vdev, const StaticAnalysisReport *report);

// @brief Render DETAILED line-by-line report to virtual device.
void analyzer_render_detailed(VDevContext *vdev, MemoryContext *mem, const StaticAnalysisReport *report);

// @brief Render structured JSON report payload to virtual device.
void analyzer_render_json(VDevContext *vdev, const StaticAnalysisReport *report);

#endif // DEBUG_ANALYZER_H
