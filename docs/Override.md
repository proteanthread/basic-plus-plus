# BASIC++ v6.5.2 Override System

## 1. THE OVERRIDE STATEMENT

OVERRIDE replaces the behavior of an existing keyword with a user-defined subroutine. Unlike ALIAS (which adds a synonym), OVERRIDE changes what the keyword does:

```basic
10 OVERRIDE "PRINT" WITH 5000
20 PRINT "Hello!"                ' Now calls the subroutine at line 5000
30 END
5000 ' Custom PRINT handler
5010 ' The original arguments are available through OVERRIDE.PARAM$
5020 PRINT.ORIGINAL UCASE$(OVERRIDE.PARAM$)
5030 RETURN
```

After line 10, every PRINT statement in the program calls the subroutine at line 5000 instead of the built-in PRINT handler.

## 2. PRINT.ORIGINAL

When a keyword is overridden, the original behavior is still accessible through the .ORIGINAL suffix:

```basic
5020 PRINT.ORIGINAL ">>> " + OVERRIDE.PARAM$ + " <<<"
```

This allows the override handler to wrap the original behavior with additional logic (logging, formatting, filtering) rather than completely replacing it.

## 3. USE CASES

### Logging

```basic
10 OVERRIDE "OPEN" WITH 8000
8000 ' Log all file opens
8010 PRINT.ORIGINAL #9, TIME$; " OPEN: "; OVERRIDE.PARAM$
8020 OPEN.ORIGINAL OVERRIDE.PARAM$
8030 RETURN
```

### Input Validation

```basic
10 OVERRIDE "INPUT" WITH 8100
8100 ' Validate all numeric input
8110 INPUT.ORIGINAL OVERRIDE.PARAM$
8120 IF VAL(OVERRIDE.RESULT$) < 0 THEN
8130   PRINT "Negative values not allowed"
8140   GOTO 8100
8150 END IF
8160 RETURN
```

### Teaching Mode

```basic
10 OVERRIDE "GOTO" WITH 8200
8200 PRINT "Warning: GOTO is considered harmful!"
8210 PRINT "Consider using structured constructs instead."
8220 GOTO.ORIGINAL OVERRIDE.PARAM$
8230 RETURN
```

## 4. OVERRIDE SCOPE

An override is global — it affects all subsequent executions of the overridden keyword throughout the program. Overrides persist until explicitly removed or until NEW clears the program.

## 5. REMOVING OVERRIDES

OVERRIDE "PRINT" RESET removes the override and restores the original built-in behavior. OVERRIDE RESET with no keyword argument removes all overrides.

## 6. LISTING OVERRIDES

OVERRIDE with no arguments lists all active overrides:

```basic
> OVERRIDE
Active overrides:
  PRINT -> line 5000
  OPEN  -> line 8000
```

## 7. RESTRICTIONS

Not all keywords can be overridden. Structural keywords (IF, THEN, ELSE, END IF, FOR, NEXT, WHILE, WEND, DO, LOOP, SELECT, CASE) cannot be overridden because they are parsed at a syntactic level before statement dispatch. The overridable keywords are those that are dispatched through the statement registry: PRINT, INPUT, OPEN, CLOSE, GOTO, GOSUB, RUN, LOAD, SAVE, and similar action statements.

## 8. IMPLEMENTATION

OVERRIDE is implemented in engine/src/statements/dialect/override.c. It works by replacing the function pointer in the statement registry for the target keyword. The original function pointer is saved so that .ORIGINAL can invoke it. When RESET is called, the original pointer is restored.
