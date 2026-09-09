<!--
Title:        OOP_Architecture
Tier:         1
Applies to:   BASIC++ v6.5.2
Authority:    engine/include/runtime/set.h
Generated:    no, hand-written
Status:       superseded by Set_Based_Object_Model
-->

# OOP Architecture

**This document is superseded by `Set_Based_Object_Model`.**

It previously described a conventional class-and-object model that BASIC++
does not implement. The engine implements a set-based model in which the
containment is `BLOCK { SET [ GROUP ( OBJECT ) ] }`, four levels by
convention with three as the floor, realised in `BppSet` and `BppGroup` with
arbitrary-depth path addressing.

Nothing in the old text was retained, because none of it described the code.
The types it referred to do not exist in `engine/include/runtime/set.h`.

## Where to go instead

| You want | Read |
|---|---|
| The object model itself | `Set_Based_Object_Model` |
| What OOP capabilities exist, and what do not | `Set_Based_Object_Model`, section 9 |
| Why sets rather than objects, with the costs | `Set_Based_Object_Model`, section 10 |
| The delimiters | `Delimiter_And_Bracket_Keyword_Taxonomy` |
| Records and user types | `User_Defined_Types` |
