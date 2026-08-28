// FILENAME: aalib.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libkernel (aalib.c)
// NEEDS: platform, memory
// Implements virtual device and graphics rendering logic for aalib.
//
// ---- Includes ----

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

#endif // AALIB_H
