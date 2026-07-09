BASIC++ Progressive Testing Strategy

The .\tests directory is the permanent home for cumulative, progressive regression test suites for the BASIC++ project. 

Testing Philosophy

BASIC++ relies heavily on deterministic behavior, multi-platform parity, and absolute backward compatibility with standards like GW-BASIC, QBASIC, and ECMA-116. As new features, functions, and keywords are integrated into the interpreter, there is always a risk of introducing regression failures.

To combat this, the .\tests directory is NOT a scratchpad for disposable scripts. All test files placed here must persist permanently.

Progressive Testing Mandate: Whenever you implement or run new tests in .\tests, you MUST rerun ALL previous tests in the directory to explicitly check for regression failures.

Directory Structure

Just like the .\help and .\docs folders, the .\tests folder MUST be organized into subdirectories by functional category. 

- tests/strings/ - String manipulation and functions
- tests/math/ - Mathematical operations
- tests/progmgmt/ - Program management and editing (AUTO, RENAME, RENUM)
- tests/filemgmt/ - File I/O and handling
- tests/virtual_memory/ - POKEB, PEEKB, BANK tests
- ...

All .BAS test scripts, .LOG output files, and .OUT artifacts MUST reside strictly within these subdirectories. 

Test Case Documentation

Every test case (.BAS script) in the .\tests directory must be fully commented. These comments must detail:
1. The goal of the test.
2. What is being tested for.
3. Expected behavior and correct outputs.
4. Specific instructions for the corresponding .c source code (what can be changed, what cannot, expectations, and troubleshooting steps if the test fails).

Manual Verification

If you do not wish to use the automated Python comprehensive tester (python -u scratch/run_comprehensive_tests.py), you can manually verify a test on any of our supported targets. 

For example, to test RENAME manually on Windows Console:
1. Navigate to the project root.
2. Pipe or load the test into the console executable:
   .\basicpp-console.exe -c "LOAD \"tests\filemgmt\test_rename.bas\" : LIST"
3. Alternatively, you can run the interpreter interactively and manually execute the keywords to observe the active program buffer changes. 

All progressive tests MUST be verified to work identically across all four targets:
- basicpp.exe (Windows SDL GUI)
- basicpp-console.exe (Windows Console)
- baspp (Linux SDL GUI)
- baspp-console (Linux Console)

Cleanup

After tests have been successfully passed, always clean up any leftover cruft, .LOG, and .OUT files from the test directories.
