# Creating and Metaprogramming Dialects in BASIC++

### Version 6.2.0 — Dynamic Dialect Metaprogramming Manual

BASIC++ features a fully dynamic, metaprogrammable dialect engine that allows scriptwriters and system programmers to redefine how the lexer, parser, variable system, and execution environments behave.

Rather than relying on legacy hardcoded C configurations, Version 6.x.x uses dynamic key-value maps (`BppMap`) which can be validated, documented, registered, and switched entirely at runtime using BASIC++ code.

---

## 1. Dialect Map Properties & Schema

A custom dialect is declared as a dictionary map containing the following configuration fields:

| Field Key | Type | Default | Description |
|---|---|---|---|
| `name` | String | `"CUSTOM"` | The human-readable name of the dialect. |
| `comment_char` | String | `"'"` | The comment prefix character (e.g. `'`, `#`, `!`). Supports escaped sequences like `\n`. |
| `stmt_separator` | String | `":"` | The statement separator character separating multi-statement lines. |
| `default_array_base` | Number | `0` | Default lower bound for arrays (`0` or `1`). |
| `case_sensitive` | Number/Bool | `0` | Case-sensitivity toggle (`1` = active, `0` = case-insensitive). |
| `math_precedence` | String | `"STANDARD"` | Math evaluation order (`"STANDARD"` for PEMDAS, `"LEFT_TO_RIGHT"` for sequential evaluation). |
| `keywords` | Map | `(None)` | Nested map translating custom aliases to target core keywords (e.g. `DISPLAY` -> `PRINT`). |
| `preprocessor_hook` | String | `""` | User-defined BASIC++ function hook (e.g. `PREPROC$`) to rewrite lines before tokenization. |

---

## 2. Dynamic Built-in Statements and Functions

The dialect metaprogramming engine registers a set of statement and function keywords:

### `DIALECT_VALIDATE(map)`
Validates that the provided dictionary map conforms to the dialect schema requirements. Returns `1.0` if valid, or `0.0` if an error occurs.
```basic
10 LET M = MAP_NEW()
20 LET OK = MAP_SET(M, "name", "MyDialect")
30 PRINT DIALECT_VALIDATE(M)
```

### `DIALECT_DOC$(map)`
Generates structured Markdown documentation detailing the custom dialect configuration properties and keyword mapping tables.
```basic
40 PRINT DIALECT_DOC$(M)
```

### `DIALECT REGISTER map`
Loads, validates, and registers the dialect config map into the virtual machine's active state.
```basic
50 DIALECT REGISTER M
```

### `DIALECT LOAD map_or_filepath_or_spec [, format_string]`
Dynamically registers and switches the active parser configuration to the target dialect schema. It accepts:
- A configuration map object (e.g. `DIALECT LOAD M`).
- A file path to a dialect configuration file (`.json`, `.ini`, `.xml`, or `.yaml`). The format is automatically inferred from the file extension if the second parameter is omitted (e.g. `DIALECT LOAD "custom.json"`).
- An inline spec string, with the format specified as the second parameter (e.g. `DIALECT LOAD "{...}", "JSON"`).
```basic
60 DIALECT LOAD M
70 DIALECT LOAD "my_dialect.json"
80 DIALECT LOAD "my_dialect_spec.txt", "JSON"
```

---

## 3. Dynamic Preprocessor Hooking

The preprocessor hook allows you to parse, rewrite, and extend the grammar of your dialect before the tokenizer parses the line.

When `preprocessor_hook` points to a user-defined function (e.g., `PREPROC$`), the VM executes this function for every line of input. To prevent stack overflow, recursive hooks are safely guarded inside the execution pipeline.

### Preprocessor Hook Example
This hook intercepts custom assignment operators `<-` and translates them into standard `=` assignments:

```basic
10 LET M = MAP_NEW()
20 LET OK = MAP_SET(M, "name", "MyCustomBASIC")
30 LET OK = MAP_SET(M, "preprocessor_hook", "PREPROC$")
40 DIALECT REGISTER M
50 DIALECT LOAD M
60 LET target <- 999
70 PRINT "Result: "; target
80 END

90 FUNCTION PREPROC$(LINE_STR$)
100   LET RES$ = ""
110   LET L = LEN(LINE_STR$)
120   LET I = 1
130   WHILE I <= L
140     LET IS_HOOK = (MID$(LINE_STR$, I, 2) = "<-")
150     IF IS_HOOK THEN LET RES$ = RES$ + "="
160     IF IS_HOOK THEN LET I = I + 2
170     IF IS_HOOK = 0 THEN LET RES$ = RES$ + MID$(LINE_STR$, I, 1)
180     IF IS_HOOK = 0 THEN LET I = I + 1
190   WEND
200   LET PREPROC$ = RES$
210 END FUNCTION
```

---

## 4. Dynamic Dialect Configuration Files

Instead of building a dialect schema programmatically using maps, you can define your custom dialect in a file. BASIC++ supports parsing JSON, INI, XML, and YAML configuration file structures.

### Dialect Configuration Formats

#### JSON (`.json`)
```json
{
  "name": "CustomJSON",
  "comment_char": "#",
  "stmt_separator": ";",
  "default_array_base": 1,
  "case_sensitive": 1,
  "math_precedence": "LEFT_TO_RIGHT",
  "keywords": {
    "DISPLAY": "PRINT",
    "QUIT": "END"
  }
}
```

#### INI (`.ini` / `.cfg`)
```ini
name = CustomINI
comment_char = #
stmt_separator = ;
default_array_base = 1
case_sensitive = 1
math_precedence = LEFT_TO_RIGHT

[keywords]
DISPLAY = PRINT
QUIT = END
```

#### XML (`.xml`)
```xml
<dialect>
  <name>CustomXML</name>
  <comment_char>#</comment_char>
  <stmt_separator>;</stmt_separator>
  <default_array_base>1</default_array_base>
  <case_sensitive>1</case_sensitive>
  <math_precedence>LEFT_TO_RIGHT</math_precedence>
  <keywords>
    <DISPLAY>PRINT</DISPLAY>
    <QUIT>END</QUIT>
  </keywords>
</dialect>
```

#### YAML (`.yaml` / `.yml`)
```yaml
name: CustomYAML
comment_char: "#"
stmt_separator: ";"
default_array_base: 1
case_sensitive: 1
math_precedence: LEFT_TO_RIGHT
keywords:
  DISPLAY: PRINT
  QUIT: END
```

Loading these files is done by passing the path to `DIALECT LOAD`:
```basic
10 DIALECT LOAD "custom_dialect.json"
20 DISPLAY "Dynamic dialect loaded successfully!" ; # Custom syntax active
```

---

## 5. Compile-Time Static Custom Dialects (bppc)

Dynamic map allocation and JSON/INI parsing require additional code footprint and runtime overhead, which is unsuitable for memory-restricted embedded targets or microcontrollers running `blite` and `bscript`. 

To support custom dialects on lightweight builds, the compiler/transpiler (`bppc`) includes a compile-time static header generator option.

### Generating Static Dialect Headers
Use the `--dialect` switch with `bppc` to compile a dialect JSON/INI specification directly into a static C header file (`custom_dialect_static.h`):

```bash
# Generate static header from JSON config
bppc --dialect my_dialect.json include/custom_dialect_static.h

# Generate static header from INI config
bppc --dialect my_dialect.ini include/custom_dialect_static.h
```

### Compiling Custom Dialect Binaries
To build `blite` and `bscript` with the statically generated custom dialect active from boot:
1. Generate the `custom_dialect_static.h` header in the search path (e.g. inside `include/`).
2. Compile the binaries with the `BASIC_USE_CUSTOM_STATIC_DIALECT` compiler option enabled:
   - **CMake**: Enforce `-DBPP_USE_CUSTOM_STATIC_DIALECT=ON` during configuration.
   - **MSVC / MSBuild**: Define the preprocessor macro `BASIC_USE_CUSTOM_STATIC_DIALECT`.

This compiles a custom footprint-optimized executable with zero runtime BppMap allocation overhead.

