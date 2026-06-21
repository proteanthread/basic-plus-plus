# Using Aliases in BASIC++

**Version 4.0.1**


---

## Table of Contents

- Syntax
- How It Works
- Managing Aliases
- Alias for String Functions
- Use Cases
  - Localization
  - Shorthand
  - Compatibility
  - Teaching
- Limitations

---

The `ALIAS` command lets you redefine keywords, creating custom names for built-in commands and functions.

---

## 1. Syntax

```basic
ALIAS "newname" = KEYWORD
```

**Examples:**

```basic
ALIAS "IMPRIMER" = PRINT       ' French PRINT
ALIAS "IMPRIME" = PRINT        ' Portuguese PRINT
ALIAS "EINGABE" = INPUT        ' German INPUT
ALIAS "ESCRIBE" = PRINT        ' Spanish PRINT
ALIAS "WENN" = IF              ' German IF
ALIAS "DANN" = THEN            ' German THEN
ALIAS "P" = PRINT              ' Shorthand
ALIAS "?" = PRINT              ' Shorthand (if not built-in)
```

---

## 2. How It Works

Aliases are processed by the **lexer**, not the parser. When the lexer encounters an identifier, it checks the alias table **before** the keyword table. If the identifier matches an alias, the alias's target keyword is substituted.

This means aliases work everywhere: statements, expressions, function calls, and conditions.

```basic
ALIAS "SCHREIB" = PRINT
ALIAS "WENN" = IF
ALIAS "DANN" = THEN

10 SCHREIB "Hallo Welt!"
20 WENN 1 = 1 DANN SCHREIB "Ja!"
```

This runs exactly as if you had written:

```basic
10 PRINT "Hallo Welt!"
20 IF 1 = 1 THEN PRINT "Ja!"
```

---

## 3. Managing Aliases

| Command | Description |
|---------|-------------|
| `ALIAS LIST` | List all active aliases |
| `ALIAS CLEAR "newname"` | Remove a specific alias |
| `ALIAS CLEAR ALL` | Remove all aliases |

Aliases persist across `RUN` and `NEW`. They are session-wide.

---

## 4. Alias for String Functions

Aliases work with `$` functions too:

```basic
ALIAS "GAUCHE" = LEFT$         ' French LEFT$
ALIAS "DROITE" = RIGHT$        ' French RIGHT$
ALIAS "MILIEU" = MID$          ' French MID$
ALIAS "LONGUEUR" = LEN         ' French LEN

10 A$ = "Bonjour le monde"
20 PRINT GAUCHE$(A$, 7)         ' prints "Bonjour"
```

> **Note:** The `$` is automatically handled when the alias target is a `$`-function keyword.

---

## 5. Use Cases

### A. Localization

Create a fully localized BASIC environment:

```basic
ALIAS "AFFICHER" = PRINT
ALIAS "SAISIR" = INPUT
ALIAS "ALLER" = GOTO
ALIAS "SI" = IF
ALIAS "ALORS" = THEN
ALIAS "SINON" = ELSE
ALIAS "FIN" = END
ALIAS "POUR" = FOR
ALIAS "A" = TO
ALIAS "SUIVANT" = NEXT

10 POUR I = 1 A 10
20   AFFICHER I
30 SUIVANT I
```

### B. Shorthand

Create abbreviations for common commands:

```basic
ALIAS "P" = PRINT
ALIAS "I" = INPUT
ALIAS "G" = GOTO
```

### C. Compatibility

Map commands from other BASIC dialects:

```basic
ALIAS "LOCATE" = PRINT AT     ' Map LOCATE to PRINT AT
ALIAS "CLS" = CLS             ' (already exists)
```

### D. Teaching

Create English-like keywords for beginners:

```basic
ALIAS "DISPLAY" = PRINT
ALIAS "ASK" = INPUT
ALIAS "REPEAT" = FOR
ALIAS "UNTIL" = NEXT
```

---

## 6. Limitations

- Maximum **32 aliases**
- Alias names are **case-insensitive**
- Alias names cannot conflict with existing keywords
- Aliases **cannot chain** (alias of an alias)
- Aliases are **not saved** with the program
- Alias names: max **31 characters**
