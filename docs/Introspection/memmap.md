# MEMMAP$ Function Reference

The `MEMMAP$` built-in string function returns the identifier name of the currently active virtual BIOS hardware memory map architecture.

## Syntax

```basic
model$ = MEMMAP$
```

## Parameters

*(No parameters accepted; takes 0 arguments.)*

## Return Value

- Returns a **string** (`VAL_STRING`) representing the active hardware profile:
  - `"IBM_PC"` — Standard 1981 IBM PC 5150 architecture (CGA / MDA memory map).
  - `"IBM_XT"` — IBM PC/XT 5160 architecture (Fixed disk BDA, 640KB RAM).
  - `"IBM_AT"` — IBM PC/AT 5170 architecture (286 real mode, high-res EGA/VGA).
  - `"IBM_PCJR"` — IBM PCjr architecture (16-color graphics, 3-voice audio).
  - `"(None)"` — If the virtual BIOS subsystem is disabled (`SUPPORT_BIOS=0`).

---

## Description

In BASIC++ v6.5.2, the virtual BIOS layer (`libbios`) virtualizes authentic IBM PC x86 memory layouts (Interrupt Vector Table `0x0000`, BIOS Data Area `0x0400`, Video RAM `0xA000` / `0xB800`, ROM BIOS `0xF000`).

The `MEMMAP$` function allows programs, retro game emulators, and diagnostic utilities to inspect the active hardware architecture dynamically before making model-specific memory accesses or BIOS interrupt calls.

```basic
10 ModelName$ = MEMMAP$
20 PRINT "Active Architecture: "; ModelName$
30 IF ModelName$ = "IBM_PCJR" THEN
40     PRINT "PCjr 16-color graphics mode available."
50 ELSE
60     PRINT "Standard CGA/VGA configuration."
70 END IF
```

---

## Code Examples

### Example 1: Hardware-Aware Feature Dispatch
```basic
100 REM Query active architecture
110 SELECT CASE MEMMAP$
120     CASE "IBM_PCJR"
130         CALL INIT_PCJR_AUDIO
140     CASE "IBM_AT"
150         CALL INIT_VGA_256_COLOR
160     CASE ELSE
170         CALL INIT_STANDARD_CGA
180 END SELECT
```

---

## Engine Implementation (`eval_builtins.c` & `bios.c`)

In `engine/src/eval/eval_builtins.c`:

```c
else if (strcmp(uname, "MEMMAP$") == 0) {
    if (arg_count != 0) {
        err->code = 13; err->message = "MEMMAP$ expects no arguments"; return res;
    }
    const char *model_str = "(None)";
    BiosContext *bios = vm_get_bios(vm);
    if (bios) {
        switch (bios_get_model(bios)) {
            case BIOS_MODEL_IBM_PC:   model_str = "IBM_PC"; break;
            case BIOS_MODEL_IBM_XT:   model_str = "IBM_XT"; break;
            case BIOS_MODEL_IBM_AT:   model_str = "IBM_AT"; break;
            case BIOS_MODEL_IBM_PCJR: model_str = "IBM_PCJR"; break;
        }
    }
    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), model_str, strlen(model_str));
    return res;
}
```

---

## Error Codes

| Error Code | Name | Condition |
|------------|------|-----------|
| 13 | Type Mismatch / Argument Error (`ERR_TYPE_MISMATCH`) | Arguments passed to `MEMMAP$` (expects 0 arguments) |

---

## Cross-References

- **`BIOS`** — Executes BIOS interrupts or switches active hardware profiles (`BIOS "PCAT"`).
- **`Devices_Hardware/virtual_mach/VMEM.md`** — Segmented memory access (`DEF SEG`, `PEEK`, `POKE`).
- **`IBM_BIOS_Porting.md`** — Freestanding BIOS subsystem architecture.

---

## Proposed Expansion or Changes

1. **Custom Architecture Registry**: Allow custom retro machine models (e.g. `"TANDY_1000"`, `"AMSTRAD_PC1512"`) to register custom memory map descriptors.
2. **`MEMMAP(segment%)` Query**: Add numeric query to retrieve the physical base address of a virtual segment handle.
