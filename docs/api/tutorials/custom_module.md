# Tutorial: Building Dynamic Extension Modules in BASIC++

## 1. Overview

This tutorial demonstrates how to compile and package native C17 functions, statements, and devices into a standalone dynamic shared library module (`.dll` on Windows, `.so` on Linux) that can be loaded dynamically into the BASIC++ virtual machine using `MODULE LOAD` or `IMPORT`.

---

## 2. Module Anatomy & Lifecycle

A dynamic BASIC++ module exports a single standard descriptor struct named `bpp_module_export`:

```c
#include "module/module.h"
#include "runtime/funcreg.h"
#include "stmt/stmt.h"
#include "types/types.h"
#include <stdio.h>

static int crypto_mod_init(void *rt) {
    printf("[CryptoMod] Initializing Cryptographic Extensions...\n");
    /* Register functions and statements into the active VM */
    return 0; /* Success */
}

static void crypto_mod_cleanup(void) {
    printf("[CryptoMod] Cleaning up Cryptographic Extensions...\n");
}

/* Master Module Descriptor */
#ifdef _WIN32
__declspec(dllexport)
#endif
const BppModuleInfo bpp_module_export = {
    .name           = "CRYPTO_EXT",
    .version        = "1.0.0",
    .description    = "Native Cryptographic Hashing and RNG Extensions",
    .mod_class      = MOD_EXTENSION,
    .capabilities   = CAP_MATH | CAP_STRING | CAP_SYSTEM,
    .required_level = SEC_STANDARD,
    .init           = crypto_mod_init,
    .cleanup        = crypto_mod_cleanup
};
```

---

## 3. Implementing Module Functions & Statements

Inside the module, implement functions and register them inside `init()`:

```c
static BValue func_sha256_mock(BValue *args, int argc, void *rt) {
    (void)argc;
    VMContext *vm = (VMContext *)rt;
    StringContext *sc = vm_get_str(vm);

    if (args[0].type != VAL_STRING) {
        return bval_error(13); /* Type mismatch */
    }

    /* Mock SHA-256 hex digest */
    const char *mock_hash = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";
    BppStringRef hash_str = str_create(sc, mock_hash, 64);

    BValue res;
    res.type = VAL_STRING;
    res.as.string = hash_str;
    return res;
}

static int crypto_mod_init(void *rt) {
    FunctionEntry sha_entry = {
        .name         = "SHA256$",
        .category     = FCAT_UTIL,
        .ret_type     = FRET_STRING,
        .min_args     = 1,
        .max_args     = 1,
        .safety       = FSAFE_PURE,
        .handler      = func_sha256_mock,
        .help_text    = "Computes SHA-256 cryptographic digest of string.",
        .module_name  = "CRYPTO_EXT"
    };

    funcreg_register(&sha_entry);
    return 0;
}
```

---

## 4. Compiling the Shared Library

### Linux / POSIX (GCC / Clang):
```bash
gcc -std=c17 -O2 -fPIC -shared -I../../engine/include crypto_mod.c -o libcrypto_ext.so
```

### Windows (MSVC):
```cmd
cl /std:c17 /O2 /LD /I..\..\engine\include crypto_mod.c /Fe:crypto_ext.dll
```

---

## 5. Loading and Using the Module from BASIC++

```basic
10 PRINT "Loading dynamic cryptographic module..."
20 MODULE LOAD "crypto_ext"
30 LET MSG$ = "Hello, BASIC++"
40 PRINT "Message: "; MSG$
50 PRINT "SHA-256: "; SHA256$(MSG$)
60 UNLOAD "crypto_ext"
70 PRINT "Module unloaded cleanly."
```
