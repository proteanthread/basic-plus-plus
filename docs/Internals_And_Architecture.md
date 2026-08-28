# BASIC++ v6.5.2 Internals and Architecture

## 1. EXECUTION PIPELINE

BASIC++ processes source code through a five-stage pipeline: Lexer, Parser, AST, Bytecode (optional), and VM. In the default interpreted mode, the first four stages are ephemeral — tokens and parse trees are generated on the fly for each statement execution and discarded immediately afterward. Source code is canonical and is never permanently tokenized. This design allows programs to modify themselves (by inserting or replacing lines) while maintaining predictable execution semantics.

The pipeline flow for a single statement execution:

1. The VM selects the next line from the ProgramStore (sorted line-number index).
2. The LexerContext (lex_init) tokenizes the source text, producing BppToken values on demand via lex_next() and lex_peek().
3. The parser consumes tokens and routes the statement to the appropriate handler based on the keyword dispatch table.
4. The statement handler evaluates expressions using the iterative evaluator (not recursive), accesses variables through the VariableContext, and performs I/O through the VDevContext.
5. The handler returns a BppError. If err.code is zero, the VM advances to the next statement. If non-zero, the error propagation system takes over.

## 2. THE NON-RECURSIVE VM

The VM is strictly non-recursive. All execution state resides on heap-managed structures rather than the C host stack. This means that deeply nested BASIC programs — hundreds of GOSUB levels, deeply nested FOR loops, complex expressions with many operators — execute without risk of C stack overflow.

The VMContext (defined in engine/include/vm/vm.h, implemented in engine/src/vm/context.c) is the central state container. It holds references to all subsystem contexts:

- **MemoryContext** — Program line storage and scratch allocator.
- **StringContext** — Reference-counted string heap manager.
- **VariableContext** — Named variable storage and scope management.
- **ArrayContext** — DIM array storage and MAT operations.
- **VDevContext** — Virtual device bus for all I/O operations.
- **FileContext** — File channel management.
- **VConContext** — Virtual console (cursor, colors, screen size).
- **VfsContext** — Virtual filesystem abstraction.
- **VNetContext** — Virtual network socket operations.
- **VMemContext** — Segmented virtual memory (vmem).
- **BiosContext** — BIOS virtualization for PC hardware emulation.
- **StmtRegistry** — Statement handler dispatch table.
- **TryStack** — TRY/CATCH exception handler stack.
- **MetadataRegistry** — Runtime metadata for introspection.

The VM also maintains separate stacks for each control-flow construct: GosubStack, ForStack, WhileStack, DoStack, SelectStack, and SubStack. Each stack has its own push, pop, peek, and depth query functions. This separation ensures that NEXT checks only the FOR stack, RETURN checks only the GOSUB stack, and WEND checks only the WHILE stack, producing precise error messages when control-flow statements are mismatched.

## 3. THE LEXER

The lexer (engine/include/lexer/lexer.h, engine/src/lexer/lexer.c) tokenizes a single source line on demand. It is initialized with lex_init(mem, source) and produces tokens via lex_next(ctx). Each token is a BppToken struct containing:

- **type** — The BppTokenType enum value (TOK_NUMBER, TOK_STRING, TOK_IDENT, TOK_KEYWORD, TOK_PLUS, TOK_MINUS, etc.).
- **as.number** — The numeric value (valid only for TOK_NUMBER).
- **as.string** — A pointer into the source buffer (valid only for TOK_STRING, NOT null-terminated).
- **as.keyword** — The BppKeywordId (valid only for TOK_KEYWORD).
- **start** — Pointer to the first character of the token in the source string.
- **length** — Length of the token in the source string.

The lexer recognizes approximately 367 keywords (the BppKeywordId enum in lexer.h). Custom keywords can be registered at runtime via keyword_register_custom(name), which returns a new BppKeywordId for dynamic language extension.

Tokens are ephemeral — they exist only while the lexer context is active. When the statement handler returns, the lexer context is freed and all token pointers become invalid.

## 4. THE EXPRESSION EVALUATOR

Expressions are evaluated iteratively using a Pratt parser / precedence-climbing algorithm. The evaluator does NOT use recursive descent (no recursive C function calls). Instead, it maintains an operator stack and a value stack on the heap, consuming tokens from the lexer and applying operators in precedence order.

The evaluator supports 14 precedence levels, from unary operators (highest) to IMP (lowest). Parentheses are handled by pushing a sentinel onto the operator stack at the left parenthesis and popping all operators down to the sentinel at the right parenthesis.

Function calls are dispatched through the function registry (engine/include/runtime/funcreg.h). Each built-in function is registered with its BppKeywordId, a function pointer, and the expected argument count. The evaluator recognizes function keywords, collects arguments, and dispatches through the registry.

## 5. THE VALUE SYSTEM

All runtime values are represented by the BValue tagged union (engine/include/types/types.h):

```c
typedef struct {
    ValueType type;     // VAL_NONE, VAL_NUMBER, VAL_INTEGER, VAL_STRING, VAL_MAP, VAL_ARRAY_REF, VAL_FIELD_STRING
    union {
        double       number;     // VAL_NUMBER and VAL_INTEGER
        BppStringRef string;     // VAL_STRING (reference-counted handle)
        BppMap      *map;        // VAL_MAP
        const char  *array_name; // VAL_ARRAY_REF
        struct { int channel; int offset; int length; } field_str; // VAL_FIELD_STRING
    } as;
} BValue;
```

VAL_NUMBER is the universal numeric type. All arithmetic operates on double-precision floating-point values. VAL_INTEGER is an optimization hint; internally it still uses the double field but signals that the value is known to be an exact integer.

VAL_STRING holds a BppStringRef, which is a pointer to a BppString managed by the StringContext. Strings are reference-counted: str_add_ref() increments the count, str_release() decrements it and frees the string when the count reaches zero. Every expression that produces a string increments the reference count; every consumer must release it.

VAL_FIELD_STRING is a special type for GW-BASIC random access file field buffers. It stores the channel number, byte offset, and byte length within the record buffer rather than a separate string heap allocation.

## 6. THE ERROR SYSTEM

Errors are represented by the BppError structure:

```c
typedef struct {
    int              code;       // GW-BASIC/QBASIC-compatible error code (1-255)
    BppErrorCategory category;   // SYNTAX, RUNTIME, SYSTEM, INTERNAL
    const char      *message;    // Human-readable error message
    BppLineNumber    line;       // BASIC line number
    int              col;        // Column position
    const char      *file;       // Host C source file (for diagnostics)
} BppError;
```

Error codes are defined in engine/include/types/errors.h and follow the GW-BASIC numbering (Error 1 through Error 76). Error categories enable filtering: syntax errors (parse failures), runtime errors (type mismatches, overflow, stack violations), system errors (file I/O, devices, permissions), and internal errors (VM panics, compiler limits).

When a statement handler returns a non-zero error code, the VM checks for an active error handler (ON ERROR GOTO or TRY/CATCH). If a handler is active, execution transfers to it. If no handler is active, the error message and line number are printed and execution stops.

## 7. THE SECURITY SYSTEM

The security system (engine/include/security/security.h) enforces access control through six levels:

| Level | Name | Description |
|-------|------|-------------|
| 0 | OPEN | All operations permitted |
| 1 | SAFE | Secure but functional |
| 2 | STANDARD | Controlled sandbox |
| 3 | EDUCATIONAL | Classroom mode |
| 4 | RESTRICTED | Very limited operations |
| 5 | PARANOID | Pure computation only |

Each security level defines a permission matrix for 19 operation categories (SECOP_FILE_READ through SECOP_EXT_LOAD). Before executing a sensitive operation, the statement handler calls security_check(op, line_num). If the check fails, it returns Error 70 (Permission denied).

Individual keywords can be restricted independently of the security level via security_restrict_keyword(kw_id). Individual operations can be restricted via security_restrict_op(op).

## 8. THE VIRTUAL DEVICE BUS

All I/O operations are virtualized through the VDev (Virtual Device) layer. The device bus provides a uniform interface for console output, file access, network operations, and hardware emulation. Statement handlers never call raw C library functions (printf, putchar, fopen) directly — they always go through VDev.

This abstraction enables: cross-platform portability (the same BASIC program produces the same output on Windows, Linux, and FreeDOS), testing (virtual devices can be mocked), and security (device access can be denied based on the security level).

## 9. THE DIALECT SYSTEM

Dialects are defined by the BppDialect structure, which specifies keyword availability, syntax rules, and behavioral overrides for each supported BASIC variant. The VM executes dialect-agnostic bytecode — the dialect only affects how source text is parsed, not how the resulting operations execute.

The active dialect is set by vm_set_active_dialect(vm, dialect). Multiple dialects can be loaded simultaneously, and the user can switch between them at runtime with the DIALECT command. Custom dialects can be defined by BASIC++ programs using the specification system.
