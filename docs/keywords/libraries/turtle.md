# `turtle` Logo-Style Turtle Graphics Engine Library

## 1. Library Overview & Usage

The `turtle` graphics library implements Logo-style turtle vector drawing primitives (`FORWARD`, `BACK`, `LEFT`, `RIGHT`, `PENUP`, `PENDOWN`, `PENCOLOR`) in BGI.

### Features:
- **Heading Mathematics**: Angle rotation and trigonometry for precise vector step movements.
- **Fractal Generation**: Easily constructs Koch snowflakes, Sierpinski triangles, and dragon curves.

---

## 2. BASIC Example

```basic
10 SET SCREEN 320, 200, 8
20 REM Draw a square with turtle
30 FOR I = 1 TO 4
40   TURTLE.FORWARD 50
50   TURTLE.RIGHT 90
60 NEXT I
```
