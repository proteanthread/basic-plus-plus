/**
 * @file main_tests.c
 * @brief Native C-Level Unit Test Suite Runner.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Registers all unit test suites (memory, lexer, value, strings)
 *   and executes them sequentially, reporting passes/fails and returning standard exit codes.
 * - Why it exists: Serves as the automated low-level verification executable for developer checks
 *   and continuous integration.
 * - Why it works this way: It loops over arrays of TestEntry structures. If any assertion fails
 *   and returns false, it increments the failure count and prints the failed test name.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: List of suites to execute, reporting message formats.
 * - What cannot be changed: Obligation to return a non-zero exit code if any test fails.
 * - What to expect: Running this executable prints detailed reports on every test case.
 * - What to do if something breaks: If tests crash due to segfaults, run inside a debugger
 *   to identify the exact assertion line.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: stdout is writable.
 * - Portability concerns: None. C17 standard compliant.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add command-line filters to run specific suites or test names.
 * - How to write external extensions: New test suites are registered in the main execution array.
 */

#include "test_harness.h"
#include <stdio.h>

extern TestEntry memory_test_suite[];
extern TestEntry lexer_test_suite[];
extern TestEntry value_test_suite[];
extern TestEntry strings_test_suite[];
extern TestEntry bytecode_test_suite[];

typedef struct {
    const char *name;
    TestEntry  *suite;
} SuiteEntry;

static SuiteEntry g_suites[] = {
    {"Memory Subsystem", memory_test_suite},
    {"Lexer Subsystem",  lexer_test_suite},
    {"Value/Variable Subsystem", value_test_suite},
    {"String Heap Subsystem", strings_test_suite},
    {"Bytecode & Detokenizer Subsystem", bytecode_test_suite},
    {NULL, NULL}
};

int main(void) {
    printf("==================================================\n");
    printf("        BASIC++ v6.0.0 NATIVE UNIT TESTS          \n");
    printf("==================================================\n");

    int total_runs = 0;
    int total_fails = 0;

    for (int s = 0; g_suites[s].name != NULL; ++s) {
        printf("\nSuite: %s\n", g_suites[s].name);
        printf("--------------------------------------------------\n");

        TestEntry *suite = g_suites[s].suite;
        for (int t = 0; suite[t].name != NULL; ++t) {
            printf("  Running %s...", suite[t].name);
            total_runs++;
            
            bool ok = suite[t].fn();
            if (ok) {
                printf(" [PASS]\n");
            } else {
                total_fails++;
                /* Error description already printed inside macro */
            }
        }
    }

    printf("\n==================================================\n");
    printf("TEST RUN COMPLETED\n");
    printf("Total Executed: %d\n", total_runs);
    printf("Passed:         %d\n", total_runs - total_fails);
    printf("Failed:         %d\n", total_fails);
    printf("==================================================\n");

    return (total_fails == 0) ? 0 : 1;
}
