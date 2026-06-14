# BASIC++ Scripting Functions

**Version 2.0.0**

BASIC++ includes shell-scripting capabilities that make it viable as a replacement for batch files and shell scripts.

---

## 1. SHELL Statement

Execute an OS command and wait for completion:

```basic
SHELL "dir"                 ' Run a command
SHELL "echo Hello"          ' Simple command
SHELL                       ' Open interactive shell
```

The exit code is captured in `ERRORLEVEL`:

```basic
SHELL "cmd /c exit 5"
PRINT ERRORLEVEL            ' prints 5
IF ERRORLEVEL = 0 THEN PRINT "Success"
```

---

## 2. SHELL$ Function

Capture the stdout of a command into a string:

```basic
A$ = SHELL$("echo Hello World")
PRINT A$                     ' prints "Hello World"

A$ = SHELL$("dir /b *.bas")
PRINT A$                     ' file listing

A$ = SHELL$("git status")
PRINT A$                     ' git output
```

Maximum capture size: **32 KB**. Trailing newlines are stripped. `ERRORLEVEL` is also set by `SHELL$`.

---

## 3. ERRORLEVEL

A pseudo-variable containing the exit code of the last `SHELL` or `SHELL$` command. Usable in any expression context:

```basic
SHELL "compile.bat"
IF ERRORLEVEL <> 0 THEN PRINT "Build failed!"
X = ERRORLEVEL * 10
```

---

## 4. EXEC Statement

Launch a program without waiting (fire-and-forget):

```basic
EXEC "notepad readme.txt"     ' opens Notepad, continues
EXEC "firefox http://example.com"
```

On Windows, uses `start`. On Unix, uses background (`&`).

---

## 5. SHELL Redirection

Redirect `SHELL` output to a file:

```basic
SHELL "dir /b" > "filelist.txt"          ' Overwrite
SHELL "echo entry" >> "log.txt"          ' Append
```

Pipe `SHELL` output through another command:

```basic
SHELL "dir /b" | "sort"
SHELL "type data.txt" | "findstr hello" | "sort"
```

Multi-pipe chains are supported. Each pipe stage runs sequentially.

---

## 6. PRINT Redirection

Redirect `PRINT` output to files or commands:

```basic
PRINT > "output.txt", "Hello World"       ' Overwrite
PRINT >> "log.txt", "Entry: "; TIME$       ' Append
PRINT | "sort", "cherry"; CHR$(10); "apple"  ' Pipe
```

This lets you generate files from BASIC without opening file channels:

```basic
10 FOR I = 1 TO 100
20   PRINT >> "data.csv", I; ","; I*I
30 NEXT I
```

Or pipe computed data to system commands:

```basic
10 PRINT | "mail -s 'Report' admin@co.com", REPORT$
```

---

## 7. ENVIRON$ Function

Read environment variables:

```basic
PRINT ENVIRON$("PATH")        ' system PATH
PRINT ENVIRON$("HOME")        ' home directory
PRINT ENVIRON$("USER")        ' current user

IF ENVIRON$("DEBUG") = "1" THEN TRON
```

---

## 8. Shebang Support

Make BASIC++ scripts directly executable on Unix/macOS:

```basic
#!/usr/bin/env basicpp
10 PRINT "Running as a script!"
20 PRINT "Time: "; TIME$
30 SYSTEM
```

```bash
$ chmod +x myscript.bas
$ ./myscript.bas
Running as a script!
Time: 14:30:45
```

The `#!` line is silently skipped during `LOAD`.

---

## 9. Scripting Patterns

### A. Build automation

```basic
10 SHELL "gcc -o app main.c util.c"
20 IF ERRORLEVEL <> 0 THEN PRINT "FAIL" : END
30 SHELL "./app"
40 PRINT "Exit code:"; ERRORLEVEL
```

### B. File processing

```basic
10 A$ = SHELL$("find . -name '*.log'")
20 ' Process each filename in A$
30 FOR I = 1 TO LEN(A$)
40   IF MID$(A$, I, 1) = CHR$(10) THEN PRINT "---"
50 NEXT I
```

### C. System monitoring

```basic
#!/usr/bin/env basicpp
10 WHILE 1
20   A$ = SHELL$("df -h /")
30   PRINT DATE$; " "; TIME$
40   PRINT A$
50   SHELL "sleep 60"
60 WEND
```

### D. Log aggregation

```basic
10 PRINT > "report.txt", "=== DAILY REPORT ==="
20 PRINT >> "report.txt", "Date: "; DATE$
30 PRINT >> "report.txt", ""
40 A$ = SHELL$("wc -l access.log")
50 PRINT >> "report.txt", "Hits: "; A$
```

---

## 10. Security Notes

`SHELL`, `SHELL$`, and `EXEC` execute real OS commands. In sandboxed mode (`SECURITY LEVEL 2+`), these commands are blocked.

See [Security](Security.md) for details on the security model.
See [Shell_Scripting_Integration](Shell_Scripting_Integration.md) for using BASIC++ inside bash scripts, batch files, and PowerShell scripts.
