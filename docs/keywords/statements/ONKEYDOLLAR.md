# `ONKEYDOLLAR` Keypress Event Trapping Statement

## 1. BASIC Usage and Keyword Definition

Installs an asynchronous event trap handler for a specific keyboard character.

### Syntax Signatures:
```basic
ON KEY$(key_char$) GOSUB {line_number% | @label}
```

### Operational Notes:
- Event-driven programming model.

---

## 2. Code Examples

```basic
10 ON KEY$("Q") GOSUB 100
20 KEY("Q") ON
30 DO : LOOP
100 PRINT "Quit requested!": END
```
