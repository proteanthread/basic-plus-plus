================================================================
 BASIC++ Transpiler Tutorial
 Extending COMPILE to Target Other Languages
================================================================

 Version:  1.1.0
 Command:  COMPILE "filename"
 Status:   Informational — for future development

================================================================
 TABLE OF CONTENTS
================================================================

 1.  How the Transpiler Works
 2.  The Three-Stage Pipeline
 3.  The AST — Language-Neutral Intermediate Representation
 4.  The C89 Backend (Current)
 5.  Mapping BASIC++ to Pascal
 6.  Mapping BASIC++ to Fortran
 7.  Mapping BASIC++ to Java
 8.  Mapping BASIC++ to Python 3
 9.  Mapping BASIC++ to 6502 Assembly
 10. Mapping BASIC++ to Z80 Assembly
 11. The Hardest Problems: GOTO, GOSUB, and ON
 12. The Runtime Shim Pattern
 13. String Handling Across Languages
 14. How to Add a New Backend
 15. Example: Full Translation of a Program
 16. Current Limitations
 17. Future Directions

================================================================
 1. HOW THE TRANSPILER WORKS
================================================================

The COMPILE command translates a BASIC++ program stored in
memory into a self-contained source file in another language.
Currently, only the C89 backend is implemented.

Usage:

  10 PRINT "Hello, World!"
  20 END
  COMPILE "hello.c"

This produces a complete, standalone C89 source file that
can be compiled with any ANSI C compiler:

  gcc -o hello hello.c -lm
  cl /Fe:hello.exe hello.c
  owcc -o hello hello.c

The resulting executable runs the program without needing
the BASIC++ interpreter at all.

================================================================
 2. THE THREE-STAGE PIPELINE
================================================================

The transpiler uses a clean three-stage architecture:

  ┌──────────┐     ┌──────────┐     ┌──────────┐
  │  LEXER   │────>│   AST    │────>│ CODEGEN  │
  │ (tokens) │     │  (tree)  │     │ (output) │
  └──────────┘     └──────────┘     └──────────┘
   Stage 1          Stage 2          Stage 3

Stage 1: LEXER (lexer.c)
  The lexer tokenizes each stored program line into a stream
  of tokens: keywords, numbers, strings, operators, variables.
  This is the same lexer used by the interpreter — no separate
  compilation lexer is needed.

Stage 2: AST BUILDER (ast.c)
  The AST builder parses the token stream and constructs an
  Abstract Syntax Tree for each program line. The AST is a
  language-neutral representation of the program's structure.

  Key: The AST knows nothing about C, Pascal, Fortran, or any
  target language. It represents pure BASIC++ semantics.

Stage 3: CODE GENERATOR (codegen.c)
  The code generator walks the AST and emits source code in
  the target language. Currently, only the C89 backend exists.

  To target a new language, you write a NEW codegen module
  that walks the SAME AST but emits different output.

The beauty of this architecture is that Stages 1 and 2 are
shared. Only Stage 3 changes when you add a new target.

================================================================
 3. THE AST — LANGUAGE-NEUTRAL INTERMEDIATE REPRESENTATION
================================================================

The AST is defined in ast.h. Every BASIC++ construct maps to
a node type:

  BASIC++ Construct     AST Node Type
  -----------------     -------------
  LET A = 10            STMT_LET
  LET A$ = "hello"      STMT_LET_STRVAR
  PRINT expr            STMT_PRINT
  INPUT var             STMT_INPUT
  GOTO 100              STMT_GOTO
  GOSUB 500             STMT_GOSUB
  RETURN                STMT_RETURN
  IF cond THEN ...      STMT_IF
  FOR I=1 TO 10         STMT_FOR
  NEXT I                STMT_NEXT
  WHILE cond            STMT_WHILE
  WEND                  STMT_WEND
  DIM A(10)             STMT_DIM
  DATA 1,2,3            STMT_DATA
  READ A                STMT_READ
  RESTORE               STMT_RESTORE
  END                   STMT_END
  STOP                  STMT_STOP
  REM comment           STMT_REM
  DEF FNA(X)=...        STMT_DEF_FN
  ON X GOTO 10,20       STMT_ON_GOTO

Expressions have their own node types:

  Expression            AST Node
  ----------            --------
  42                    EXPR_INT_LIT
  3.14                  EXPR_FLOAT_LIT
  "hello"               EXPR_STRING_LIT
  A (variable)          EXPR_VAR
  A$ (string var)       EXPR_STRING_VAR
  A+B                   EXPR_BINOP (BOP_ADD)
  A*B                   EXPR_BINOP (BOP_MUL)
  -A                    EXPR_UNOP (UOP_NEG)
  SIN(X)                EXPR_FUNC_CALL (FUNC_SIN)
  @(N) (array)          EXPR_ARRAY_AT
  DIM(I,J)              EXPR_DIM_ACCESS
  FNA(X) (user fn)      EXPR_FUNC_CALL (FUNC_FN_USER)

Each backend walks these same nodes and emits the target
language equivalent.

================================================================
 4. THE C89 BACKEND (CURRENT)
================================================================

The C89 backend (codegen.c) emits a self-contained ANSI C89
program with the following structure:

  1. INCLUDES:    #include <stdio.h>, <stdlib.h>, etc.

  2. RUNTIME:     Static helper functions for:
                  - GOSUB stack (call/return)
                  - DIM arrays (allocate/reference)
                  - String operations (concat, left$, etc.)
                  - DATA/READ pool
                  - PRINT formatting

  3. VARIABLES:   bpp_vars[26]     (A-Z as doubles)
                  bpp_strvars[26]  (A$-Z$ as char[256])
                  bpp_at_array[]   (legacy @ array)

  4. MAIN:        int main(void) { ... }
                  Each BASIC line becomes a labeled block:
                    L10: printf("Hello\n");
                    L20: return 0;
                  GOTO maps to C "goto L100;"
                  GOSUB uses a stack + goto + switch return

This pattern — a runtime shim + labeled blocks — is the
template that other backends follow, adapted to each
language's capabilities and idioms.

================================================================
 5. MAPPING BASIC++ TO PASCAL
================================================================

Pascal is the most natural target for BASIC++ because the
languages share many structural similarities.

  BASIC++                      Pascal (Free Pascal / Turbo)
  -------                      -----
  LET A = 10                   A := 10;
  PRINT "Hello"                WriteLn('Hello');
  PRINT A;                     Write(A:1);
  INPUT A                      ReadLn(A);
  IF A>5 THEN PRINT A          if A>5 then WriteLn(A);
  GOTO 100                     goto L100;  (* native! *)
  GOSUB 500                    (* push return, goto L500 *)
  FOR I=1 TO 10                for I:=1 to 10 do begin
  NEXT I                       end;
  WHILE A>0                    while A>0 do begin
  WEND                         end;
  DIM A(10)                    var A: array[0..10] of Real;
  REM comment                  { comment }
  END                          Halt;
  ABS(X)                       Abs(X)
  SIN(X)                       Sin(X)
  SQR(X)                       Sqrt(X)
  INT(X)                       Trunc(X)
  LEN(A$)                      Length(A_str)
  LEFT$(A$,3)                  Copy(A_str, 1, 3)
  MID$(A$,2,3)                 Copy(A_str, 2, 3)
  CHR$(65)                     Chr(65)
  ASC("A")                     Ord('A')
  STR$(42)                     (* IntToStr or Str() *)
  VAL("42")                    (* StrToInt or Val() *)

Pascal advantages:
  - GOTO is a native statement (labels must be declared)
  - BEGIN/END blocks map naturally to BASIC blocks
  - Strong typing catches errors at compile time
  - WriteLn/Write match PRINT semantics well
  - Arrays can be declared with any base (OPTION BASE)

Pascal challenges:
  - String variables need a different declaration
  - Type declarations are strict (can't mix int/float freely)
  - Computed GOTO (ON X GOTO) needs a CASE statement

Generated program structure:

  program Basic_Program;
  label L10, L20, L30, L100;  { all line numbers }
  var
    A, B, C: Real;             { A-Z variables }
    A_str, B_str: String;      { A$-Z$ strings }
    bpp_gosub_sp: Integer;
    bpp_gosub_stack: array[0..255] of Integer;
  begin
    L10: WriteLn('Hello, World!');
    L20: A := 42;
    L30: if A > 10 then WriteLn(A:1);
    L100: Halt;
  end.

================================================================
 6. MAPPING BASIC++ TO FORTRAN
================================================================

Fortran (specifically Fortran 90/95 or later) is another
strong target because of its native GOTO, computed GOTO,
and mathematical library.

  BASIC++                      Fortran 90
  -------                      ----------
  LET A = 10                   A = 10
  PRINT "Hello"                PRINT *, 'Hello'
  INPUT A                      READ *, A
  GOTO 100                     GO TO 100
  GOSUB 500                    CALL sub500()
  FOR I=1 TO 10                DO I=1, 10
  NEXT I                       END DO
  IF A>5 THEN PRINT A          IF (A > 5) PRINT *, A
  DIM A(10)                    REAL :: A(0:10)
  REM comment                  ! comment
  END                          STOP
  ON X GOTO 10,20,30           GO TO (10,20,30), X  (* native! *)
  ABS(X)                       ABS(X)
  SIN(X)                       SIN(X)
  SQR(X)                       SQRT(X)
  LOG(X)                       LOG(X)

Fortran advantages:
  - GOTO and computed GOTO are native statements
  - Excellent mathematical function library
  - Array indexing with any starting index
  - FORMAT statements for complex output
  - Statement labels (line numbers) are native

Fortran challenges:
  - Strings are fixed-length by default
  - GOSUB/RETURN has no direct equivalent (use subroutines)
  - Mixed numeric/string operations are awkward
  - I/O formatting differs significantly from BASIC

Generated program structure:

  PROGRAM basic_program
    IMPLICIT NONE
    REAL :: bpp_vars(26)          ! A-Z
    CHARACTER(LEN=256) :: bpp_str(26)  ! A$-Z$
    INTEGER :: bpp_gosub_sp = 0
    INTEGER :: bpp_gosub_stack(256)

    10 PRINT *, 'Hello, World!'
    20 bpp_vars(1) = 42           ! A = 42
    30 IF (bpp_vars(1) > 10) PRINT *, bpp_vars(1)
    100 STOP
  END PROGRAM basic_program

================================================================
 7. MAPPING BASIC++ TO JAVA
================================================================

Java presents unique challenges because it lacks GOTO entirely.
The backend must use a state-machine approach.

  BASIC++                      Java
  -------                      ----
  LET A = 10                   vars[0] = 10;  // A
  PRINT "Hello"                System.out.println("Hello");
  PRINT A;                     System.out.print(vars[0]);
  INPUT A                      vars[0] = scanner.nextDouble();
  IF A>5 THEN PRINT A          if (vars[0]>5) System.out.println(vars[0]);
  GOTO 100                     state = 100; continue;
  GOSUB 500                    gosubStack.push(state); state=500; continue;
  RETURN                       state = gosubStack.pop(); continue;
  FOR I=1 TO 10                for (vars[8]=1; vars[8]<=10; vars[8]++)
  END                          System.exit(0);
  ABS(X)                       Math.abs(x)
  SIN(X)                       Math.sin(x)
  SQR(X)                       Math.sqrt(x)
  RND(1)                       Math.random()
  LEN(A$)                      strVars[0].length()
  LEFT$(A$,3)                  strVars[0].substring(0,3)
  MID$(A$,2,3)                 strVars[0].substring(1,4)
  CHR$(65)                     String.valueOf((char)65)
  ASC("A")                     (double)"A".charAt(0)

Java's main challenge is GOTO. The solution is a dispatch
loop (state machine):

  public class BasicProgram {
    static double[] vars = new double[26];
    static String[] strVars = new String[26];
    static java.util.Stack<Integer> gosubStack
      = new java.util.Stack<>();
    static java.util.Scanner scanner
      = new java.util.Scanner(System.in);

    public static void main(String[] args) {
      int state = 10;  // first line number
      while (true) {
        switch (state) {
          case 10:
            System.out.println("Hello!");
            state = 20; break;
          case 20:
            vars[0] = 42;  // A = 42
            state = 30; break;
          case 30:
            if (vars[0] > 10)
              System.out.println(vars[0]);
            state = 100; break;
          case 100:
            System.exit(0);
        }
      }
    }
  }

This "switch-in-a-loop" pattern perfectly emulates GOTO
semantics in any language that lacks it. GOSUB pushes the
next state number, RETURN pops it.

================================================================
 8. MAPPING BASIC++ TO PYTHON 3
================================================================

Python 3 is a high-priority target because of its enormous
user base, readability, and rich standard library. Like Java,
Python 3 has no GOTO statement, so the state-machine pattern
is required.

IMPORTANT: This targets Python 3.6+ (f-strings). The
alternate match/case dispatcher targets Python 3.10+.
Python 2 is NOT supported (it reached end-of-life in 2020).

--- 8.1 Statement Mapping ---

  BASIC++                      Python 3
  -------                      --------
  LET A = 10                   v[0] = 10
  LET A$ = "hello"             sv[0] = "hello"
  PRINT "Hello"                print("Hello")
  PRINT A;                     print(v[0], end="")
  PRINT A,B                    print(f"{v[0]:14g}{v[1]:14g}")
  PRINT TAB(20);"HERE"         print(" "*20 + "HERE")
  INPUT A                      v[0] = float(input())
  INPUT "Name? ";N$            sv[13] = input("Name? ")
  LINE INPUT A$                sv[0] = input()
  GOTO 100                     state = 100; continue
  GOSUB 500                    gosub_stack.append(state)
                               state = 500; continue
  RETURN                       state = gosub_stack.pop(); continue
  IF A>5 THEN PRINT A          if v[0] > 5: print(v[0])
  FOR I=1 TO 10                (see FOR/NEXT note below)
  WHILE A>0 ... WEND           while v[0] > 0:
  DIM A(10)                    dim_a = [0.0] * 11
  DIM A$(10)                   dim_as = [""] * 11
  DATA 1,2,3                   _data = [1, 2, 3]
  READ A                       v[0] = _data[_data_ptr]; _data_ptr += 1
  RESTORE                      _data_ptr = 0
  RANDOMIZE                    random.seed()
  RANDOMIZE 42                 random.seed(42)
  SLEEP 2                      time.sleep(2)
  END                          sys.exit(0)
  STOP                         breakpoint()  # Python 3.7+
  REM comment                  # comment
  ON X GOTO 10,20,30           state = [10,20,30][int(v[23])-1]; continue

--- 8.2 Function Mapping ---

  BASIC++          Python 3             Notes
  -------          --------             -----
  ABS(X)           abs(x)               built-in
  SGN(X)           (1 if x>0 else (-1 if x<0 else 0))
  INT(X)           math.floor(x)        not int()!
  FIX(X)           math.trunc(x)        toward zero
  CINT(X)          round(x)             banker's rounding
  SIN(X)           math.sin(x)
  COS(X)           math.cos(x)
  TAN(X)           math.tan(x)
  ATN(X)           math.atan(x)
  SQR(X)           math.sqrt(x)
  LOG(X)           math.log(x)
  EXP(X)           math.exp(x)
  RND(1)           random.random()      0.0 to 1.0
  RND(N)           random.randint(1,N)  1 to N
  LEN(A$)          len(sv[0])           built-in
  LEFT$(A$,3)      sv[0][:3]            slice
  RIGHT$(A$,3)     sv[0][-3:]           slice
  MID$(A$,2,3)     sv[0][1:4]           slice (1-based adj)
  INSTR(A$,B$)     sv[0].find(sv[1])+1  0=not found
  CHR$(65)         chr(65)              built-in
  ASC("A")         ord("A")             built-in
  STR$(42)         str(42)              built-in
  VAL("42")        float("42")          built-in
  UCASE$(A$)       sv[0].upper()
  LCASE$(A$)       sv[0].lower()
  LTRIM$(A$)       sv[0].lstrip()
  RTRIM$(A$)       sv[0].rstrip()
  TRIM$(A$)        sv[0].strip()
  SPACE$(N)        " " * n
  STRING$(N,"*")   "*" * n
  HEX$(N)          hex(n)[2:].upper()
  OCT$(N)          oct(n)[2:]
  ENVIRON$("PATH") os.environ.get("PATH", "")
  DATE$            datetime.date.today().isoformat()
  TIME$            datetime.datetime.now().strftime("%H:%M:%S")
  TIMER            time.time() % 86400

--- 8.3 FOR/NEXT Handling ---

FOR/NEXT is tricky in a state-machine dispatcher because
Python's "for x in range()" is a structured loop that can't
be interrupted by state changes. Two approaches:

Approach 1 — Inline (when body has no GOTO):
  If the loop body contains no GOTO or GOSUB, emit a normal
  Python for loop:

    # 20 FOR I=1 TO 10 STEP 2
    # 30 PRINT I
    # 40 NEXT I
    for v[8] in range(1, 11, 2):   # I = vars[8]
        print(int(v[8]))

Approach 2 — Unrolled (when body has GOTO/GOSUB):
  If the loop body contains flow control, unroll it into
  the state machine:

    elif state == 20:
        v[8] = 1                   # I = 1
        state = 30; continue
    elif state == 30:
        print(int(v[8]))
        state = 40; continue
    elif state == 40:
        v[8] += 2                  # STEP 2
        if v[8] <= 10:
            state = 30; continue   # loop back
        state = 50; continue       # fall through

--- 8.4 Generated Program Structure ---

A complete Python 3 translation looks like this:

  #!/usr/bin/env python3
  """Generated by BASIC++ Compiler v1.1.0"""

  import math
  import sys
  import random
  import os
  import time
  from datetime import date, datetime

  # === Runtime Variables ===
  v = [0.0] * 26           # A-Z numeric variables
  sv = [""] * 26           # A$-Z$ string variables
  gosub_stack: list[int] = []

  # === Legacy array @() ===
  at_array = [0.0] * 4096

  # === DIM arrays ===
  dim_arrays: dict[str, list] = {}

  def dim_alloc(name: str, d1: int, d2: int = 0) -> None:
      size = (d1 + 1) * (d2 + 1 if d2 > 0 else 1)
      if name.endswith("$"):
          dim_arrays[name] = [""] * size
      else:
          dim_arrays[name] = [0.0] * size

  def dim_ref(name: str, i1: int, i2: int = 0) -> int:
      """Return the flat index into the dim array."""
      if name not in dim_arrays:
          raise IndexError(f"Array not found: {name}")
      return i1  # simplified; 2D: i1*(d2+1)+i2

  # === DATA/READ ===
  _data: list = []  # populated from DATA statements
  _data_ptr: int = 0

  def bpp_read() -> float:
      global _data_ptr
      if _data_ptr >= len(_data):
          print("OUT OF DATA", file=sys.stderr)
          sys.exit(1)
      val = _data[_data_ptr]
      _data_ptr += 1
      return float(val)

  # === DEF FN ===
  user_fns: dict[str, callable] = {}

  # === PRINT width ===
  pw = 14

  # === Main Program ===
  state = 10  # first line number
  while True:
      if state == 10:
          print("Hello, World!")
          state = 20
      elif state == 20:
          v[0] = 42  # A = 42
          state = 30
      elif state == 30:
          if v[0] > 10:
              print(v[0])
          state = 100
      elif state == 100:
          sys.exit(0)
      else:
          print(f"UNDEF'D LINE {state}", file=sys.stderr)
          sys.exit(1)

--- 8.5 Python 3.10+ match/case Alternative ---

Python 3.10 introduced structural pattern matching, which
provides a cleaner state machine than if/elif chains:

  state = 10
  while True:
      match state:
          case 10:
              print("Hello, World!")
              state = 20
          case 20:
              v[0] = 42
              state = 30
          case 30:
              if v[0] > 10:
                  print(v[0])
              state = 100
          case 100:
              sys.exit(0)
          case _:
              print(f"UNDEF'D LINE {state}",
                    file=sys.stderr)
              sys.exit(1)

The match/case version is more readable and may be slightly
faster for programs with many line numbers, since the Python
compiler can optimize the dispatch.

--- 8.6 Python 3 Advantages ---

  - Strings are native and powerful (slicing, f-strings)
  - No type declarations needed (duck typing)
  - Rich standard library (math, random, os, time)
  - List comprehensions replace many BASIC patterns
  - Exception handling maps to ON ERROR GOTO
  - Dictionary maps to AppKey-style storage
  - input() with prompt maps directly to INPUT "prompt"
  - print() with end="" maps to PRINT with semicolon

--- 8.7 Python 3 Challenges ---

  - No GOTO (state machine required)
  - No GOSUB (stack + state machine required)
  - Indentation is syntactically significant
  - Strings are immutable (no in-place modification)
  - Integer division differs: 7/2=3.5 not 3
    (use 7//2 for integer division)
  - 1-based vs 0-based indexing (MID$ starts at 1)
  - BASIC's semicolon in PRINT needs end="" handling
  - DEF FN maps to lambda (single expression only)

================================================================
 9. MAPPING BASIC++ TO 6502 ASSEMBLY
================================================================

The 6502 (Atari 8-bit, Apple II, C64) is the original home
of many BASIC dialects. A 6502 backend would produce native
machine code.

  BASIC++                      6502 Assembly
  -------                      -------------
  LET A = 10                   LDA #10 / STA VAR_A
  PRINT "Hello"                LDX #<MSG / LDY #>MSG / JSR PRINT
  GOTO 100                     JMP L0100
  GOSUB 500                    JSR L0500
  RETURN                       RTS
  IF A>5 THEN GOTO 100         LDA VAR_A / CMP #5 / BCS L0100
  FOR I=1 TO 10                LDA #1 / STA VAR_I / ...
  PEEK(addr)                   LDA addr   (native!)
  POKE addr,val                LDA val / STA addr   (native!)
  END                          BRK  or  RTS

6502 challenges:
  - 8-bit registers (need 16-bit or floating-point library)
  - No hardware multiply or divide
  - Stack is only 256 bytes (limits GOSUB depth)
  - Floating point requires a software library (e.g. Woz FP)
  - String handling must be done entirely in software

This backend would be the most complex but would produce
the fastest possible code for retro platforms.

================================================================
 10. MAPPING BASIC++ TO Z80 ASSEMBLY
================================================================

The Z80 (TRS-80, ZX Spectrum, MSX, CPC) is another classic
target.

  BASIC++                      Z80 Assembly
  -------                      ------------
  LET A = 10                   LD A, 10 / LD (VAR_A), A
  PRINT "Hello"                LD HL, MSG / CALL PRINT
  GOTO 100                     JP L0100
  GOSUB 500                    CALL L0500
  RETURN                       RET
  IF A>5 THEN GOTO 100         LD A,(VAR_A) / CP 5 / JP NC,L0100
  PEEK(addr)                   LD A, (addr)   (native!)
  POKE addr,val                LD A, val / LD (addr), A

Z80 advantages over 6502:
  - 16-bit register pairs (HL, DE, BC)
  - Hardware stack for CALL/RET
  - Block move instructions (LDIR/LDDR)
  - More addressing modes

================================================================
 11. THE HARDEST PROBLEMS: GOTO, GOSUB, AND ON
================================================================

The biggest challenge in transpiling BASIC++ is GOTO. Some
target languages support it, some don't:

  Language      GOTO?   GOSUB?       Solution
  --------      -----   ------       --------
  C89           YES     goto+stack   Direct mapping
  Pascal        YES     goto+stack   Direct mapping
  Fortran       YES     CALL/RET     Near-direct
  Java          NO      NO           State machine
  Python 3      NO      NO           State machine (if/elif or match/case)
  JavaScript    NO      NO           State machine
  6502 ASM      YES     JSR/RTS      Direct mapping
  Z80 ASM       YES     CALL/RET     Direct mapping

The State Machine Pattern:

For languages without GOTO, every program becomes:

  state = first_line_number
  while (true) {
    switch (state) {
      case 10:  /* line 10 code */  state = 20; break;
      case 20:  /* line 20 code */  state = 30; break;
      case 100: /* GOTO 100 */      state = 100; break;
    }
  }

GOTO becomes: state = target; break;
GOSUB becomes: push(next_state); state = target; break;
RETURN becomes: state = pop(); break;

ON X GOTO 10,20,30 becomes:
  targets = [10, 20, 30]
  state = targets[(int)X - 1]
  break

This pattern works in ANY language with a loop and a switch
(or if/elif chain).

================================================================
 12. THE RUNTIME SHIM PATTERN
================================================================

Every backend needs a "runtime shim" — a set of helper
functions that implement BASIC operations in the target
language. The current C89 backend includes:

  Shim Function      Purpose
  -------------      -------
  bpp_dim_alloc      Allocate a DIM array
  bpp_dim_ref        Reference a DIM array element
  bpp_concat         String concatenation
  bpp_left           LEFT$ function
  bpp_right          RIGHT$ function
  bpp_mid            MID$ function
  bpp_chr            CHR$ function
  bpp_str            STR$ function
  bpp_tab            TAB function
  bpp_read_num       DATA/READ support
  bpp_tmp            Temporary string pool

When targeting a new language, you write equivalent shim
functions in that language. For example:

  Pascal shim for LEFT$:
    function bpp_left(s: String; n: Integer): String;
    begin
      bpp_left := Copy(s, 1, n);
    end;

  Java shim for LEFT$:
    static String bppLeft(String s, int n) {
      return s.substring(0, Math.min(n, s.length()));
    }

  Python 3 shim for LEFT$:
    def bpp_left(s: str, n: int) -> str:
      return s[:n]

Some languages need very little shimming (Python 3 has
native slicing), while others need extensive support (6502
needs
a complete floating-point library).

================================================================
 13. STRING HANDLING ACROSS LANGUAGES
================================================================

Strings are the second-hardest problem after GOTO. Each
language handles strings very differently:

  Language   Strings          Concat   Substring
  --------   -------          ------   ---------
  C89        char arrays      strcat   memcpy
  Pascal     String type      +        Copy()
  Fortran    CHARACTER(LEN=)  //       str(a:b)
  Java       String objects   +        substring()
  Python 3   str objects      +        slicing [:]
  6502       byte arrays      custom   custom
  Z80        byte arrays      LDIR     LDIR

The BASIC++ string model:
  - Strings are mutable (you can LET A$ = "new value")
  - Maximum length is 255 characters
  - A$-Z$ are the 26 string variables
  - String arrays via DIM A$(10)

For C89, strings are char[256] with strncpy for assignment.
For Pascal, native String type handles everything.
For Java and Python 3, strings are immutable — every
modification creates a new string object.
For assembly targets, strings are length-prefixed byte
arrays with custom manipulation routines.

================================================================
 14. HOW TO ADD A NEW BACKEND
================================================================

Step-by-step process to add a new target language:

1. CREATE A NEW CODEGEN FILE

   Create codegen_pascal.c (or codegen_java.c, etc.)
   Copy the structure from codegen.c but change all
   fprintf calls to emit the new language.

2. IMPLEMENT emit_runtime_shim()

   Write the runtime helper functions in the target
   language. Include:
   - Variable storage (A-Z numeric, A$-Z$ string)
   - GOSUB stack
   - DIM array support
   - String helper functions
   - DATA/READ pool

3. IMPLEMENT emit_expr()

   Walk each EXPR_* node and emit the target equivalent:
   - EXPR_INT_LIT:    "42" (same in most languages)
   - EXPR_FLOAT_LIT:  "3.14" (same in most languages)
   - EXPR_STRING_LIT: "'hello'" (Pascal) or "hello" (others)
   - EXPR_VAR:        A (or vars[0] for array-based)
   - EXPR_BINOP:      Handle each operator
   - EXPR_FUNC_CALL:  Map to target's equivalent

4. IMPLEMENT emit_stmt()

   Walk each STMT_* node and emit the target equivalent:
   - STMT_PRINT:   WriteLn() / System.out.println() / print()  # Python 3
   - STMT_LET:     A := 10 / vars[0] = 10
   - STMT_IF:      if ... then / if (...) { / if ...:
   - STMT_GOTO:    goto L100 / state = 100; break
   - STMT_FOR:     for I:=1 to 10 do / for (...) {
   - STMT_GOSUB:   Push + goto / Push + state = target

5. IMPLEMENT codegen_emit_xxx()

   Write the main entry point that:
   - Emits the file header (program declaration, imports)
   - Emits the runtime shim
   - Emits variable declarations
   - Emits each line as a labeled block
   - Emits the file footer (end of program)

6. ADD A COMPILE TARGET OPTION

   Modify the COMPILE command in parser.c to accept an
   optional target parameter:
     COMPILE "output.pas" TARGET "PASCAL"
     COMPILE "output.java" TARGET "JAVA"

   The default remains C89 for backward compatibility.

7. TEST WITH SIMPLE PROGRAMS FIRST

   Start with:
     10 PRINT "Hello, World!"
     20 END

   Then work up to:
     10 FOR I=1 TO 10
     20 PRINT I
     30 NEXT I
     40 END

   Then test GOTO, GOSUB, IF/THEN, DIM arrays, strings.

================================================================
 15. EXAMPLE: FULL TRANSLATION OF A PROGRAM
================================================================

Source program:

  10 PRINT "Squares 1-10"
  20 FOR I=1 TO 10
  30 PRINT I, I*I
  40 NEXT I
  50 END

--- C89 Output (current) ---

  #include <stdio.h>
  #include <stdlib.h>
  #include <math.h>

  static double bpp_vars[26];
  static int bpp_print_width = 14;

  int main(void) {
  L10: printf("Squares 1-10\n");
  L20: for (bpp_vars[8]=1; bpp_vars[8]<=10; bpp_vars[8]++) {
  L30:   { double _v=bpp_vars[8];
         if(_v==(double)(long)_v)
           printf("%*ld",bpp_print_width,(long)_v);
         else printf("%g",_v); }
         { double _v=bpp_vars[8]*bpp_vars[8];
         if(_v==(double)(long)_v)
           printf("%*ld",bpp_print_width,(long)_v);
         else printf("%g",_v); }
         printf("\n");
  L40: }
  L50: return 0;
  }

--- Pascal Output (theoretical) ---

  program Squares;
  label 10, 20, 30, 40, 50;
  var
    I: Real;
    bpp_pw: Integer;
  begin
    bpp_pw := 14;
    10: WriteLn('Squares 1-10');
    20: for I := 1 to 10 do begin
    30:   Write(I:bpp_pw:0, (I*I):bpp_pw:0);
          WriteLn;
    40: end;
    50: Halt;
  end.

--- Fortran 90 Output (theoretical) ---

  PROGRAM Squares
    IMPLICIT NONE
    REAL :: bpp_vars(26)
    INTEGER :: I

    10 PRINT *, 'Squares 1-10'
    20 DO I=1, 10
    30   PRINT '(I14, I14)', I, I*I
    40 END DO
    50 STOP
  END PROGRAM Squares

--- Java Output (theoretical) ---

  public class Squares {
    static double[] v = new double[26];
    public static void main(String[] args) {
      int state = 10;
      while (true) { switch (state) {
        case 10:
          System.out.println("Squares 1-10");
          state = 20; break;
        case 20:
          for (v[8]=1; v[8]<=10; v[8]++) {
            System.out.printf("%14d%14d%n",
              (int)v[8], (int)(v[8]*v[8]));
          }
          state = 50; break;
        case 50:
          System.exit(0);
      }}
    }
  }

--- Python 3 Output (theoretical) ---

  #!/usr/bin/env python3
  """Generated by BASIC++ Compiler v1.1.0"""
  import math, sys

  v = [0.0] * 26   # A-Z numeric variables
  sv = [""] * 26   # A$-Z$ string variables
  pw = 14          # PRINT width

  state = 10
  while True:
      if state == 10:
          print("Squares 1-10")
          state = 20
      elif state == 20:
          for v[8] in range(1, 11):   # FOR I=1 TO 10
              print(f"{int(v[8]):>{pw}}{int(v[8]*v[8]):>{pw}}")
          state = 50
      elif state == 50:
          sys.exit(0)
      else:
          print(f"UNDEF'D LINE {state}", file=sys.stderr)
          sys.exit(1)

--- Python 3.10+ Output (with match/case) ---

  #!/usr/bin/env python3
  import math, sys

  v = [0.0] * 26
  pw = 14

  state = 10
  while True:
      match state:
          case 10:
              print("Squares 1-10")
              state = 20
          case 20:
              for v[8] in range(1, 11):
                  print(f"{int(v[8]):>{pw}}"
                        f"{int(v[8]*v[8]):>{pw}}")
              state = 50
          case 50:
              sys.exit(0)
          case _:
              print(f"UNDEF'D LINE {state}",
                    file=sys.stderr)
              sys.exit(1)

================================================================
 16. CURRENT LIMITATIONS
================================================================

The existing C89 backend has these known limitations:

  - Only one backend (C89) is currently implemented
  - Computed GOTO (ON X GOTO) uses switch dispatch
  - GOSUB/RETURN uses a manual stack + switch
  - Named variables (multi-char) map to A-Z only
  - No SUB/FUNCTION compilation (interpreter-only)
  - No error handler compilation (ON ERROR GOTO)
  - No file I/O compilation (OPEN/CLOSE/etc.)
  - No graphics/sound compilation
  - No FujiNet/virtual device compilation
  - No SELECT CASE compilation
  - No DO/LOOP compilation (WHILE/WEND only)

These limitations exist because codegen.c targets the
common subset of BASIC that maps cleanly to C89 goto-based
flow control. Adding more AST node handlers to codegen.c
(or to a new backend) would progressively close these gaps.

================================================================
 17. FUTURE DIRECTIONS
================================================================

Potential enhancements to the transpiler system:

  NEAR-TERM:
  - Add Pascal backend (most natural mapping)
  - Add Python 3 backend (most useful for modern users)
  - Support SUB/FUNCTION compilation
  - Support SELECT CASE and DO/LOOP
  - Support ON ERROR GOTO

  MEDIUM-TERM:
  - Add Java backend
  - Add JavaScript/Node.js backend
  - Add Fortran 90 backend
  - Add TypeScript backend
  - Optimization passes on the AST before codegen
  - Dead code elimination
  - Constant folding

  LONG-TERM:
  - Add 6502 assembly backend (Atari 8-bit native code)
  - Add Z80 assembly backend (TRS-80 native code)
  - Add ARM assembly backend
  - Add WebAssembly backend
  - Cross-compilation to FujiNet embedded targets
  - Source-level debugger integration

  SYNTAX ADDITION (planned):
    COMPILE "output.pas" TARGET "PASCAL"
    COMPILE "output.f90" TARGET "FORTRAN"
    COMPILE "Prog.java"  TARGET "JAVA"
    COMPILE "prog.py"    TARGET "PYTHON3"
    COMPILE "prog.asm"   TARGET "6502"

================================================================
 SUMMARY
================================================================

The BASIC++ transpiler architecture is designed for multiple
backends. The AST is completely language-neutral. Adding a
new target language requires:

  1. A new codegen_xxx.c file
  2. Runtime shim functions in the target language
  3. Expression and statement emitters
  4. A target identifier for the COMPILE command

The hardest problems are GOTO (use a state machine for
languages without it) and strings (each language handles
them differently). Everything else maps with varying degrees
of directness.

The existing C89 backend serves as both a working reference
and a template for new backends.

================================================================
 END OF DOCUMENT
================================================================
