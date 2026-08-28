# `RANDOMIZE` Random Number Generator Seeding Statement

## 1. BASIC Usage and Keyword Definition

The `RANDOMIZE` statement re-seeds the pseudo-random number generator used by the `RND` function.

### Syntax Signatures:
```basic
RANDOMIZE [seed_value%]
RANDOMIZE TIMER
```

### Operational Rules:
- If `seed_value%` is supplied, seeds the PRNG deterministically with that value.
- If `TIMER` is supplied or no argument is given, prompts the user or seeds from the system clock.

---

## 2. Code Examples

```basic
10 RANDOMIZE TIMER
20 PRINT "Random number: "; RND
```
