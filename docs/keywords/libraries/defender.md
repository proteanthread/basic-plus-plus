# `defender` Horizontal Scrolling Radar & Arcade Game Template

## 1. Library Overview & Usage

The `defender` game template implements continuous wraparound horizontal scrolling, minimap / radar scanner rendering, and thrust physics.

### Features:
- **Minimap Radar**: Scaled top-screen scanner tracking player and lander positions.
- **Wraparound World**: Seamless circular looping world coordinate space.

---

## 2. BASIC Example

```basic
10 SET SCREEN 320, 200, 8
20 LINE (0, 20)-(319, 20), 7 : REM Radar boundary
30 PRINT "Defender radar active."
```
