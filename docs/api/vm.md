# C17 API Reference: Virtual Machine Engine Core (`vm/vm.h`)

## 1. Subsystem Overview & Responsibilities

The Virtual Machine Engine Core Subsystem (`vm/vm.h`, implemented in `engine/src/vm/vm.c`) provides the central execution loop, program counter management, interpreter-managed heap stacks, error trapping state (`ON ERROR GOTO`), structured call frames (`GOSUB`, `FOR/NEXT`, `WHILE/WEND`, `DO/LOOP`, `SUB`), and subsystem context aggregation for the BASIC++ v6.5.2 engine.

Key architectural responsibilities include:
- **Strict Non-Recursive Execution Loop**: All control flow stacks (evaluation stack, call frames, loop contexts) reside strictly on the interpreter heap, completely eliminating host C stack recursion.
- **Context Aggregator & Accessors**: Serves as the central anchor for all sub-contexts via fast inlined accessors:
  - `vm_get_mem(vm)`: `MemoryContext *`
  - `vm_get_str(vm)`: `StringContext *`
  - `vm_get_var(vm)`: `VariableContext *`
  - `vm_get_arr(vm)`: `ArrayContext *`
  - `vm_get_file(vm)`: `FileContext *`
  - `vm_get_vdev(vm)`: `VDevContext *`
  - `vm_get_vcon(vm)`: `VConContext *`
  - `vm_get_vfs(vm)`: `VfsContext *`
- **Execution Lifecycle & Boundaries**: Manages execution state transitions (`vm_run_program()`, `vm_execute_line()`, `vm_reset_for_run()`).
- **Error Trapping & Stack Unwinding**: Records active error codes (`vm_set_error()`), unwinding frames during `ON ERROR GOTO` traps.

## 2. Header Inclusion & Prerequisites

```c
#include "vm/vm.h"
#include "memory/memory.h"
#include "runtime/strings.h"
#include "runtime/variables.h"
#include "device/vdev.h"
```

## 3. Data Structures & Types

```c
/* Opaque Handle to Virtual Machine Context */
typedef struct VMContext VMContext;

/* Subroutine / Procedure Call Stack Frame */
typedef struct BppSubFrame {
    BppLineNumber       return_line;    /* Line to resume after RETURN / END SUB */
    const char         *return_pos;     /* Character pointer into line text */
    char                proc_name[64];  /* Procedure identifier */
    bool                is_proc;        /* True for SUB/FUNCTION, false for GOSUB */
    struct BppSubFrame *prev;           /* Previous frame on heap stack */
} BppSubFrame;
```

## 4. Function Prototypes & Operational Contracts

### Context Lifecycle
```c
VMContext *vm_init(MemoryContext *mem, StringContext *str, VariableContext *var, VDevContext *vdev);
void       vm_shutdown(VMContext *vm);
```

### Execution Control
```c
/**
 * @brief Runs the stored BASIC program from the first line number.
 */
void vm_run_program(VMContext *vm);

/**
 * @brief Executes a single line of BASIC code immediately.
 */
BppError vm_execute_line(VMContext *vm, const char *line_text);

/**
 * @brief Resets VM execution registers, stacks, and variables for a clean RUN.
 */
void vm_reset_for_run(VMContext *vm);

bool vm_is_running(VMContext *vm);
void vm_set_running(VMContext *vm, bool running);
```

### Context Accessor APIs
```c
MemoryContext   *vm_get_mem(VMContext *vm);
StringContext   *vm_get_str(VMContext *vm);
VariableContext *vm_get_var(VMContext *vm);
ArrayContext    *vm_get_arr(VMContext *vm);
FileContext     *vm_get_file(VMContext *vm);
VDevContext     *vm_get_vdev(VMContext *vm);
VConContext     *vm_get_vcon(VMContext *vm);
VfsContext      *vm_get_vfs(VMContext *vm);
```

### Error State APIs
```c
void         vm_set_error(VMContext *vm, int error_code, const char *msg);
int          vm_get_error(VMContext *vm);
const char  *vm_get_error_msg(VMContext *vm);
void         vm_clear_error(VMContext *vm);
```

## 5. Architectural Invariants

- **Zero Host Recursion**: Evaluator and execution loops must NEVER call themselves recursively in C.
- **Dialect-Agnostic Core**: Dialects configure syntax rules at the parser level; the VM execution core executes agnostic bytecode and AST nodes.

## 6. Code Example: Executing Script Lines in C

```c
#include "vm/vm.h"
#include "core/boot.h"
#include <stdio.h>

void execute_basic_snippet(void) {
    VMContext *vm = boot_system(671088640L);
    if (vm) {
        vm_execute_line(vm, "A = 10 : B = 20");
        vm_execute_line(vm, "PRINT \"SUM: \"; A + B");
        boot_shutdown_vm(vm);
    }
}
```
