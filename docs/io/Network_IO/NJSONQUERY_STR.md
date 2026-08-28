# NJSONQUERY$ Function Reference

The `NJSONQUERY$` built-in string function parses and queries structured JSON data received over a network I/O channel using a dot-delimited JSONPath expression.

## Syntax

```basic
field_val$ = NJSONQUERY$(channel%, json_path$)
```

## Parameters

- **`channel%`** — Active network socket channel number (`#1` to `#16`).
- **`json_path$`** — A string expression containing the JSONPath query (e.g. `"status"`, `"user.name"`, `"items[0].price"`).

## Return Value

- Returns a **string** (`VAL_STRING`) containing the extracted value.
- If the JSON field is not found or malformed, returns `""` (empty string).

---

## Code Examples

### Example 1: Extracting REST API JSON Fields
```basic
10 OPEN "HTTP://api.weather.local/current" AS #1
20 Temp$ = NJSONQUERY$(1, "current.temperature_c")
30 City$ = NJSONQUERY$(1, "location.city")
40 PRINT "Current weather in "; City$; ": "; Temp$; " °C"
50 CLOSE #1
```

---

## Engine Implementation (`server/vnet.c` & `server/json.c`)

In `engine/src/server/vnet.c`:
`NJSONQUERY$` parses the response body into an ephemeral AST node via `libserver/json.c` and resolves the JSONPath expression without storing intermediate DOM trees.

---

## Error Codes

| Error Code | Name | Condition |
|------------|------|-----------|
| 52 | Bad File Number (`ERR_BAD_FILE_NUMBER`) | Channel not open |
| 13 | Type Mismatch (`ERR_TYPE_MISMATCH`) | Argument error |

---

## Cross-References

- **`NHTTPSTATUS.md`** — HTTP status code.
- **`NREAD_STR.md`** — Raw network read.
- **`tinydb.md`** — In-memory document storage.
