# `TASK` Background Thread and Task Management Statement

## 1. BASIC Usage and Keyword Definition

Creates, inspects, and manages asynchronous background execution worker threads.

### Syntax Signatures:
```basic
TASK CREATE task_name$, entry_sub$
TASK KILL task_id%
TASK STATUS [task_id%]
```

### Operational Notes:
- Multi-threading engine abstraction.

---

## 2. Code Examples

```basic
10 TASK CREATE "Worker", "ComputePi"
20 TASK STATUS
```
