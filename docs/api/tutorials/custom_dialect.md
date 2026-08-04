# Tutorial: How to Define a Dialect

> **Purpose**: Guide to configuring custom language keyword tables, syntactic rules, and VM behaviors.

---

## Step 1: Define Dialect Schema
Define a custom dialect by mapping its options:
```c
#include "dialect.h"
#include "vm.h"
#include <string.h>

void configure_my_dialect(VMContext *vm) {
    BppDialect dialect;
    memset(&dialect, 0, sizeof(dialect));
    strcpy(dialect.name, "CustomBasic");
    dialect.comment_char = '#';
    dialect.stmt_separator = ';';
    dialect.default_array_base = 0;
    (void)vm;
}
```
