# `boardgame` Turn-Based Board Game & Grid Engine

## 1. Library Overview & Usage

The `boardgame` library provides 2D board representations, turn management, piece movement validators, and minimax AI search templates (Chess, Checkers, Reversi, Tic-Tac-Toe).

### Features:
- **Board Matrices**: $8\times 8$ or arbitrary $N\times M$ grid management.
- **Turn Rotation**: Player 1 / Player 2 / AI alternating state machine.

---

## 2. BASIC Example

```basic
10 DIM BOARD(8, 8)
20 BOARD(4, 4) = 1 : BOARD(4, 5) = 2 : REM Initial Reversi setup
30 PRINT "Board initialized."
```
