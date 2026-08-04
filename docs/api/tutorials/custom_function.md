# Tutorial: How to Add a Built-in Function

> **Purpose**: Guide to writing built-in expressions functions and registering them in the function evaluator.

---

## Step 1: Implement Function Handler
Implement the function signature taking values and returning a result:
```c
#include "funcreg.h"

BValue fn_my_func(BValue *args, int argc, void *rt) {
    (void)rt;
    if (argc < 1 || args[0].type != VAL_NUMBER) {
        return bval_float(0.0);
    }
    double val = args[0].as.number;
    return bval_float(val * val);
}
```
