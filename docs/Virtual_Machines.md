# BASIC++ v6.5.2 Virtual Machines

## 1. THE BASIC++ VM

The BASIC++ Virtual Machine is the execution engine at the heart of the interpreter. It is a strictly non-recursive, stack-based VM that maintains all state on heap-managed structures. The VM is implemented across multiple source files in engine/src/vm/ and is defined by the VMContext interface in engine/include/vm/vm.h.

## 2. VM ARCHITECTURE

The VM does not execute machine code. It executes BASIC++ source lines through an ephemeral tokenize-parse-execute cycle. For each statement:

1. The lexer tokenizes the source line on demand.
2. The parser identifies the statement type from the first keyword.
3. The statement handler function executes the statement's semantics.
4. The handler returns a BppError indicating success or failure.
5. The VM advances to the next statement or handles the error.

This cycle repeats until the program reaches END, STOP, or an unhandled error.

## 3. VM CONTEXT

The VMContext is the central state container. It holds references to all subsystem contexts:

- MemoryContext — Program line storage.
- StringContext — Reference-counted string heap.
- VariableContext — Named variable table.
- ArrayContext — Array storage.
- VDevContext — Virtual device bus.
- FileContext — File channel management.
- VConContext — Virtual console state.
- SecurityContext — Active security level and restrictions.
- StmtRegistry — Statement handler dispatch table.
- MetadataRegistry — Runtime metadata for introspection.

## 4. CONTROL-FLOW STACKS

The VM maintains separate stacks for each control-flow construct:

**GosubStack** — GOSUB/RETURN. Each frame stores the return line and position.

**ForStack** — FOR/NEXT. Each frame stores the loop variable, limit, step, and loop body line.

**WhileStack** — WHILE/WEND. Each frame stores the WHILE line for looping back.

**DoStack** — DO/LOOP. Each frame stores the DO line and the test condition type.

**SelectStack** — SELECT CASE/END SELECT. Each frame stores the test expression and match state.

**SubStack** — SUB/FUNCTION calls. Each frame stores the procedure name, return line, and parameter state.

**TryStack** — TRY/CATCH. Each frame stores the CATCH line, END TRY line, and all other stack depths for unwinding.

Each stack has a configurable depth limit: 1023 on modern builds, 63 on FreeDOS, 31 on embedded. Exceeding any stack produces the appropriate error (Error 1 for NEXT without FOR, Error 3 for RETURN without GOSUB, etc.).

## 5. THE EXECUTION LOOP

The main execution loop (engine/src/vm/exec.c) follows this algorithm:

```text
1. Get the current line from ProgramStore
2. Initialize a LexerContext for the line
3. Peek at the first token
4. If TOK_NUMBER, this is a line-number reference — skip it
5. If TOK_KEYWORD, look up the handler in StmtRegistry
6. Call the handler function
7. If err.code != 0, enter error propagation
8. If no more tokens on the line, advance to the next line
9. If there is a colon separator, continue with the next statement
10. Poll the event queue (KEY, TIMER, COM, etc.)
11. If an event is pending, invoke the event handler
12. Repeat from step 1
```

The execution loop never recurses. Statement handlers that need to evaluate expressions use the iterative Pratt parser evaluator, which maintains its own stack on the heap.

## 6. ERROR PROPAGATION

When a statement handler returns a non-zero error code:

1. The VM checks the TryStack. If a TRY frame is active, it restores all stack depths to the TRY entry point and transfers execution to the CATCH line.

2. If no TRY frame is active, the VM checks the error trap (ON ERROR GOTO). If a trap is set, it saves the error state (ERR, ERL) and transfers execution to the trap handler line.

3. If neither TRY nor ON ERROR GOTO is active, the VM prints the error message and line number, then returns to the REPL prompt (or exits for the batch runner).

## 7. BYTECODE COMPILATION

The bppc compiler translates BASIC++ source into bytecode opcodes defined by the BppOpcode enum in engine/include/types/types.h. The bytecode is a compact binary representation of the program's operations. The bppc target can either:

- Emit bytecode as a standalone file that the VM stub loads and executes.
- Transpile the program to C17 source code that compiles to a native executable.

The bytecode execution loop (separate from the interpreter loop) reads opcodes from a byte array and dispatches through a switch statement. Bytecode execution is faster than source interpretation because it skips the tokenization and parsing stages.

## 8. THE DIALECT SYSTEM

The dialect system allows the parser to accept different keyword sets and syntax rules without changing the VM execution semantics. Each dialect defines:

- Which keywords are recognized.
- How certain syntax constructs are parsed (e.g., ECMA-116 WHEN EXCEPTION vs. TRY/CATCH).
- What default behaviors apply (e.g., OPTION BASE defaults).

The VM executes dialect-agnostic operations. A FOR loop parsed from GW-BASIC syntax and a FOR loop parsed from Tymshare Super BASIC syntax both produce the same BppForFrame on the ForStack and execute identically.

## 9. HOST CALLBACKS

The host callback interface (engine/include/vm/host.h) allows the host application to intercept VM operations. Callbacks can be registered for:

- Line execution (called before each line executes).
- Error reporting (called when an error occurs).
- Output capture (called for each character output).
- Input interception (called for each input request).

Host callbacks enable embedding the BASIC++ VM in larger applications that need to monitor or control program execution.
