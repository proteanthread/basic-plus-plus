# =====================================================================
# BASIC++ v6.0.0 — tests/c_unit/ CMake Configuration
# =====================================================================
#
# SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
# - What it does: Compiles the C-level unit test runner binary (run_c_tests).
# - Why it exists: Provides direct compile-and-test targets for low-level memory, lexer,
#   and value subsystems verification.
# - Why it works this way: It aggregates test sources, links against the static core library
#   libbasicpp, and maps include directories.
#
# SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
# - What can be changed: Adding new test source files to the sources list.
# - What cannot be changed: Obligation to link against libbasicpp.
# - What to expect: Running "run_c_tests" executes all assertions and exits with status 0 on success.
# - What to do if something breaks: If compilation fails, check include mappings.
#
# =====================================================================

add_executable(run_c_tests
    main_tests.c
    test_memory.c
    test_lexer.c
    test_value.c
    test_strings.c
    test_bytecode.c
)

target_link_libraries(run_c_tests PRIVATE libbasicpp_lite)
target_include_directories(run_c_tests PRIVATE ../../include)
