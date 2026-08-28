# BASIC++ v6.5.2 System and Environment

## 1. SYSTEM INFORMATION

INFO displays the complete system configuration including version, build target, dialect, memory profile, security level, and available subsystems:

```basic
> INFO
BASIC++ Standard Edition v6.5.2 "Phoenix"
Build: baspp (Standard Console & SDL Combined)
Dialect: GWBS (GW-BASIC Compatible)
Memory: MODERN (640 MB)
Security: OPEN (Level 0)
Platform: Windows x64
Keywords: 367
Errors: 43 codes
Libraries: 12
Modules: 3 loaded
```

VER or VERSION displays the version string: `BASIC++ v6.5.2`. VER$ returns the version as a string for use in expressions: `IF VER$ >= "6.5" THEN ...`.

## 2. ENVIRONMENT VARIABLES

ENVIRON$("NAME") reads an environment variable from the host operating system:

```basic
10 PRINT "Home: "; ENVIRON$("HOME")
20 PRINT "User: "; ENVIRON$("USERNAME")
30 PRINT "Path: "; ENVIRON$("PATH")
40 PRINT "Temp: "; ENVIRON$("TEMP")
```

ENVIRON "NAME=VALUE" sets an environment variable for the current process and its children:

```basic
10 ENVIRON "MYAPP_CONFIG=production"
20 SHELL "myapp.exe"
```

The change affects only the current process. It does not modify the system environment permanently.

## 3. SYSTEM FUNCTIONS

HOSTNAME$ returns the machine's network hostname.

USERNAME$ returns the current user's login name.

PATH$ returns the system PATH environment variable.

PWD$ returns the current working directory.

DIALECT$ returns the active dialect name ("GWBS", "QBAS", "SBAS", "E116", etc.).

MEMMAP$ returns the active memory profile ("MODERN", "LITE", "FREEDOS", "EMBEDDED").

CLOCK$ returns a full timestamp string.

## 4. DATE AND TIME

DATE$ returns the current date as "MM-DD-YYYY":

```basic
10 PRINT DATE$           ' e.g., "08-15-2026"
```

TIME$ returns the current time as "HH:MM:SS":

```basic
10 PRINT TIME$           ' e.g., "14:30:45"
```

TIMER returns the number of seconds elapsed since midnight as a double-precision value:

```basic
10 Start = TIMER
20 ' ... do work ...
30 Elapsed = TIMER - Start
40 PRINT "Elapsed:"; Elapsed; "seconds"
```

TICKS returns the system tick count (platform-dependent resolution).

DATE$ and TIME$ can be assigned to set the system clock (requires appropriate OS permissions and security level 0 or 1):

```basic
10 DATE$ = "12-25-2026"
20 TIME$ = "00:00:00"
```

## 5. PROCESS EXIT

SYSTEM exits the interpreter. SYSTEM n exits with the specified exit code. BYE is an alias for SYSTEM 0.

```basic
10 IF ErrorOccurred THEN SYSTEM 1
20 SYSTEM 0
```

ERRORLEVEL contains the exit code of the last SHELL command:

```basic
10 SHELL "command"
20 IF ERRORLEVEL > 0 THEN PRINT "Command failed"
```

## 6. THE SHELL COMMAND

SHELL "command" executes an OS command and returns when it completes:

```basic
10 SHELL "dir *.bas"            ' Windows
20 SHELL "ls -la *.bas"         ' Linux
```

SHELL with no argument opens an interactive OS shell. Type EXIT to return to BASIC++.

SHELL is denied at security levels 2 and above.

## 7. EXEC STATEMENT

EXEC has two behaviors depending on context:

Inside a program, EXEC "string" interprets the string as if typed at the BASIC++ prompt. If the string has a line number, the line is stored. If not, it is executed immediately. This is the self-programming facility (see Self_Programming.md).

As a process operation, EXEC replaces the BASIC++ process with the specified command (on Unix, this is the exec system call). Unlike SHELL, EXEC does not return.

## 8. RANDOM NUMBERS

RANDOMIZE seeds the pseudo-random number generator. RANDOMIZE TIMER uses the current time (common pattern for non-reproducible sequences). RANDOMIZE n uses a specific seed for reproducible sequences.

RND returns the next pseudo-random number in the range [0, 1). RND(0) repeats the last number. RND(n) where n < 0 seeds the generator.

```basic
10 RANDOMIZE TIMER
20 FOR I = 1 TO 6
30   DiceRoll = INT(RND * 6) + 1
40   PRINT DiceRoll;
50 NEXT I
```

## 9. SLEEP AND DELAY

SLEEP n pauses for n seconds. SLEEP with no argument waits for a keypress. DELAY n pauses for n milliseconds. PAUSE displays a prompt and waits for a keypress.
