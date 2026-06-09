/*
 * =====================================================================
 * BASIC++ Dialect Profile — Commodore BASIC v2 (C64B)
 * =====================================================================
 *
 * MACHINE:   Commodore 64 (primary), VIC-20, PET, C128, Plus/4
 * AUTHOR:    Microsoft (licensed by Commodore, v2.0)
 * YEAR:      1982 (C64), 1977 (PET 2001)
 * ROM SIZE:  8KB BASIC ROM + 8KB KERNAL ROM
 * RAM:       38911 bytes free on C64 (64KB total, ROM overlaid)
 * CPU:       MOS 6510 @ 1.023 MHz (NTSC) / 0.985 MHz (PAL)
 * DISPLAY:   Text: 40x25 (PETSCII charset);
 *            Bitmap: 320x200 (hi-res, 2 colors/cell),
 *                    160x200 (multi-color, 4 colors/cell)
 * SOUND:     SID chip (6581/8580) — 3 voices, ADSR envelopes,
 *            4 waveforms, filters, ring modulation
 * SPRITES:   8 hardware sprites (24x21 pixels, expandable)
 * STORAGE:   Cassette (Datasette), floppy disk (1541)
 *
 * HISTORICAL NOTES:
 *   The most popular home computer ever made (~17 million units).
 *   Commodore licensed Microsoft BASIC v2.0 in 1977 for the PET
 *   and NEVER UPDATED IT for the C64, despite the C64 having
 *   vastly superior hardware (SID, VIC-II sprites, bitmap modes).
 *
 *   This means C64 users must use PEEK/POKE/SYS for ALL hardware
 *   access: sprites, sound, graphics, color, interrupts. This is
 *   both the C64's greatest weakness (programming difficulty) and
 *   its greatest strength (direct hardware control = demoscene).
 *
 *   Famous startup:
 *     **** COMMODORE 64 BASIC V2 ****
 *     64K RAM SYSTEM  38911 BASIC BYTES FREE
 *     READY.
 *
 * DIALECT QUIRKS:
 *   - "READY." prompt (with period — distinctive)
 *   - 10-column PRINT zones
 *   - No ON ERROR GOTO (errors are fatal)
 *   - No WHILE/WEND, no DO/LOOP
 *   - No CLS (use PRINT CHR$(147) to clear screen)
 *   - No TRON/TROFF
 *   - PEEK(addr) / POKE addr,val — the C64 way of life
 *   - SYS addr — call machine language routine
 *   - WAIT addr, mask [, xor] — wait for hardware state
 *   - RND(0) — returns last random number (not re-seed)
 *   - CLR — clear variables (but keep program)
 *   - Max line number: 63999
 *   - GET A$ — read single keypress (no Enter needed)
 *   - STATUS (ST) — disk/tape status variable
 *   - TI / TI$ — jiffy clock (1/60th second ticks)
 *   - USR() — user function call (machine language)
 *
 * WHAT'S IMPLEMENTED:
 *   [x] Core Microsoft BASIC (PRINT, LET, IF/THEN, FOR/NEXT...)
 *   [x] DATA/READ/RESTORE
 *   [x] DIM arrays, DEF FN
 *   [x] String functions (LEFT$, RIGHT$, MID$, LEN, ASC, CHR$...)
 *   [x] PEEK/POKE (via virtual memory segment)
 *   [x] SYS (stubbed — prints address)
 *   [x] CLR
 *   [x] GET (single character input)
 *   [x] "READY." prompt
 *   [x] 10-column PRINT zones
 *
 * WHAT'S MISSING (for 100% compatibility):
 *   [ ] C64-specific PEEK/POKE memory map
 *       Key addresses for VIC-II:
 *         $D000-$D02E: VIC-II registers (sprites, scroll, color)
 *         $D800-$DBFF: Color RAM (1000 bytes)
 *         $0400-$07FF: Screen RAM (default)
 *       Key addresses for SID:
 *         $D400-$D418: SID registers (3 voices + filter)
 *       Fix: Map these to VDev-backed virtual memory. POKE to
 *       SID registers could trigger actual audio output.
 *   [ ] WAIT addr, mask [, xor] (hardware polling)
 *   [ ] RND(0) behavior (return last random, don't reseed)
 *       Fix: Add rnd_zero_repeats flag to DialectConfig.
 *       When set, RND(0) returns the previous RND value.
 *   [ ] TI / TI$ jiffy clock
 *       Fix: Map to platform_get_ticks() / 60 for TI,
 *       format as HH:MM:SS for TI$.
 *   [ ] STATUS (ST) variable — disk/tape error status
 *   [ ] Commodore PETSCII character set
 *       C64 uses PETSCII, not ASCII. Upper/lower case are
 *       reversed from standard ASCII. POKE 53272,23 for
 *       lowercase mode.
 *       Note: BASIC++ operates in ASCII. Perfect PETSCII
 *       emulation would require a character mapping layer.
 *   [ ] Direct-mode screen editing (cursor keys to edit lines)
 *   [ ] USR() machine language call
 *
 * HOW TO MODIFY THIS FILE:
 *   1. Edit c64_config to change feature flags.
 *   2. For SID sound: create a VDev audio device that responds
 *      to POKE at $D400-$D418. See vdev.h for the interface.
 *   3. For VIC-II sprites: create a VDev sprite device that
 *      responds to POKE at $D000-$D02E.
 *   4. For RND(0): add has_rnd_zero_repeat to DialectConfig,
 *      check in builtin_rnd() in builtins.c.
 *   5. Rebuild. Test with: DIALECT "C64B"
 *
 * ANSI C89/C90 COMPLIANT
 * =====================================================================
 */

#include "dialect.h"

static const DialectConfig c64_config = {
    DIALECT_COMMODORE,          /* id */
    "Commodore BASIC v2",       /* name */
    ':',    /* stmt_separator */
    1,      /* has_then_keyword */
    1,      /* has_let_optional */
    1,      /* has_for_next */
    1,      /* has_string_vars */
    0,      /* has_print_hash */
    0,      /* has_array_at */
    1,      /* has_rnd_function */
    1,      /* has_abs_function */
    0,      /* has_size_function: NO — uses FRE(0) */
    63999,  /* max_line_number */
    0,      /* not_eq_is_hash */
    1,      /* has_data_read */
    0,      /* has_while_wend: NO */
    0,      /* has_do_loop: NO */
    1,      /* has_extended_vars */
    0,      /* has_merge_chain: NO */
    1,      /* has_float */
    1,      /* has_dim_arrays */
    1,      /* has_string_functions */
    "READY.",   /* ready_prompt: with period! */
    10,         /* print_zone_width: 10 columns */
    0,          /* has_on_error: NO */
    0,          /* has_cls: NO (use PRINT CHR$(147)) */
    0,          /* has_tron_troff: NO */
    "C64B",     /* short_name */
    DFLAG_C64B  /* dialect_flag */
};

void dialect_register_c64(void)
{
    dialect_register(&c64_config);
}
