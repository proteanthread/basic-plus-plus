<!--
Title:        Extension_Guide
Tier:         1
Applies to:   BASIC++ v6.5.2, all targets
Authority:    engine/include/device/vdev.h, engine/include/module/module.h,
              engine/include/runtime/language_descriptor.h,
              engine/include/runtime/funcreg.h
Generated:    no, hand-written
Status:       current
-->

# Extension Guide

BASIC++ is intended to be a metalanguage: a language you extend rather than
merely use. This document covers every mechanism for doing that, from the one
that needs no code at all to the one that changes the engine.

---

## 1. Four mechanisms, in ascending order of cost

Pick the cheapest one that does the job. Most people reach for the last when
they need the second.

| # | Mechanism | Written in | Needs a rebuild | Use when |
|---|---|---|---|---|
| 1 | **Alias** | Nothing | No | You want a different name for something that exists |
| 2 | **User-Defined Device** | BASIC | No | You want a device that behaves your way |
| 3 | **Module** | C | The module, not the engine | You want new keywords, or a packaged capability |
| 4 | **Native keyword** | C, in-tree | Yes | The keyword belongs in the language itself |

---

## 2. Aliases: renaming without code

The cheapest extension. `ALIAS` binds a new name to an existing device or
keyword; `OVERRIDE` replaces a keyword's behaviour.

```basic
10 ALIAS LPT: = PRN:
20 ALIAS PRINTOUT = LPRINT
```

Underneath, `vdev_alias_set`, `vdev_alias_remove` and `vdev_alias_resolve`
maintain a table of `VDevAliasEntry` records, capped at `MAX_VDEV_ALIASES`
(64). An alias holds a 32-character name and a 64-character target.

This is how a vintage program expecting a device name your system does not
have is made to run without editing the program.

See `Using_Aliases` and `Override`.

---

## 3. User-defined devices: drivers written in BASIC

This is the mechanism most people miss, and it is the clearest expression of
the metalanguage idea: **you can write a device driver in BASIC.**

The `UserDefinedDevice` structure binds five BASIC subroutines to the five
device operations:

```c
typedef struct {
    char name[32];
    char class_name[32];
    int  buffer_size;
    int  open_gosub_line;
    int  read_gosub_line;
    int  write_gosub_line;
    int  ioctl_gosub_line;
    int  close_gosub_line;
    VMContext *vm;
    bool active;
} UserDefinedDevice;
```

When something opens, reads from, writes to, controls or closes your device,
the engine performs a `GOSUB` to the line you nominated. Your BASIC code is
the driver. Up to `MAX_VDEV_UDD` (32) user-defined devices can be registered
at once.

The registration API is `vdev_udd_register`, `vdev_udd_unregister` and
`vdev_udd_get`. See `docs/api/tutorials/custom_vdev.md` and
`docs/keywords/statements/DEVICE.md`.

### Device classes

A device declares its class from `VDevClass`, which has 29 values:

```
CONSOLE  FILE     SERIAL   PRINTER  AUDIO     NETWORK  GPIO
I2C      SPI      SENSOR   DISPLAY  STORAGE   HID      CAMERA
BRIDGE   BLUETOOTH CLIPBOARD PIPE    TIMER     USER     TAPE
MODEM    BARCODE  WINDOW   FRAMEBUFFER KEYMAP  LOOPBACK
UNKNOWN  CUSTOM
```

The class determines how the device is discovered and what the engine expects
of it. `CUSTOM` (99) is the escape hatch.

### Capability flags and gating

Each device carries `dev_caps` and `dev_req_caps`. The `VDCAP_` flags start
`VDCAP_READ`, `VDCAP_WRITE`, `VDCAP_BINARY`. A device that does not declare a
capability will not be asked to perform it, and `dev_req_caps` gates the
device behind the `CAP_` permissions in section 4.

### Chains and multiplexers

Two composition mechanisms exist beyond a single device:

- `vdev_chain_create(ctx, name, filters, count, target)` builds a pipeline:
  output passes through a sequence of filter devices before reaching the
  target. Up to `MAX_VDEV_CHAINS` (16).
- `vdev_mux_create(ctx, name, targets, count)` fans one stream out to several
  devices at once. Up to `MAX_VDEV_MUX` (16).

A logging device that timestamps, compresses and writes to both a file and the
console is a chain plus a mux, with no C at all.

### Sub-devices

`vdev_parse_subdevice_index` splits a name like `COM2:` into a base and an
index from 1 to 8. Indices 0 and 9 are reserved.

---

## 4. Modules: packaged capability in C

A module is a C plugin. `BppModuleInfo` is the contract:

```c
typedef struct {
    const char *name;
    const char *version;
    const char *description;
    BppModuleClass mod_class;
    unsigned int capabilities;
    BppSecLevel required_level;
    int (*init)(void *);
    void (*cleanup)(void);
} BppModuleInfo;
```

### The four module classes

```c
MOD_LIBRARY   = 0    // a body of functionality
MOD_DIALECT   = 1    // a language variant
MOD_DEVICE    = 2    // a device driver
MOD_EXTENSION = 3    // an engine extension
```

**`MOD_DIALECT` is how you build your own dialect.** Read that carefully,
because it is not the purged `DIALECT` keyword and must not be confused with
it. `DIALECT` and `OPTION DIALECT` were removed from the language: they were a
hidden runtime switch that broke modularity and analysability. A dialect
module is different in kind — it is a compile-time, registered, capability-
declared, security-gated unit with no hidden global state. Section 6 covers
building one.

### Capabilities

A module declares what it needs as a bitmask:

```
CAP_MATH      CAP_STRING   CAP_IO      CAP_FILE     CAP_SYSTEM
CAP_GRAPHICS  CAP_SOUND    CAP_NETWORK CAP_GPIO     CAP_I2C
CAP_SPI       CAP_SENSOR   CAP_CAMERA  CAP_BLUETOOTH CAP_USB
```

`module_caps_string` renders the mask for display. A module that does not
declare `CAP_FILE` cannot touch files, and this is enforced rather than
advisory.

### Security level

`required_level` is a `BppSecLevel`. A module requiring a higher level than
the running context has will not activate. See `Security`.

### Lifecycle

| Function | Purpose |
|---|---|
| `module_register(info)` | Make the engine aware of it |
| `module_activate(name, rt)` | Bring it up; calls `init` |
| `module_deactivate(name)` | Take it down; calls `cleanup` |
| `module_is_active(name)` | Query |
| `module_find(name)` | Look up by name |
| `module_load_dynamic(vm, path)` | Load a shared library at run time |

`MAX_MODULES` is 16 concurrently registered.

From BASIC, `MODULE`, `IMPORT`, `PUBLIC` and `LOAD` are the relevant
statements. See `Module_Guide`.

---

## 5. Native keywords: extending the language itself

When a capability belongs in the language rather than beside it, add a
keyword. Every keyword in BASIC++ is one source file and one CMake `OBJECT`
target, and the process is a full vertical:

```
Lexer -> AST Parser -> AST Evaluator -> C Runtime Handler and LangDesc
      -> VM Registry and Dispatch -> CMake -> Assertion Tests -> Documentation
```

`PROJECT_RULES.md` requires the whole stack on the first pass. A half-added
keyword is worse than none.

### Registration

Declarative, through `engine/include/runtime/language_descriptor.h`:

```c
static const LangDesc g_mykeyword_desc = {
    .name        = "MYKEYWORD",
    .category    = "Custom",
    .syntax      = "MYKEYWORD arg[, arg]",
    .description = "One sentence, in plain prose.",
    .error_summary = "Error 5: Illegal Function Call",
    .subsystem   = SUBSYSTEM_EXT,
    .safety      = SAFETY_SAFE,
    .type        = FEATURE_STATEMENT
};

REGISTER_STATEMENT(&g_mykeyword_desc, my_handler);
```

`REGISTER_FUNCTION` and `REGISTER_VARIABLE` are the other two macros.

**The metadata is mandatory and it is not paperwork.** Your keyword's
reference documentation is generated from this descriptor, the help system
reads it, introspection queries it, and leaving a field NULL is prohibited by
`PROJECT_RULES.md`. Fill in `syntax`, `description` and `error_summary`
properly and the documentation writes itself.

### Functions also register in the function registry

```c
FunctionEntry entry = {
    .name = "MYFUNC$", .keyword = KW_NONE, .category = FCAT_UTIL,
    .ret_type = FRET_STRING, .min_args = 1, .max_args = 1,
    .safety = FSAFE_SYSTEM, .overridable = 0,
    .handler = my_evaluator,
    .help_text = "One line", .module_name = "MyModule"
};
funcreg_register(&entry);
```

Note `.overridable`: set it to allow `OVERRIDE` to replace your function.

### Delimiters

`delim_mask` declares which bracket forms your keyword accepts:
`DELIM_PAREN` for infix, `DELIM_BRACKET` for prefix and slicing,
`DELIM_BRACE` for postfix, maps and sets. `dispatch_bracket_fn` and
`dispatch_brace_fn` let one keyword behave differently under each. The
invariant is project-wide; see `Delimiter_And_Bracket_Keyword_Taxonomy`.

---

## 6. Building your own dialect

From smallest to largest.

**Smallest: aliases only.** A file of `ALIAS` statements loaded at startup
gives you vintage spellings over the existing engine. No code, no rebuild.

**Small: a dialect module.** A `MOD_DIALECT` module registering the keywords
your dialect needs, with its own capability mask and security level, loaded
with `module_load_dynamic`. The engine is untouched.

**Large: a build configuration.** Because every keyword is its own CMake
`OBJECT` target, a dialect can be defined by which targets are linked. This is
how the minimal builds are produced, and it is what
`Building_A_Minimal_BASIC++` documents.

**What you must not do:** reintroduce a runtime dialect switch. There is no
`DIALECT` keyword and no `OPTION DIALECT`. Of the `OPTION` statement, only
`OPTION BASE 0/1` and the retained standards options — `OPTION ANGLE
DEGREES/RADIANS`, `OPTION COLLATE`, `OPTION ARITHMETIC DECIMAL/NATIVE` —
exist. Selecting a dialect is a build and registration decision, made once,
visible in the binary.

---

## 7. Which mechanism for which question

| You want to | Use |
|---|---|
| Rename a device | `ALIAS` (section 2) |
| Rename or replace a keyword | `ALIAS`, `OVERRIDE` (section 2) |
| A device that behaves your way, written in BASIC | User-defined device (section 3) |
| A pipeline or a fan-out of devices | Chains and multiplexers (section 3) |
| A packaged library others can load | `MOD_LIBRARY` module (section 4) |
| Your own dialect | `MOD_DIALECT` module or a build config (section 6) |
| A device driver in C | `MOD_DEVICE` module (section 4) |
| A new keyword in the language | Native keyword (section 5) |
| A shared library callable from BASIC | `MODULE` and `IMPORT`; see `Module_Guide` |
| To embed BASIC++ in a C program | `basicpp.h`; see `C_Programmers_Guide` |

---

## 8. Where the code is

| Concern | Path |
|---|---|
| Virtual devices, aliases, UDD, chains | `engine/include/device/vdev.h` |
| Module system | `engine/include/module/module.h` |
| Keyword metadata | `engine/include/runtime/language_descriptor.h` |
| Function registry | `engine/include/runtime/funcreg.h` |
| Host embedding | `engine/include/basicpp.h` |
| Tutorials | `docs/api/tutorials/` |

## See also

- `Module_Guide` for modules from the BASIC side
- `Virtual_Devices` and `Device_Reference` for the device layer
- `Using_Aliases`, `Override` for aliasing
- `Security` for capability gating and safety levels
- `C_Programmers_Guide` for the host API
