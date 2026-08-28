# BASIC++ v6.5.2 Using Aliases

## 1. THE ALIAS STATEMENT

ALIAS creates a new name for an existing keyword, statement, or function. The alias behaves identically to the original — it is a syntactic synonym, not a wrapper or macro:

```basic
10 ALIAS "DISPLAY" AS "PRINT"
20 DISPLAY "Hello, World!"      ' Works exactly like PRINT
```

After this alias, DISPLAY can be used anywhere PRINT would be used, with all the same syntax (semicolons, commas, USING, #channel, etc.).

## 2. ALIAS SYNTAX

ALIAS "new_name" AS "original_name" creates the alias. Both names are case-insensitive. The original keyword remains available — ALIAS adds a name, it does not replace the original.

```basic
10 ALIAS "AFFICHER" AS "PRINT"      ' French
20 ALIAS "LIRE" AS "INPUT"          ' French
30 ALIAS "SI" AS "IF"               ' French
40 ALIAS "ALORS" AS "THEN"          ' French
50 ALIAS "FIN" AS "END"             ' French
```

After these aliases, a program can be written entirely in French keywords:

```basic
100 AFFICHER "Bonjour!"
110 LIRE "Votre nom: "; Nom$
120 SI Nom$ = "" ALORS FIN
130 AFFICHER "Bienvenue, "; Nom$
```

## 3. USE CASES

### Localization

ALIAS allows BASIC++ to be used with keywords in any human language. Teachers can create alias sets for their students' native language.

### Compatibility

Programs from one BASIC dialect can use aliases to map unfamiliar keywords:

```basic
10 ALIAS "REPEAT" AS "DO"           ' Map REPeat to DO
20 ALIAS "ENDREPEAT" AS "LOOP"      ' Map END REPeat to LOOP
```

### Abbreviation

Frequently used keywords can be abbreviated:

```basic
10 ALIAS "P" AS "PRINT"
20 ALIAS "I" AS "INPUT"
30 P "Quick typing!"
```

### Domain-Specific Languages

Aliases can create domain-specific vocabularies:

```basic
10 ALIAS "MEASURE" AS "INPUT"
20 ALIAS "RECORD" AS "PRINT"
30 ALIAS "SAMPLE" AS "READ"
```

## 4. ALIAS FOR FUNCTIONS

ALIAS works with functions as well as statements:

```basic
10 ALIAS "LONGUEUR" AS "LEN"
20 ALIAS "GAUCHE$" AS "LEFT$"
30 PRINT LONGUEUR("HELLO")          ' Prints 5
40 PRINT GAUCHE$("HELLO", 3)        ' Prints HEL
```

## 5. LISTING AND REMOVING ALIASES

ALIAS with no arguments lists all active aliases:

```basic
> ALIAS
Active aliases:
  DISPLAY -> PRINT
  AFFICHER -> PRINT
  LONGUEUR -> LEN
```

ALIAS$ returns the original keyword for an alias: `PRINT ALIAS$("DISPLAY")` prints "PRINT". If the name is not an alias, ALIAS$ returns the name itself.

REMOVE "DISPLAY" removes the alias. The original keyword remains unaffected.

## 6. PERSISTENCE

Aliases defined in a program persist for the duration of the session. They are cleared by NEW. Aliases can be saved in a startup file that runs before the main program using CHAIN or MERGE.

## 7. IMPLEMENTATION

ALIAS is implemented in engine/src/statements/dialect/alias.c. It modifies the lexer's keyword lookup table by adding an entry that maps the alias name to the same keyword ID as the original. This means aliased keywords are recognized at lex time and produce the same token type, ensuring complete compatibility with all statement handlers.
