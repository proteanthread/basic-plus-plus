<!--
Title:        Set_Based_Object_Model_And_Python3_Import
Tier:         4
Applies to:   BASIC++ v6.5.2
Authority:    superseded document
Generated:    no, hand-written
Status:       superseded
-->

# Set-Based Object Model and Python 3 Import (superseded)

This document has been split, because it covered two unrelated subjects under
one filename.

**The object model** is now `Set_Based_Object_Model`: the
`BLOCK { SET [ GROUP ( OBJECT ) ] }` model, three levels standard and four the
ceiling, with the set and group algebra, path addressing, projection, and the
Pick MultiValue attribute-value-subvalue layer. Written against
`engine/include/runtime/set.h`, `set_core.c`, `set_ops.c` and `set_pick.c`.

**Python interoperation** is now covered in two places.
`Python_Programmers_Guide` has the language mapping — how a Python programmer
should think about BASIC++ types, scoping and collections. `Language_Interop`
section 5 has the mechanism, and the honest verdict on it: calling BASIC++
from Python works today through `ctypes`; calling Python from BASIC++ does
not, because `PYTHON$` returns the literal string `"None"`.

## See also

- `Set_Based_Object_Model` — the object model
- `Python_Programmers_Guide` — the language mapping
- `Language_Interop` — the interop mechanisms and their state
- `Implementation_Status`
