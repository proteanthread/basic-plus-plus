# BASIC++ v6.5.2 Shell Scripting Integration

## 1. THE BATCH SCRIPT RUNNER (bs)

The bs executable is designed specifically for shell scripting integration. It runs a BASIC++ source file non-interactively, produces no banner or prompt, and exits with a numeric exit code:

```bash
bs script.bas
echo $?           # 0 = success, non-zero = error
```

Exit codes: 0 = success, 1 = runtime error, 2 = file not found, 3 = syntax error.

## 2. PIPELINE INTEGRATION

The bs runner connects to standard I/O streams. INPUT reads from stdin, PRINT writes to stdout, and error messages go to stderr:

```bash
# Filter: read numbers, output squares
echo -e "3\n4\n5" | bs square.bas

# Transform: process CSV data
cat data.csv | bs transform.bas > output.csv

# Chain: pipeline of BASIC++ scripts
bs generate.bas | bs filter.bas | bs format.bas > report.txt
```

The BASIC++ script for a filter:

```basic
' square.bas - Read numbers from stdin, output their squares
10 ON ERROR GOTO 100
20 WHILE 1
30   LINE INPUT X$
40   PRINT VAL(X$) ^ 2
50 WEND
100 END
```

## 3. THE SHELL STATEMENT

SHELL executes an operating system command from within a BASIC++ program:

```basic
10 SHELL "dir *.bas"          ' Windows
20 SHELL "ls -la *.bas"       ' Linux
```

SHELL with no argument opens an interactive command shell. Type EXIT to return to BASIC++.

SHELL captures the exit code in the ERRORLEVEL variable: `SHELL "command" : PRINT ERRORLEVEL`.

The EXEC statement is similar to SHELL but replaces the BASIC++ process with the specified command (on Linux, this is equivalent to the exec system call).

## 4. ENVIRONMENT VARIABLES

ENVIRON$("NAME") reads an environment variable:

```basic
10 Home$ = ENVIRON$("HOME")
20 Path$ = ENVIRON$("PATH")
30 User$ = ENVIRON$("USERNAME")
```

ENVIRON sets an environment variable for the current process: `ENVIRON "MYVAR=VALUE"`.

## 5. THE -c FLAG

The baspp and bpp executables accept the -c flag for executing a single statement from the command line, making them usable as command-line tools:

```bash
# Quick calculation
baspp -c "PRINT SQR(144)"

# System info
baspp -c "INFO"

# Date stamping
baspp -c "PRINT DATE$; \" \"; TIME$"
```

The -c flag is useful in shell scripts that need to perform a single BASIC++ operation:

```bash
#!/bin/bash
RESULT=$(baspp -c "PRINT 2^32")
echo "2^32 = $RESULT"
```

## 6. FILE OPERATIONS FROM SCRIPTS

BASIC++ scripts can perform file management operations that complement shell scripting:

```basic
' cleanup.bas - Delete temporary files older than 7 days
10 FILES "*.tmp"
20 ' Process each file...
30 KILL "old.tmp"
40 MKDIR "archive"
50 NAME "data.txt" AS "archive\data.txt"
```

## 7. CGI SCRIPTING

The bs runner can serve as a CGI script handler for web servers. The script reads the query string from ENVIRON$("QUERY_STRING") and writes HTTP headers and content to stdout:

```basic
' hello.bas - CGI script
10 PRINT "Content-Type: text/html"
20 PRINT
30 PRINT "<html><body>"
40 PRINT "<h1>Hello from BASIC++!</h1>"
50 Query$ = ENVIRON$("QUERY_STRING")
60 PRINT "<p>Query: "; Query$; "</p>"
70 PRINT "</body></html>"
```

Configure the web server to execute bs as the CGI handler for .bas files.

## 8. SECURITY IN SCRIPTING

When running scripts from automated systems (cron, scheduled tasks, CI/CD), set the security level to RESTRICTED or PARANOID to prevent scripts from accessing resources beyond their intended scope:

```bash
bs --security=4 untrusted_script.bas
```

Security level 4 (RESTRICTED) disables SHELL, file creation, and network access. Level 5 (PARANOID) restricts the script to pure computation with no I/O of any kind.

## 9. RETURN CODES AND ERROR REPORTING

BASIC++ programs can set the process exit code using the ERRORLEVEL variable or the SYSTEM statement with a numeric argument:

```basic
10 IF ProcessingFailed THEN SYSTEM 1
20 SYSTEM 0
```

SYSTEM 0 exits with success (exit code 0). SYSTEM n exits with code n. The default exit code after END is 0. The default after an unhandled error is 1.

Shell scripts can check the exit code:

```bash
bs validate.bas
if [ $? -ne 0 ]; then
    echo "Validation failed"
    exit 1
fi
```
