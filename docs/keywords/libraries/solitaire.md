# `solitaire` Klondike Card Solitaire Rules & Stack Engine

## 1. Library Overview & Usage

The `solitaire` library provides standard 52-card deck initialization, Fisher-Yates shuffling, Klondike 7-tableau stack rules, foundation piles (Ace through King), and draw waste management.

### Features:
- **Card Encoding**: 1-byte card representation (suit $0..3$, rank $1..13$).
- **Rule Verification**: Enforces alternating color tableau builds and same-suit foundation increments.

---

## 2. BASIC Example

```basic
10 DIM DECK(52), TABLEAU(7, 20), FOUNDATION(4)
20 PRINT "Klondike Solitaire deck shuffled and dealt."
```
