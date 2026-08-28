# `security` Capability Sandbox & Permission Model (`libkernel`)

## 1. Architectural Purpose & Overview

The `security` subsystem (`engine/src/security/security.c`) implements a fine-grained capability-based sandbox enforcing security constraints on file I/O, network access, raw memory mutation, and OS command execution.

### Key Architectural Invariants:
- **Capability Bitmasks**: `CAP_NONE`, `CAP_IO`, `CAP_FS`, `CAP_NET`, `CAP_MEM`, `CAP_SYS`.
- **Security Levels**: `SEC_UNRESTRICTED`, `SEC_STANDARD`, `SEC_SANDBOXED`, `SEC_STRICT`, `SEC_PARANOID`.
- **Permission Denied Error**: Unauthorized operations immediately return Error 70 (`ERR_PERMISSION_DENIED`).

---

## 2. Technical API Signatures (C17)

```c
int security_check(SecurityOperation op, int param);
void security_set_level(SecurityLevel level);
SecurityLevel security_get_level(void);
```
