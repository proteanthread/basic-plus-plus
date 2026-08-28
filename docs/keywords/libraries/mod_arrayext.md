# `mod_arrayext` Extended Array Operations Module (`libflex`)

## 1. Architectural Purpose & Overview

The `mod_arrayext` module provides advanced array manipulation algorithms (sorting, binary searching, reversing, slicing, filtering) in BASIC++.

### Key Features:
- **Array Sorting**: Fast in-place quicksort / introsort (`ARRAY.SORT`).
- **Binary Search**: Fast logarithm search over sorted arrays (`ARRAY.SEARCH%`).
- **Slicing & Copy**: Block memory copy between arrays (`ARRAY.COPY`).

---

## 2. Technical API Signatures (C17)

```c
void arrayext_sort(BppArray *arr, bool ascending);
int arrayext_search(BppArray *arr, BValue target);
void arrayext_reverse(BppArray *arr);
```
