# BASIC++ v6.5.2 Quick Reference

## 1. PROGRAM ENTRY

Line numbers range from 1 to 65529. Maximum line length is 255 characters. Multiple statements per line are separated by colons. REM or apostrophe begins a comment.

## 2. IMMEDIATE COMMANDS

RUN executes the stored program. LIST displays program lines. LIST 100-200 displays a range. NEW clears the program and all variables. CONT continues after STOP or breakpoint. SYSTEM or BYE exits the interpreter. AUTO 100,10 begins automatic line numbering at 100 with step 10. RENUM renumbers lines. DELETE 100-200 removes a range. EDIT n edits line n interactively.

## 3. FILE COMMANDS

SAVE "file" writes the program to disk. LOAD "file" loads a program. MERGE "file" overlays lines without clearing. CHAIN "file" loads and runs, preserving COMMON variables. RUN "file" loads and runs, clearing variables. BSAVE "file" saves bytecode. BLOAD "file" loads bytecode. BRUN compiles and executes bytecode.

## 4. DATA TYPES

Numbers are double-precision floating point by default. Type suffixes: % (integer), ! (single), # (double), $ (string). Maximum string length is 255 characters. DEFINT, DEFSNG, DEFDBL, DEFSTR set default types by letter range.

## 5. OPERATORS

Arithmetic: + - * / \ (integer div) ^ (power) MOD. Comparison: = <> < > <= >=. Logical/Bitwise: AND OR NOT XOR EQV IMP. Shifts: SHL SHR. String: + (concatenation).

## 6. CONTROL FLOW

```basic
IF cond THEN stmt [ELSE stmt]
IF cond THEN / ELSEIF / ELSE / END IF
FOR var = start TO end [STEP n] / NEXT var
WHILE cond / WEND
DO [WHILE|UNTIL cond] / LOOP [WHILE|UNTIL cond]
SELECT CASE expr / CASE val / CASE ELSE / END SELECT
GOTO line
GOSUB line / RETURN
ON n GOTO line, line, ...
ON n GOSUB line, line, ...
EXIT FOR / EXIT DO
```

Postfix modifiers: PRINT "ok" IF A = 1. PRINT "no" UNLESS B > 0.

## 7. PROCEDURES

```basic
SUB Name(params) / END SUB
FUNCTION Name(params) / END FUNCTION
CALL Name(args)
DEF FN Name(params) = expression
DECLARE SUB Name(params)
```

LOCAL, STATIC, SHARED control variable scope.

## 8. ARRAYS

```basic
DIM A(100)
DIM Grid(10, 20)
OPTION BASE 0 | 1
REDIM A(200)
REDIM PRESERVE A(200)
ERASE A
LBOUND(A, 1) / UBOUND(A, 1)
```

## 9. STRING FUNCTIONS

LEN(s$), LEFT$(s$,n), RIGHT$(s$,n), MID$(s$,start,len), INSTR([start,]s$,find$), CHR$(n), ASC(s$), STR$(n), VAL(s$), HEX$(n), OCT$(n), BIN$(n), SPACE$(n), STRING$(n,char), UCASE$(s$), LCASE$(s$), TCASE$(s$), LTRIM$(s$), RTRIM$(s$), TRIM$(s$), REPLACE$(s$,find$,rep$), REVERSE$(s$).

## 10. MATH FUNCTIONS

ABS(n), SGN(n), INT(n), FIX(n), CINT(n), SQR(n), SIN(n), COS(n), TAN(n), ATN(n), ASIN(n), ACOS(n), SINH(n), COSH(n), TANH(n), LOG(n), LOG2(n), LOG10(n), EXP(n), PI, RND, ROUND(n,places), MIN(a,b), MAX(a,b), AVG(list), MED(list), CLAMP(n,lo,hi), FLOOR(n), CEIL(n).

## 11. I/O STATEMENTS

PRINT expr [; | ,] expr. INPUT ["prompt";] var. LINE INPUT var$. WRITE expr, expr. LOCATE row, col. COLOR fg [,bg]. CLS. WIDTH cols. BEEP. SOUND freq, duration. PLAY "music string".

## 12. FILE I/O

```basic
OPEN "file" FOR INPUT|OUTPUT|APPEND|RANDOM AS #n [LEN = reclen]
INPUT #n, var
PRINT #n, expr
LINE INPUT #n, var$
WRITE #n, expr
GET #n [, rec]
PUT #n [, rec]
CLOSE #n
EOF(n) / LOC(n) / LOF(n) / SEEK #n, pos
FIELD #n, width AS var$ [, width AS var$]
```

## 13. GRAPHICS

```basic
SCREEN mode
PSET (x,y) [,color]
PRESET (x,y)
LINE (x1,y1)-(x2,y2) [,color[,B|BF]]
CIRCLE (x,y),radius [,color]
PAINT (x,y) [,color[,border]]
DRAW "command string"
PALETTE attr, color
POINT(x,y)
VIEW (x1,y1)-(x2,y2)
WINDOW (x1,y1)-(x2,y2)
```

## 14. ERROR HANDLING

```basic
ON ERROR GOTO line
ON ERROR GOTO 0
RESUME / RESUME NEXT / RESUME line
ERR / ERL / ERR$
ERROR n
TRY / CATCH / END TRY / THROW n
```

## 15. EVENT TRAPPING

```basic
ON KEY(n) GOSUB line / KEY(n) ON|OFF|STOP
ON TIMER(n) GOSUB line / TIMER ON|OFF|STOP
ON COM(n) GOSUB line / COM(n) ON|OFF|STOP
ON PEN GOSUB line / PEN ON|OFF|STOP
ON STRIG(n) GOSUB line / STRIG(n) ON|OFF|STOP
ON PLAY(n) GOSUB line / PLAY ON|OFF|STOP
```

## 16. METAPROGRAMMING

```basic
ALIAS "newname" FOR "keyword"
ALIAS OPERATOR "newop" FOR "op"
OVERRIDE "keyword" WITH GOSUB line
SCOPE HOOK BEFORE|AFTER "keyword" GOSUB line
SCOPE RESTRICT "keyword"
SCOPE ENABLE "keyword"
```

## 17. DEBUGGING

TRON / TROFF. DEBUG ON / OFF. BREAK line. CONT. VARS. DUMP. BACKTRACE. ASSERT condition. TEST "name" / ENDTEST. SELFTEST. VERIFY condition.

## 18. SECURITY

SECURITY LEVEL 0-5. RESTRICT operation. Levels: 0=OPEN, 1=SAFE, 2=STANDARD, 3=EDUCATIONAL, 4=RESTRICTED, 5=PARANOID.

## 19. SYSTEM

SYSTEM exits to OS. SHELL "cmd" executes OS command. ENVIRON$("VAR") reads environment. INFO shows system info. VER shows version. HELP keyword shows documentation. CATALOG lists keywords.

## 20. ERROR CODES

1=NEXT without FOR, 2=Syntax error, 3=RETURN without GOSUB, 4=Out of DATA, 5=Illegal function call, 6=Overflow, 7=Out of memory, 8=Undefined line, 9=Subscript out of range, 10=Duplicate definition, 11=Division by zero, 12=Illegal direct, 13=Type mismatch, 14=Out of string space, 17=Cannot continue, 52=Bad file number, 53=File not found, 54=Bad file mode, 55=File already open, 62=Input past end, 70=Permission denied.
