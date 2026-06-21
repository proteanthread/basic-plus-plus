import os
import sys

def get_file_docs(filepath, filename):
    # Determine the directory/component category
    norm_path = filepath.replace('\\', '/')
    parts = norm_path.split('/')
    parent_dir = parts[-2] if len(parts) > 1 else ""
    
    # Defaults
    purpose = "Utility or helper code for BASIC++ interpreter."
    expect = "Executes with low overhead, relying on fixed compile-time limits and memory pools."
    can_change = "Internal helper functions, optimization passes, or local naming adjustments."
    cannot_change = "Public API structures, parameter contracts, or global type definitions."
    troubleshoot = "Check memory pool margins, look for segmentation faults, and trace parameter values via a debugger."
    
    is_header = filename.endswith('.h')
    
    # Core Subsystem
    if parent_dir == 'core' or filename in ['memory.h', 'value.h', 'stringpool.h', 'errors.h', 'platform.h', 'security.h', 'config_file.h']:
        purpose = "Core interpreter engine infrastructure, memory pool allocator, error model, values, platform, security gating, and boot configurations."
        expect = "Fixed memory footprint utilizing compile-time pool allocators (defined in config.h). Avoids malloc/free at runtime."
        can_change = "Diagnostic logs, specific error message phrasing, platform detection strings, security sandbox policy matrices."
        cannot_change = "BValue tagged union structure fields, core memory allocator logic, security capability ratings."
        troubleshoot = "Check config.h pool sizes (e.g. increase PROGRAM_MEMORY_SIZE). If security level is ratcheted, check security level enforcement policies."
        
    # Lexer / Tokenizer
    elif parent_dir == 'lexer' or filename in ['lexer.h', 'keyword_props.h', 'alias_lang.h']:
        purpose = "Lexical analysis, zero-copy tokenization scanner, keyword property tables, and natural language keyword alias packs."
        expect = "Single-pass zero-copy tokenization. Keywords are filtered based on the active dialect configuration at parse time."
        can_change = "New keywords definitions, keyword property modifiers, and multilingual alias mappings."
        cannot_change = "Core lexer scan logic, token type enumeration values, zero-copy string indexing."
        troubleshoot = "Verify the keyword list ordering (it must be sorted/indexed correctly). Run self-tests for lexical parsing to locate tokenization boundaries."
        
    # Parser Subsystem
    elif parent_dir == 'parser' or filename in ['parser.h', 'parser_internal.h', 'parser_internal_additions.h']:
        purpose = "Statement dispatch and recursive-descent expression parsing."
        expect = "Highly recursive execution structure. Expression evaluation strictly obeys operator precedence levels."
        can_change = "Syntax parsing details for expressions, parser diagnostic logging, and parsing warnings."
        cannot_change = "Operator precedence hierarchy, recursive call stack structure, keyword parsing dispatch methods."
        troubleshoot = "Increase stack allocation limit if stack overflow occurs. Ensure expression parentheses evaluate in correct recursion."
        
    # Flow Control
    elif parent_dir == 'flow':
        purpose = "Program flow control statements including GOTO, GOSUB/RETURN, loops (FOR/NEXT, WHILE/WEND, DO/LOOP), and event/interrupt handlers."
        expect = "Maintains execution pointers and nested flow stack frames (such as FOR loop frames)."
        can_change = "Maximum loop nesting limits, flow tracing printouts."
        cannot_change = "GOSUB stack logic, execution loop frame structure, line number target checking."
        troubleshoot = "Ensure nested flow stacks do not overflow (increase MAX_STACK_DEPTH). Verify return addresses align with target loops."
        
    # Input / Output
    elif parent_dir == 'io' or filename in ['fileio.h']:
        purpose = "Input/Output operations, screen print layout, file streams (sequential/binary/random), block I/O formatting, and record buffers."
        expect = "Delegates file operations through the Virtual Device (VDev) mapping layer. Relies on standard ANSI C streams."
        can_change = "File channel limit rules, I/O default buffers, record layout details."
        cannot_change = "Binary packing format definitions (MKI$, MKS$, MKD$), file channel index lookups."
        troubleshoot = "Check file channel descriptor leaks (ensure CLOSE #ch is called). Verify binary file alignments and record record size boundaries."
        
    # File Management
    elif parent_dir == 'filemgmt':
        purpose = "File system directory operations (FILES, DIR, KILL, SCRATCH, COPY, MOVE, PWD, CHDIR, MKDIR, RMDIR)."
        expect = "Translates BASIC system actions into platform-specific directory routines safely."
        can_change = "Default file filter patterns, directory listing layouts."
        cannot_change = "Path separators validations, file deletion security gates."
        troubleshoot = "Verify that host OS permissions permit write/delete access. Ensure path lengths do not exceed compile constants."
        
    # Graphics Subsystem
    elif parent_dir == 'graphics' or filename in ['gfxbuf.h']:
        purpose = "Screen mode setting, line/circle drawing algorithms, palette mappings, and ANSI/Unicode text-mode framebuffer."
        expect = "Renders coordinate-scaled virtual buffer to terminal using Unicode half-blocks. High execution density."
        can_change = "Default palette colors, screen dimensions limits, drawing layout algorithms."
        cannot_change = "Framebuffer structure, Unicode screen printing characters mappings."
        troubleshoot = "If display is garbled, verify terminal supports UTF-8 and ANSI escape codes. Check coordinate math ranges."
        
    # Display & Sound
    elif parent_dir in ['display', 'sound']:
        purpose = "Console attributes (CLS, LOCATE, WIDTH, colors) and sound/music statements (BEEP, SOUND, PLAY)."
        expect = "Uses ANSI escape sequences for text styling. Enforces console bounds checking."
        can_change = "Styling escape sequences, default frequencies, music tempo constraints."
        cannot_change = "Coordinate range checks, pitch calculation formulas."
        troubleshoot = "Ensure terminal color settings are correct. Verify sound timer routines correspond to system sleep functions."
        
    # Variables, Arrays, Structs
    elif parent_dir in ['variables', 'arrays', 'struct'] or filename in ['funcreg.h', 'builtins.h', 'ast.h']:
        purpose = "Variables lookup, DIM arrays, User-Defined Types (UDT), matrix operations (MAT), and abstract syntax trees (AST)."
        expect = "Static array bounds mapping. Variable lookup hashes utilize fast string indexing."
        can_change = "Maximum array dimension numbers, matrix operators, UDT array constraints."
        cannot_change = "Row-major layout logic, variable storage offset rules, AST node type enumerations."
        troubleshoot = "Check array index limits. Ensure REDIM operations do not corrupt memory (verify array bounds rules)."
        
    # Functions Registry
    elif parent_dir == 'functions':
        purpose = "Built-in function master registrations and DEF FN evaluation."
        expect = "Evaluates functions parameters dynamically via a generic dispatch table."
        can_change = "Custom built-in functions additions, math function parameter limits."
        cannot_change = "Function registry lookup engine, registration metadata structure."
        troubleshoot = "Ensure function names are unique and match casing rules. Check parameter count validation limits."
        
    # Error Handling
    elif parent_dir == 'errhand':
        purpose = "Error catching, exception handlers, and exception recovery (ON ERROR, RESUME, WHEN/USE)."
        expect = "Enforces execution rollback using nested error handling scopes."
        can_change = "Custom error codes, debug tracking messages."
        cannot_change = "Exception scope stack traversal logic, error recovery pathways."
        troubleshoot = "If error handlers loop infinitely, check RESUME target line numbers and error registry states."
        
    # Shell & Debugging
    elif parent_dir in ['shell', 'debug'] or filename in ['selftest.h', 'check.h']:
        purpose = "Shell command execution, debugging tools (TRON/TROFF, breakpoints), self-test diagnostics, and static analysis verification."
        expect = "Interactions with the host OS shell are capability-gated. Self-test runs isolated test codes."
        can_change = "Self-test test cases, diagnostic log prints, static analyzer syntax warnings."
        cannot_change = "Breakpoints registry, self-test verification formulas."
        troubleshoot = "If shell fails, check host environment permissions. If self-test fails, check recent syntax changes."
        
    # Help & Config
    elif parent_dir in ['help', 'config'] or filename in ['help.h', 'override.h', 'scope.h']:
        purpose = "Self-describing HELP interactive system, OPTION overrides, and security scope permissions."
        expect = "Help prints keyword usage. Overrides rewrite token attributes at parse time."
        can_change = "Command summaries, help topics, scope presets mappings."
        cannot_change = "Help databases lookup logic, keyword gating routines."
        troubleshoot = "If help command fails, ensure help databases are sorted. Check override loop guards."
        
    # Program Management / Compiler
    elif parent_dir in ['progmgmt', 'codegen'] or filename in ['compiler.h', 'codegen.h', 'bytecode.h', 'detok.h']:
        purpose = "Stored program editor commands (RENUM, DELETE), compiler driver pipeline (BASIC-to-C), and bytecode serializers."
        expect = "Code generator constructs self-contained C89 files. Bytecode serializes code to files."
        can_change = "C89 codegen shims, editor warnings, target language mapping layout."
        cannot_change = "AST translation loops, bytecode file format specs."
        troubleshoot = "Verify target C compiler settings. If transpiled C file has compilation warnings, check codegen expressions mapping."
        
    # Virtual VM
    elif parent_dir == 'virtual' or filename in ['vdev.h', 'vm.h', 'memmap.h']:
        purpose = "Virtual Device Layer (VDev), Virtual Machine opcode dispatcher, and platform memory mappings."
        expect = "VDev slots abstract screen, keyboard, files, and GPIO. VM runs bytecode dispatch."
        can_change = "Virtual memory preset maps, new peripheral classes, custom hardware simulation registers."
        cannot_change = "VM opcodes, VM stack math, VDev operations interface."
        troubleshoot = "Trace I/O issues in VDev registration calls. Verify memory offsets are within configured segments."
        
    # Dialects
    elif parent_dir == 'dialect' or filename in ['dialect.h']:
        purpose = "BASIC historical dialect configuration profiles and strict execution filters."
        expect = "Dialect configurations switch prompt layouts, printing zone widths, statement separators, and active parsing tokens."
        can_change = "Dialect parameters (READY prompt, separator char), addition of custom dialects."
        cannot_change = "Dialect lookup logic, strict-mode keyword mask validation."
        troubleshoot = "Check dialect switches. If syntax is rejected, check if STRICT mode is enabled or keyword is filtered out."
        
    # Modules
    elif parent_dir == 'modules' or filename in ['module.h', 'mod_stdlib.h', 'mod_usb.h', 'mod_fujinet.h', 'mod_upnp.h']:
        purpose = "C-level modular expansions providing hardware wrappers and runtime libraries."
        expect = "Modules register customized functions at boot to dynamically extend vocabulary."
        can_change = "Module naming, registered commands list, setup/shutdown details."
        cannot_change = "Module lifecycle dispatcher, keyword override bindings."
        troubleshoot = "Ensure mod_init does not fail. Verify that linkage matches build profiles."
        
    # Runtime State
    elif parent_dir == 'runtime' or filename in ['exec.h', 'runtime.h']:
        purpose = "Runtime state container (RuntimeState) and statement execution dispatch loops."
        expect = "RuntimeState encapsulates all interpreter variables, stack frames, heap pools, and execution context."
        can_change = "Diagnostic trace outputs, signal trapping overrides."
        cannot_change = "RuntimeState struct layout, statement execution sequence logic."
        troubleshoot = "Verify execution pointer updates correctly. Trace memory crashes back to pool margins."

    # General fallback for specific config/main/Makefile
    elif filename == 'config.h':
        purpose = "Global compile-time configuration defines for pool memory sizes and limits."
        expect = "Constant parameters compiled directly into the binary file. Cannot negotiate limits at runtime."
        can_change = "Compile-time limits (e.g. increase MAX_STACK_DEPTH, MAX_STRING_POOL, or dial in BPP_EMBEDDED constants)."
        cannot_change = "Preprocessor flag rules, feature mappings."
        troubleshoot = "If compilation size is too big, reduce memory pool sizes. Verify all dependencies rebuild after editing config.h."
    elif filename == 'main.c':
        purpose = "Entry bootstrap logic, CLI flags parser, REPL interactive prompt loop, and startup initialization."
        expect = "First execution unit. Manages boot and clean shutdown lifecycles."
        can_change = "Prompt prints, startup banner details, custom CLI switches."
        cannot_change = "REPL input buffer processing loop, main launch conditions."
        troubleshoot = "Trace startup crashes. Verify all subsystems register during main initialization."

    doc_block = f"""/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: {filename}
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    {purpose}
 *
 * 2. WHAT TO EXPECT:
 *    {expect}
 *
 * 3. WHAT CAN BE CHANGED:
 *    {can_change}
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    {cannot_change}
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    {troubleshoot}
 * ===================================================================== */

"""
    return doc_block

def process_file(filepath, filename):
    print(f"Processing: {filepath}")
    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()
        
    # Check if comment block already exists to avoid duplication
    if "BASIC++ DEVELOPER & MAINTENANCE REFERENCE" in content:
        print(f"  Skipping (already updated): {filename}")
        return False
        
    doc_block = get_file_docs(filepath, filename)
    
    # Insert at the beginning, but handle includes/guards carefully if needed.
    # For C files and standard H files, inserting right at the top is best.
    new_content = doc_block + content
    
    with open(filepath, 'w', encoding='utf-8') as f:
        f.write(new_content)
    print(f"  Updated: {filename}")
    return True

def update_source_comments():
    base_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
    source_dir = os.path.join(base_dir, 'source')
    
    if not os.path.exists(source_dir):
        print(f"[ERROR] Source directory not found at: {source_dir}")
        sys.exit(1)
        
    updated_count = 0
    skipped_count = 0
    
    for root, dirs, files in os.walk(source_dir):
        for file in files:
            if file.endswith('.c') or file.endswith('.h'):
                filepath = os.path.join(root, file)
                if process_file(filepath, file):
                    updated_count += 1
                else:
                    skipped_count += 1
                    
    print(f"\nDone! Updated: {updated_count} files. Skipped: {skipped_count} files.")

if __name__ == '__main__':
    update_source_comments()
