/* BASIC++ Help Database - Debugging & Testing */
#include "help.h"

const HelpEntry help_db_testing[] = {
    { "ASSERT", "Assert a condition is true", "ASSERT A = 5, \"A must be 5\"", HCAT_DEBUG },
    { "BREAK", "Set a breakpoint or pause", "BREAK", HCAT_DEBUG },
    { "CONT", "Continue after STOP or BREAK", "CONT", HCAT_DEBUG },
    { "BACKTRACE", "Print the call stack", "BACKTRACE", HCAT_DEBUG },
    { "CHECK", "Check internal state", "CHECK", HCAT_DEBUG },
    { "DEBUG", "Enter debug mode", "DEBUG ON", HCAT_DEBUG },
    { "DUMP", "Dump variables or memory", "DUMP", HCAT_DEBUG },
    { "ENDTEST", "End a test block", "ENDTEST", HCAT_DEBUG },
    { "SELFTEST", "Run built-in interpreter tests", "SELFTEST", HCAT_DEBUG },
    { "TEST", "Begin a test block", "TEST \"Math functions\"", HCAT_DEBUG },
    { "TRACE", "Enable detailed tracing", "TRACE ON", HCAT_DEBUG },
    { "TROFF", "Disable execution tracing", "TROFF", HCAT_DEBUG },
    { "TRON", "Enable execution tracing", "TRON", HCAT_DEBUG },
    { "VARS", "List all defined variables", "VARS", HCAT_DEBUG },
    { "VERIFY", "Verify program integrity", "VERIFY", HCAT_DEBUG },
    { NULL, NULL, NULL, 0 }
};
