/**
 * @file aalib.h
 * @brief Standalone, clean-room, system-agnostic mini-aalib public interface.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Declares the structs, parameter constants, and functions for
 *   the ASCII Art Library (aalib) emulation layer.
 * - Why it exists: Provides a zero-dependency, vended subset of aalib that compiles
 *   everywhere to convert graphical framebuffers to readable ASCII art.
 * - Why it works this way: It models the original aalib API (aa_autoinit, aa_render,
 *   aa_flush) but maps terminal characters to averaged 2x2 grayscale pixel block
 *   averages internally.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Additional configuration parameters in structs, if needed.
 * - What cannot be changed: Function signatures (must match legacy aalib API).
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Terminal support for basic cursor home escape sequence (\033[H).
 * - Portability concerns: Pure ISO C17. No external curses/slang dependencies.
 */

#ifndef AALIB_H
#define AALIB_H

#include <stdint.h>
#include <stddef.h>

typedef struct aa_hardware_params {
    int dummy;
} aa_hardware_params;

extern struct aa_hardware_params aa_defparams;

typedef struct aa_renderparams {
    int dummy;
} aa_renderparams;

extern struct aa_renderparams aa_defrenderparams;

typedef struct aa_context {
    int scr_width;
    int scr_height;
    int img_width;
    int img_height;
    uint8_t *img_buffer;
    char *char_buffer;
} aa_context;

aa_context *aa_autoinit(const struct aa_hardware_params *params);
void aa_close(aa_context *ctx);

int aa_scrwidth(aa_context *ctx);
int aa_scrheight(aa_context *ctx);
int aa_imgwidth(aa_context *ctx);
int aa_imgheight(aa_context *ctx);

uint8_t *aa_image(aa_context *ctx);

int aa_render(aa_context *ctx, const struct aa_renderparams *params, int x1, int y1, int x2, int y2);
void aa_flush(aa_context *ctx);

#endif /* AALIB_H */
