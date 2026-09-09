<!--
Title:        Virtual Machine Tutorial
Tier:         2
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/vm/, engine/include/vm/vm.h
Generated:    no
Status:       Active
-->

# BASIC++ Virtual Machine Execution Tutorial

A step-by-step hands-on tutorial demonstrating how the BASIC++ Virtual Machine processes, tokenizes, evaluates, and executes programs across its internal execution pipeline.

## 1. The Execution Pipeline

Every statement in a BASIC++ program moves through a 4-stage pipeline:

```
[Source Code] --> [Lexer Tokens] --> [AST Parser] --> [Statement Dispatcher]
```

Unlike classic interpreted BASICs that tokenize the entire source file in advance or re-read raw ASCII strings, BASIC++ uses **ephemeral tokenization**: lines are scanned on demand and token streams are discarded immediately upon execution.

---

## 2. Step 1: Loading and Line Storage

When code is loaded into the interpreter, lines are sorted numerically into the `ProgramStore` (`engine/src/memory/`):

```basic
10 LET X = 10
20 LET Y = 20
30 PRINT "SUM: "; X + Y
40 END
```

The VM initializes a `ProgramLine` struct for each line. At this stage, no parsing has occurred; lines reside in memory as clean ASCII strings.

---

## 3. Step 2: Statement Dispatch and Execution

When `RUN` is issued:
1. **Line Fetch**: The VM sets the program counter to line 10.
2. **Lexer Pass**: `lexer_tokenize_line()` scans `10 LET X = 10` into tokens: `[TOK_LET, TOK_IDENT("X"), TOK_EQUALS, TOK_INT(10)]`.
3. **Dispatch**: The leading token `TOK_LET` triggers the assignment statement handler in `engine/src/statements/core/assign.c`.
4. **Variable Assignment**: `var_set_num(ctx, "X", 10.0)` records the variable in `VariableContext`.
5. **Advancement**: The VM proceeds to line 20, assigns `Y = 20`, and then moves to line 30.

---

## 4. Step 3: Expression Evaluation via RPN Stack

Line 30 contains an expression `X + Y`. The expression evaluator (`engine/src/eval/`):
1. Converts infix `X + Y` into Reverse Polish Notation: `X Y +`.
2. Pushes value of `X` (10.0) to the operand stack.
3. Pushes value of `Y` (20.0) to the operand stack.
4. Evaluates `+` via `eval_execute_op(OP_ADD)`, producing `30.0`.
5. Passes the result to `stmt_print()`, which emits `SUM: 30` to `CON:`.

---

## 5. Step 4: Procedure Calls and Stack Frames

Consider structured procedures:

```basic
10 PRINT "Starting..."
20 CALL AddNumbers(5, 7)
30 PRINT "Done."
40 END

SUB AddNumbers(A, B)
    PRINT "Result: "; A + B
END SUB
```

When line 20 executes:
1. `SubStack` pushes a new frame recording the caller's return line (30).
2. A new local scope is established in `VariableContext`, binding arguments `A = 5` and `B = 7`.
3. Execution jumps to `SUB AddNumbers`.
4. Upon reaching `END SUB`, the VM pops the `SubStack` frame, restores the global scope, and resumes execution at line 30.

---

## 6. Step 5: Exception Unwinding with `TRY/CATCH`

When errors occur inside a `TRY` block:

```basic
10 TRY
20   LET D = 10 / 0
30   PRINT "Not reached"
40 CATCH
50   PRINT "Caught error code: "; ERR
60 END TRY
```

1. Line 10 pushes a frame to `TryStack` recording current stack depths.
2. Line 20 triggers Error 11 (`Division by zero`).
3. Rather than halting, `vm_handle_error()` consults `TryStack`.
4. The VM unwinds `GosubStack` and `ForStack` back to the snapshot saved in line 10.
5. Control transfers to line 40 (`CATCH`), setting `ERR = 11`.
