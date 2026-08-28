# OPTION VIDEO

## Syntax

```
OPTION VIDEO {PAL | SECAM | NTSC}
```

## Parameters

- **PAL** — Configures the timing multiplier for 50 Hz video systems (European broadcast standard).
- **SECAM** — Functionally identical to PAL, configuring the timing multiplier for 50 Hz video systems (French/Eastern European broadcast standard).
- **NTSC** — Configures the timing multiplier for 60 Hz video systems (North American/Japanese broadcast standard). This is the default.

## Description

`OPTION VIDEO` selects the simulated video timing standard used by the `JIFFIES` function. In legacy systems like the Commodore 64 or ZX Spectrum, the system timer was physically tied to hardware interrupts triggered by the video chip drawing the screen. PAL systems generated 50 interrupts per second; NTSC systems generated 60. GW-BASIC and IBM PCs used the Intel 8253 timer chip (ticking at 18.2 Hz), but many retro programs assumed either 50 Hz or 60 Hz timing.

`OPTION VIDEO` bridges this compatibility gap by setting the `jiffies_multiplier` field in the VM context. The `JIFFIES` function reads this multiplier to convert raw uptime into video-frequency-scaled tick counts, allowing programs to use consistent timing regardless of the actual host clock.

```basic
10 OPTION VIDEO NTSC
20 PRINT "Waiting for 60 jiffies (1 second in NTSC)..."
30 START_TIME = JIFFIES
40 IF JIFFIES < START_TIME + 60 THEN GOTO 40
50 PRINT "Done."
```

```basic
100 REM Simulate a 50Hz environment for European software
110 OPTION VIDEO PAL
120 START_T = JIFFIES
130 GOSUB 1000
140 END_T = JIFFIES
150 PRINT "Processing took "; END_T - START_T; " PAL jiffies."
160 END
1000 REM Processing subroutine
1010 FOR I = 1 TO 1000 : NEXT I
1020 RETURN
```

## Multiplier Values

| Mode | Multiplier | Ticks Per Second |
|------|-----------|------------------|
| NTSC | 60.0 | ~60 jiffies/sec |
| PAL | 50.0 | ~50 jiffies/sec |
| SECAM | 50.0 | ~50 jiffies/sec |

The multiplier is applied by `JIFFIES` as: `result = platform_get_uptime() * multiplier`

## JIFFIES Function Interaction

The `JIFFIES` function (implemented in `engine/src/eval/eval_builtins.c`) reads the current multiplier via `vm_get_jiffies_multiplier(vm)` and multiplies it against `platform_get_uptime()` to produce the tick count.

`JIFFIES` also accepts an optional string argument that temporarily overrides the multiplier for that single call without changing the persistent setting:

```basic
OPTION VIDEO PAL
PRINT JIFFIES           ' Uses PAL (50 Hz) multiplier
PRINT JIFFIES("NTSC")   ' Temporarily uses NTSC (60 Hz) for this call only
PRINT JIFFIES           ' Still PAL (50 Hz)
```

The `JIFFIES` argument override does not mutate `jiffies_multiplier` — it only affects the return value of that single function call.

## Engine Implementation (v6.5.2)

### VM Context Infrastructure

The `jiffies_multiplier` field exists in the v6.5.2 VM context and is fully functional:

| File | Symbol | Purpose |
|------|--------|---------|
| `engine/src/vm/vm_internal.h:197` | `double jiffies_multiplier` | Storage field in VM context struct |
| `engine/src/vm/context.c:97` | `vm_get_jiffies_multiplier()` | Getter — returns multiplier (default 60.0) |
| `engine/src/vm/context.c:98` | `vm_set_jiffies_multiplier()` | Setter — updates multiplier |
| `engine/src/vm/context.c:299` | Initialization | Default set to 60.0 (NTSC) at VM creation |
| `engine/src/eval/eval_builtins.c:800` | JIFFIES evaluator | Reads multiplier and applies to uptime |

### JIFFIES Evaluator

The JIFFIES function at `engine/src/eval/eval_builtins.c:796-814`:

```c
else if (strcmp(uname, "JIFFIES") == 0) {
    if (arg_count > 1) {
        err->code = 13; err->message = "JIFFIES expects 0 or 1 arguments"; return res;
    }
    double mult = vm_get_jiffies_multiplier(vm);
    if (arg_count == 1) {
        if (args[0].type != VAL_STRING) {
            err->code = 13; err->message = "JIFFIES expects a string argument"; return res;
        }
        const char *mode = str_data(args[0].as.string);
        if (mode) {
            if (strcasecmp(mode, "NTSC") == 0) mult = 60.0;
            else if (strcasecmp(mode, "PAL") == 0) mult = 50.0;
            else if (strcasecmp(mode, "SECAM") == 0) mult = 50.0;
        }
    }
    res.type = VAL_NUMBER;
    res.as.number = platform_get_uptime() * mult;
}
```

### OPTION Statement Handler

The OPTION statement handler (`engine/src/statements/variables/option.c`) currently routes `BASE` and `EXPLICIT` sub-commands but does not yet route `VIDEO`. Attempting `OPTION VIDEO NTSC` in v6.5.2 will produce Error 2 (Syntax Error in OPTION statement) because the `else` branch catches unrecognized sub-keywords.

## Error Codes

| Error | Condition |
|-------|-----------|
| 2 (Syntax Error) | `OPTION VIDEO` not yet routed in v6.5.2 OPTION handler |

When fully wired, invalid modes should produce Error 5 (Illegal Function Call), matching the original v5 behavior where unrecognized modes triggered `error_raise(ERR_WHAT, line_num)`.

## Help System Registration

The `JIFFIES` function is registered in `engine/src/statements/dialect/help_data.h`:

```
{"JIFFIES", "System & Environ", "JIFFIES([mode$])", "Get uptime ticks based on frequency multiplier (NTSC/PAL/SECAM)"}
```

`OPTION VIDEO` does not yet have its own help entry.

## Build Target Availability

The `jiffies_multiplier` infrastructure is available in all build targets since it is part of the core VM context (`engine/src/vm/context.c`):

| Target | Available | Notes |
|--------|-----------|-------|
| `baspp` (Standard) | Yes | Full JIFFIES + multiplier support |
| `bpp` (Lite) | Yes | Full JIFFIES + multiplier support |
| `bs` (Script Runner) | Yes | Full JIFFIES + multiplier support |

## Cross-References

- **JIFFIES** — Returns uptime ticks scaled by the current video timing multiplier.
- **TICKS** — Returns uptime ticks at a fixed 100 Hz rate (not affected by OPTION VIDEO).
- **OPTION BASE** — Configures array subscript indexing base.
- **OPTION EXPLICIT** — Requires explicit variable declaration.

## Proposed Expansion or Changes

The following capabilities existed in v5.0.5 but have not yet been wired into the v6.5.2 OPTION statement handler:

1. **OPTION VIDEO sub-command routing**: The v5 parser (`source/config/parser_config.c`, function `pi_parse_option`) handled `OPTION VIDEO` by reading the next token as a named variable and writing directly to `rt->jiffies_multiplier`. In v6.5.2, the infrastructure exists (`vm_set_jiffies_multiplier()`) but the OPTION handler in `engine/src/statements/variables/option.c` needs a new `else if (platform_strcasecmp(kw, "VIDEO") == 0)` branch to parse the mode token and call `vm_set_jiffies_multiplier()`.

2. **Help system registration**: `OPTION VIDEO` should be added to `help_data.h` as a separate entry under the Config & Environment category.

3. **Micro-library metadata update**: The OPTION statement's `.syntax` field in `stmt_option_register()` should be expanded to include `OPTION VIDEO {PAL | SECAM | NTSC}`.

4. **OPTION ARITHMETIC**: Referenced in the original v5 cross-references but not yet implemented in v6.5.2.

5. **OPTION STRICT**: Referenced in the original v5 cross-references but not yet implemented in v6.5.2. May overlap with `OPTION EXPLICIT`.
