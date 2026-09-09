<!--
Title:        Introspection_Reference
Tier:         1
Applies to:   BASIC++ v6.5.2, all targets
Authority:    engine/include/runtime/language_descriptor.h,
              engine/src/statements/introspection/ (9 files),
              engine/src/runtime/metadata.c, keyword_props.c, override.c,
              engine/src/eval/functions/system/hardware/func_devinfo.c
Generated:    no, hand-written
Status:       current
-->

# Introspection Reference

Asking BASIC++ what it knows about itself — and changing the answer.

Most languages let a program inspect its own data. BASIC++ lets a program
inspect and modify **the language**: which keywords exist, what they mean,
what category they belong to, whether they are enabled, and what they do when
called. This document covers that surface.

---

## 1. Why the language is introspectable at all

Every statement, function and variable in BASIC++ registers a
`LanguageDescriptor` at start-up. There are 371 statements and 316 functions
in the registry. The descriptor is not documentation attached to the feature —
it *is* the feature's registration, and the dispatcher uses it.

That single design decision is what makes everything below possible. `HELP`
does not read a help file; it reads the registry. `CATEGORY` does not consult
a table; it queries the registry. A generated documentation page and the
running interpreter cannot disagree, because they have the same source.

---

## 2. What a descriptor carries

From `engine/include/runtime/language_descriptor.h`:

**Identity and behaviour**

| Field | Meaning |
|---|---|
| `name` | Canonical feature name |
| `category` | Functional category, e.g. "Variables & Memory" |
| `syntax` | Formal syntax signature |
| `description` (aliased `help_text`) | One-line human description |
| `error_summary` (aliased `error_codes`) | Formatted error summary |
| `errors`, `error_count` | Optional structured error array |
| `examples` | Ready-to-run code |
| `dispatch_fn` | The handler |
| `dispatch_bracket_fn`, `dispatch_brace_fn` | Handlers for the `[]` and `{}` forms |

**Classification**

| Field | Values |
|---|---|
| `type` (`FeatureType`) | `STATEMENT`, `FUNCTION`, `COMMAND`, `VARIABLE`, `MODULE`, `OPERATOR`, `DEVICE`, `WIDGET` |
| `safety` (`SafetyLevel`) | `PURE`, `SAFE`, `IO`, `SYSTEM`, `UNSAFE` |
| `subsystem` (`FunctionalSubsystem`) | One of twelve: boot, platform, kernel, engine, hardware, server, script, core, flex, standard, advanced, ext |
| `delim_mask` (`DelimiterMask`) | `PAREN`, `BRACKET`, `BRACE` — which of `()`, `[]`, `{}` the feature accepts |

The delimiter mask is how the project's delimiter invariant is enforced
mechanically rather than by convention: `()` is infix, `[]` is prefix Polish
notation and slicing, `{}` is postfix reverse Polish, maps and sets. See
`Delimiter_And_Bracket_Keyword_Taxonomy`.

**Architecture and provenance**

`rationale`, `design_notes`, `tunables`, `invariants`, `assumptions`,
`platform_mask`, `roadmap`, `compat`, `since_version`, `source_file`,
`see_also`.

These eleven fields exist in the struct today. **Almost nothing populates
them.** The descriptors read during the September 2026 documentation pass set
the identity and classification fields and stop. Filling them is item 7.1 of
the 7.0.0 plan, and `compat` — the vintage dialect lineage, machine readable —
is the one that matters most, because it turns "does this change break Rule
#1" from an argument into a query.

---

## 3. Querying the language from BASIC

| Statement | Syntax | Purpose |
|---|---|---|
| `HELP` | `HELP [keyword \| command \| block_target]` | Interactive help for statements, functions, syntax and system components |
| `CATEGORY` | `CATEGORY [keyword \| category_name]` | List all categories, ask which category a keyword is in, or list a category's keywords |
| `CATALOG` | `CATALOG [category_name \| keyword1 [, keyword2...]]` | Catalog of categories, dynamic modules, global labels, custom blocks and specifications |
| `KEYWORD` | `KEYWORD target GET prop` | Read a keyword's dynamic property |
| `VARS` | `VARS` | List defined variables |
| `SCOPE` | `SCOPE` | Report the scope chain |
| `DEVICES` | `DEVICES` | Registered virtual devices |
| `MOUNTS` | `MOUNTS` | Filesystem mounts |
| `MEMMAP` | `MEMMAP` | Memory layout |
| `SELFTEST` | `SELFTEST` | Run the internal diagnostic suite over lexer, memory, string, variable and array subsystems |

```basic
10 CATEGORY "Devices & Network"
20 HELP MAT
30 KEYWORD PRINT GET SAFETY
40 CATALOG
```

`SELFTEST` deserves separate mention: when behaviour is inexplicable, run it.
If it fails, the problem is beneath your program rather than in it.

---

## 4. Querying the environment

Fourteen statements print a host fact to the console, from
`statements/introspection/introspection.c`:

| Statement | Reports |
|---|---|
| `HOSTNAME` | System hostname |
| `COMPUTERNAME` | Machine name |
| `USERNAME` | Current user identity |
| `LOGNAME` | Login account name |
| `HOMEPATH` | Home directory path |
| `HOMEDRIVE` | Home drive letter |
| `USERPATH` | Full user profile directory |
| `COMSPEC` | Active command shell path |
| `TOTALMEM` | Total physical memory, bytes |
| `AVAILMEM` | Free physical memory, bytes |
| `UPTIME` | Host uptime, seconds |
| `EPOCH` | Seconds since the 1980-01-01 MS-DOS epoch |
| `UNIXTIME` | Seconds since the 1970-01-01 Unix epoch |
| `STARDATE` | CP/M days with a tenth-day decimal |

The three time bases are not redundancy: they are the three epochs a vintage
program might have been written against, and a converted program keeps
whichever one it used.

Alongside these, the function forms — `VERSION`, `CPU`, `FRE(0)`,
`ENVIRON$`, `CURDIR$`, `EXEPATH$`, `SCRIPTPATH$`, `WORKDIR$`, `TEMPDIR$`,
`USERPATH$`, `DEVINFO$` — return values rather than printing.

---

## 5. Changing the language from BASIC

This is the part that distinguishes BASIC++ from a language with a reflection
API. Introspection here is not read-only.

### `ALIAS` — rename anything

```
ALIAS name = expansion | ALIAS LIST | ALIAS CLEAR | ALIAS OPER op = expansion
```

Aliases a keyword, or an **operator**, to another spelling. This is the
sanctioned way to build a dialect flavour without forking the engine: alias
the keywords a vintage dialect used to their BASIC++ equivalents and a program
written for that dialect runs.

### `KEYWORD` — change a keyword's properties

```
KEYWORD target SET prop = val | KEYWORD target GET prop
KEYWORD target LIST | KEYWORD CLEAR
```

Dynamic syntactic and semantic properties, backed by
`engine/src/runtime/keyword_props.c`.

### `OVERRIDE` — replace a built-in's behaviour

```
OVERRIDE target WITH GOSUB line_num | OVERRIDE target WITH sub_name
OVERRIDE CLEAR
```

Replaces or hooks a built-in statement with a user-defined `SUB` or a `GOSUB`
target. `PRINT` can be made to log, `INPUT` can be made to read from a queue,
and the original is restored with `OVERRIDE CLEAR`.

This is powerful and correspondingly easy to misuse. Two disciplines are worth
adopting: override in a bounded scope (see `SCOPE` below) rather than
globally, and never override something the rest of your program relies on
without saying so at the top of the file.

### `SCOPE` — bound the damage

```
SCOPE [BEGIN | END | DISABLE kw | ENABLE kw | HOOK ... | MODULE name | PRIVATE sym]
```

`SCOPE BEGIN` and `SCOPE END` bracket a region. `SCOPE DISABLE kw` removes a
keyword from the language inside it — which is how you run untrusted code
without `SHELL`, or teach a class where `GOTO` does not exist. `SCOPE PRIVATE`
protects a symbol from being redefined.

### `REMOVE` — the odd one out

```
REMOVE var_or_array, value | REMOVE$ str_var_or_array, target_str$
```

Despite living in the introspection directory, `REMOVE` operates on data, not
on the language: in-place removal of values from variables and arrays and of
substrings from strings.

---

## 6. Introspection from C

`lang_desc_*` in `language_descriptor.h`:

| Function | Purpose |
|---|---|
| `lang_desc_find(name)` | Look up one descriptor |
| `lang_desc_get(index)`, `lang_desc_count()` | Walk the whole registry |
| `lang_desc_query_category(category, results, max)` | Everything in a category |
| `lang_desc_get_categories(out, max)` | Every category name |
| `lang_desc_lookup_error_msg(code)` | Error code to message |
| `lang_desc_register(desc)` | Add a feature |
| `lang_desc_override(name, desc)` | Replace one |
| **`lang_desc_export_json()`** | **Dump the entire registry as JSON** |

`lang_desc_export_json` is the one to know about. It means any external tool —
an editor's completion index, a linter, a documentation generator, a coverage
report — can have the whole language as data without parsing anything. Free
the result with `lang_desc_free_json`.

Registration from C is three macros: `REGISTER_STATEMENT`,
`REGISTER_FUNCTION` and `REGISTER_VARIABLE`, each setting `type` and
`dispatch_fn` and calling `lang_desc_register`. See `Extension_Guide`.

---

## 7. What introspection is for

Four uses, in rough order of how often they come up.

**Learning the language at the prompt.** `HELP`, `CATEGORY` and `CATALOG`
replace a manual for the common case. A user who does not know whether a
function exists can ask.

**Sandboxing.** `SCOPE DISABLE` plus the `safety` classification plus the
module capability mask is a three-layer answer to "can I run this program I
did not write". See `Security`.

**Dialect work.** `ALIAS` and `KEYWORD` reshape the surface syntax without
touching the engine, which is the sanctioned successor to the purged `DIALECT`
keyword. See `Dialect_Migration_Reference`.

**Tooling.** `lang_desc_export_json` is the interface for anything outside the
engine that needs to know what the language contains.

---

## 8. Where it is weak

Stated plainly, because a reader planning to build on this should know.

- **The extended descriptor fields are empty.** Section 2 lists eleven fields
  that exist and are not populated. Until they are, `compat`, `source_file`
  and `roadmap` cannot be queried, and the documentation cannot cite the
  descriptor for them.
- **`docgen` emits five fields of the fifteen.** It writes one monolithic
  `PUBLIC_API_REFERENCE.md` rather than a page per item, and does not mirror
  into `help/`. That is why the keyword documentation drifts. Item 7.2 of the
  7.0.0 plan.
- **Nothing enforces registry-to-documentation parity.** A keyword can exist
  with no page, and a page can exist with no keyword. Both are true today.
  Item 7.3.
- **There is no introspection of the *program*.** You can ask what the
  language contains; you cannot ask what your own program contains from within
  it. `LIST`, `VARS`, `CHECK` and `VERIFY` approach it from outside.

---

## See also

- `Extension_Guide` for adding keywords and modules
- `Dialect_Migration_Reference` for `ALIAS` and dialect shaping
- `Security` for `SCOPE DISABLE`, safety levels and capabilities
- `Delimiter_And_Bracket_Keyword_Taxonomy` for `delim_mask`
- `Keyword` and `Override` for the individual statements
- `Debugging_And_Testing` for `SELFTEST`, `CHECK` and `VERIFY`
- `Implementation_Status` for what is populated and what is not
