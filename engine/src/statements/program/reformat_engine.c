// FILENAME: reformat_engine.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libengine (reformat_internal.h)
// Provides runtime implementation for the REFORMAT_ENGINE statement in BASIC++.
//
// ---- Includes ----

#include "statements/program/reformat_internal.h"

// All reformat engine subsystem logic is decomposed into:
// - statements/program/reformat/reformat_analyze.c: Tokenization, block nesting, and Pass 1 analysis
// - statements/program/reformat/reformat_indent.c: Pass 3 indentation, casing, blank lines, and line splitting
// - statements/program/reformat/reformat_report.c: Check report rendering, diff preview, and suggestions
