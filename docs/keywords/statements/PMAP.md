# `PMAP` Coordinate Viewport Mapping Function

## 1. BASIC Usage and Keyword Definition

Translates coordinates between physical screen pixel coordinates and logical WINDOW Cartesian coordinates.

### Syntax Signatures:
```basic
physical_coord = PMAP(window_coord, mode%)
window_coord = PMAP(physical_coord, mode%)
```

### Error Handling & Boundary Conditions:
- **Error 5 (ERR_ILLEGAL_FUNCTION_CALL)**: Mode outside range (0..3).

### Operational Notes:
- Modes: 0 (Window X -> Phys X), 1 (Window Y -> Phys Y), 2 (Phys X -> Window X), 3 (Phys Y -> Window Y).

---

## 2. Code Examples

```basic
10 SCREEN 12
20 WINDOW (-10, -10)-(10, 10)
30 PRINT "Physical X for Window X=0: "; PMAP(0, 0)
```
