// FILENAME: aalib.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (string.h)
// NEEDS: libengine (string.c)
// NEEDS: libkernel (aalib.h)
// Implements virtual device and graphics rendering logic for aalib.
//
// ---- Includes ----

#include "aalib.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#if defined(_WIN32)
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

struct aa_hardware_params aa_defparams = { 0 };
struct aa_renderparams aa_defrenderparams = { 0 };

static int query_terminal_width(void) {
#if defined(_WIN32)
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        return csbi.srWindow.Right - csbi.srWindow.Left + 1;
    }
#else
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        return w.ws_col;
    }
#endif
    return 80;
}

static int query_terminal_height(void) {
#if defined(_WIN32)
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        return csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    }
#else
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        return w.ws_row;
    }
#endif
    return 25;
}

aa_context *aa_autoinit(const struct aa_hardware_params *params) {
    (void)params;
    aa_context *ctx = (aa_context *)calloc(1, sizeof(aa_context));
    if (!ctx) return NULL;

    int term_w = query_terminal_width();
    int term_h = query_terminal_height();

    ctx->scr_width = term_w > 0 ? term_w : 80;
    ctx->scr_height = term_h > 1 ? term_h - 1 : 24;

    ctx->img_width = ctx->scr_width * 2;
    ctx->img_height = ctx->scr_height * 2;

    ctx->img_buffer = (uint8_t *)calloc(ctx->img_width * ctx->img_height, sizeof(uint8_t));
    ctx->char_buffer = (char *)calloc(ctx->scr_width * ctx->scr_height, sizeof(char));

    if (!ctx->img_buffer || !ctx->char_buffer) {
        aa_close(ctx);
        return NULL;
    }

    memset(ctx->img_buffer, 0, ctx->img_width * ctx->img_height);
    memset(ctx->char_buffer, ' ', ctx->scr_width * ctx->scr_height);

    return ctx;
}

void aa_close(aa_context *ctx) {
    if (!ctx) return;
    if (ctx->img_buffer) free(ctx->img_buffer);
    if (ctx->char_buffer) free(ctx->char_buffer);
    free(ctx);
}

int aa_scrwidth(aa_context *ctx) { return ctx ? ctx->scr_width : 0; }
int aa_scrheight(aa_context *ctx) { return ctx ? ctx->scr_height : 0; }
int aa_imgwidth(aa_context *ctx) { return ctx ? ctx->img_width : 0; }
int aa_imgheight(aa_context *ctx) { return ctx ? ctx->img_height : 0; }

uint8_t *aa_image(aa_context *ctx) { return ctx ? ctx->img_buffer : NULL; }

int aa_render(aa_context *ctx, const struct aa_renderparams *params, int x1, int y1, int x2, int y2) {
    (void)params;
    (void)x1; (void)y1; (void)x2; (void)y2;
    if (!ctx || !ctx->img_buffer || !ctx->char_buffer) return -1;

    static const char ramp[] = " .:-=+*#%@";
    int ramp_len = sizeof(ramp) - 1;

    for (int y = 0; y < ctx->scr_height; y++) {
        for (int x = 0; x < ctx->scr_width; x++) {
            int p00 = ctx->img_buffer[(y * 2) * ctx->img_width + (x * 2)];
            int p10 = ctx->img_buffer[(y * 2) * ctx->img_width + (x * 2 + 1)];
            int p01 = ctx->img_buffer[(y * 2 + 1) * ctx->img_width + (x * 2)];
            int p11 = ctx->img_buffer[(y * 2 + 1) * ctx->img_width + (x * 2 + 1)];

            int avg = (p00 + p10 + p01 + p11) / 4;
            int idx = avg * ramp_len / 256;
            if (idx >= ramp_len) idx = ramp_len - 1;

            ctx->char_buffer[y * ctx->scr_width + x] = ramp[idx];
        }
    }

    return 0;
}

void aa_flush(aa_context *ctx) {
    if (!ctx || !ctx->char_buffer) return;

    printf("\033[H");
    for (int y = 0; y < ctx->scr_height; y++) {
        fwrite(&ctx->char_buffer[y * ctx->scr_width], sizeof(char), ctx->scr_width, stdout);
        putchar('\n');
    }
    fflush(stdout);
}
