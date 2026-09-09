<!--
Title:        Security
Tier:         1
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/security/security.c, engine/include/security/security.h
Generated:    no, hand-written
Status:       current
-->

# BASIC++ v6.5.2 Security Architecture & Secure Coding Practices

The authoritative specification of the BASIC++ six-level virtual execution sandbox, access control enforcement mechanisms, command-line security flags, and defensive secure coding patterns for application developers.

---

## 1. Architectural Overview & Virtual Device Enforcement

BASIC++ implements a unified six-level security sandbox (`engine/src/security/security.c` in `libkernel`). Rather than scattering ad-hoc security checks across hundreds of individual keyword handlers, security enforcement is anchored at the virtual hardware layer:
- **Virtual Filesystem (VFS)**: Intercepts path resolution, restricting directory traversal and file write operations (`engine/src/runtime/vfs.c`).
- **Virtual Network (VNet)**: Blocks socket allocation and network listening (`engine/src/runtime/vnet.c`).
- **Virtual Device Bus (VDev)**: Controls peripheral bindings, hardware port access, and system calls (`engine/src/device/vdev.c`).

---

## 2. The Six Security Levels

| Level | Identifier | Name | Operational Capabilities & Sandbox Boundary |
|:---|:---|:---|:---|
| 0 | `SEC_OPEN` | OPEN | Unrestricted host access. All operations permitted. |
| 1 | `SEC_SAFE` | SAFE | Safe development. File deletions restricted to CWD. |
| 2 | `SEC_STANDARD` | STANDARD | Sandboxed runtime. File writes confined to CWD. No `SHELL`. |
| 3 | `SEC_EDUCATIONAL` | EDUCATIONAL | Classroom mode. Read/write in CWD only. No file creation or `SHELL`. |
| 4 | `SEC_RESTRICTED` | RESTRICTED | Highly isolated. No file I/O, no networking, no `POKE`/`OUT`. |
| 5 | `SEC_PARANOID` | PARANOID | Pure mathematical computation. All I/O strictly prohibited. |

### Enforcing Security from the Host Command Line

```bash
baspp --security=2              # Launch desktop edition in Standard sandbox
bpp --security=4                # Launch REPL in Restricted mode
bs --security=3 student.bas     # Execute script in Educational classroom sandbox
```

### Raising Security from Within BASIC Code

```basic
10 SECURITY 3                   ' Elevate sandbox to Educational level
```

**Irreversible Privilege Escalation Guard**: The `SECURITY` statement can only raise the security level, never lower it. Once a level is elevated, it cannot be reduced without terminating and restarting the interpreter process.

---

## 3. Access Control Enforcement Matrix

| Operation Category | Level 0 | Level 1 | Level 2 | Level 3 | Level 4 | Level 5 |
|:---|:---:|:---:|:---:|:---:|:---:|:---:|
| Console Terminal Output (`PRINT`) | Yes | Yes | Yes | Yes | Yes | No |
| Keyboard Input (`INPUT`, `INKEY$`) | Yes | Yes | Yes | Yes | Yes | No |
| File Read (System-Wide Paths) | Yes | Yes | Yes | No | No | No |
| File Read (CWD Only) | Yes | Yes | Yes | Yes | No | No |
| File Write (System-Wide Paths) | Yes | Yes | No | No | No | No |
| File Write (CWD Only) | Yes | Yes | Yes | Yes | No | No |
| File Creation | Yes | Yes | Yes | No | No | No |
| File Deletion (`KILL`) | Yes | CWD | No | No | No | No |
| Network Socket Connections (`NET`) | Yes | Yes | Yes | No | No | No |
| Network Listen / Server (`LISTEN`) | Yes | Yes | No | No | No | No |
| Subprocess Execution (`SHELL`) | Yes | Yes | No | No | No | No |
| Direct Memory / Port Access (`POKE`) | Yes | Yes | Yes | Yes | No | No |
| Dynamic Module Loading (`MODULE`) | Yes | Yes | Yes | No | No | No |

---

## 4. Defensive Secure Coding Practices

Application developers writing robust BASIC++ scripts should adhere to these defensive programming standards:

### Strict Input Validation & Length Bounds

Never trust unvalidated user input. Check string lengths, character sets, and numeric bounds before processing:

```basic
10 LINE INPUT "Enter Username (max 20 characters): "; U$
20 IF LEN(U$) = 0 OR LEN(U$) > 20 THEN
30   PRINT "Error: Username must be between 1 and 20 characters."
40   GOTO 10
50 END IF
```

### Path Traversal Defense

When building filenames from untrusted user input, sanitize directory separators (`/`, `\`) and parent directory references (`..`):

```basic
100 FUNCTION SanitizeFilename$(raw$)
110   clean$ = ""
120   FOR I = 1 TO LEN(raw$)
130     C$ = MID$(raw$, I, 1)
140     IF C$ <> "/" AND C$ <> "" AND C$ <> ":" AND C$ <> ".." THEN
150       clean$ = clean$ + C$
160     END IF
170   NEXT I
180   SanitizeFilename$ = clean$
190 END FUNCTION
```

### Preventing Command Injection

Never concatenate untrusted input directly into `SHELL` strings. Always validate against an explicit whitelist of allowed parameters:

```basic
10 INPUT "Select Option [1-3]: "; Opt%
20 IF Opt% < 1 OR Opt% > 3 THEN PRINT "Invalid choice." : END
30 ' Pass strictly validated ordinal integers rather than raw input
40 SHELL "process_task --mode=" + STR$(Opt%)
```
