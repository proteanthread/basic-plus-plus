# BASIC++ C17 Public API Master Reference Guide

## 1. Introduction & Header Inclusion

The primary public entry point for embedding or extending the BASIC++ engine is `bpp_api.h` or individual modular headers under `engine/include/`.

```c
#include "bpp_api.h"
```

In freestanding environments, include only the necessary subsystem headers:
```c
#include "core/boot.h"
#include "vm/vm.h"
#include "types/types.h"
```

## 2. Core Engine Lifecycle APIs

### `boot_init` / `boot_run` / `boot_shutdown`
Coordinates memory pool allocation, subsystem boot sequence, script execution, and graceful teardown.

```c
#include "core/boot.h"

/* Boot configuration struct */
typedef struct {
    size_t       memory_pool_size;  /* e.g. 671088640L for 640MB */
    const char  *script_filename;   /* Optional initial script to run */
    const char  *log_filename;      /* NULL unless --log is passed */
    LogLevel     log_level;         /* LOG_NONE by default */
    bool         interactive_repl;  /* true for baspp/bpp, false for bs */
    uint32_t     security_flags;    /* Initial capability bitmask */
} BppBootConfig;

/* Initializes VMContext, MemoryContext, and virtual devices */
VMContext *boot_init(const BppBootConfig *cfg);

/* Executes an inline BASIC source statement or multi-statement block */
BppError boot_eval_string(VMContext *vm, const char *basic_code);

/* Tears down all memory pools, closes files, and shuts down subsystems */
void boot_shutdown(VMContext *vm);
```

## 3. Tagged Union Value API (`BValue`)

All runtime values, expressions, function arguments, and return types are represented using the `BValue` tagged union.

```c
#include "types/types.h"

typedef enum {
    VAL_NULL    = 0,
    VAL_NUMBER  = 1,   /* double-precision floating point */
    VAL_STRING  = 2,   /* Reference-counted BppString pointer */
    VAL_ERROR   = 3    /* Runtime error code */
} BValueType;

typedef struct {
    BValueType type;
    union {
        double        number;
        BppString    *string;
        BppErrorCode  error;
    } as;
} BValue;

/* Helper constructors */
static inline BValue bval_number(double n) {
    BValue v; v.type = VAL_NUMBER; v.as.number = n; return v;
}

static inline BValue bval_error(BppErrorCode e) {
    BValue v; v.type = VAL_ERROR; v.as.error = e; return v;
}
```

## 4. Native Function & Statement Registration

### Registering Custom Built-in Functions
Native C functions are registered via `funcreg_register()` in `libcore`/`libengine`:

```c
#include "runtime/funcreg.h"

/* Function signature callback */
typedef BValue (*NativeFuncCallback)(VMContext *vm, int argc, BValue *argv, BppError *out_err);

/* Registration API */
void funcreg_register(
    const char         *name,           /* e.g. "MYFUNC" or "MYSTR$" */
    NativeFuncCallback  cb,             /* Pointer to C callback */
    int                 min_args,       /* Minimum argument count */
    int                 max_args,       /* Maximum argument count */
    const char         *description     /* Help text description */
);
```

### Registering Custom Statements
New statement handlers are bound to keyword tokens or identifier symbols:

```c
#include "stmt/stmt.h"

typedef BppError (*StmtHandler)(VMContext *vm, LexerContext *lex);

void stmt_register(
    const char         *keyword,        /* e.g. "RENDER", "TRANSACT" */
    StmtHandler         handler,        /* Statement parser callback */
    const char         *syntax,         /* Syntax signature */
    const char         *help_text       /* Interactive HELP summary */
);
```

## 5. Virtual Device Extension API (`VDev`)

Custom virtual devices implement the `VDev` interface to attach to the device bus:

```c
#include "device/vdev.h"

typedef struct VDev VDev;

typedef struct {
    bool (*init)(VDev *dev, void *user_data);
    bool (*open)(VDev *dev, const char *path, int mode);
    int  (*read)(VDev *dev, void *buf, size_t count);
    int  (*write)(VDev *dev, const void *buf, size_t count);
    int  (*getc)(VDev *dev);
    int  (*putc)(VDev *dev, int c);
    void (*flush)(VDev *dev);
    void (*close)(VDev *dev);
    void (*destroy)(VDev *dev);
} VDevOps;

struct VDev {
    const char *name;       /* e.g. "GPIO:", "CAN0:", "GPU:" */
    VDevOps     ops;        /* Operation function table */
    void       *priv_data;  /* Device-private state */
    bool        is_open;
};

/* Attach device to master device bus */
bool vdev_bus_attach(VDevContext *bus, VDev *device);
```

## 6. String Pool Reference Counting Rules

```c
#include "runtime/strings.h"

/* Allocation */
BppString *str_create(StringContext *sc, const char *data, size_t len);

/* Access */
const char *str_data(const BppString *s);
size_t      str_len(const BppString *s);

/* Retention & Release (Mandatory 2-Parameter Signature) */
BppString *str_retain(StringContext *sc, BppString *s);
void       str_release(StringContext *sc, BppString *s);
```
