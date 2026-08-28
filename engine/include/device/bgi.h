// FILENAME: bgi.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore, libengine, libhardware, libkernel
// NEEDS: platform, memory
// Implements virtual device and graphics rendering logic for bgi.
//
// ---- Includes ----

#ifndef DEVICE_BGI_H
#define DEVICE_BGI_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// ======================================================================
// SECTION A: Memory Layout Enumeration
// ======================================================================

// @brief Describes how VRAM is organized for a given video mode.
// This models the physical memory architecture of the target hardware.
typedef enum BGI_MemLayout {
    BGI_LAYOUT_LINEAR_ARGB8888  = 0, // < 32-bit direct-color framebuffer
    BGI_LAYOUT_INDEXED_8BPP     = 1, // < 8bpp chunky palette framebuffer
    BGI_LAYOUT_PLANAR_4PLANE    = 2, // < 4-plane EGA/VGA style layout
    BGI_LAYOUT_CELL_ATTRIBUTES  = 3, // < character + attribute text mode
    BGI_LAYOUT_BEAM_RACING      = 4  // < scanline-driven, no framebuffer
} BGI_MemLayout;

// ======================================================================
// SECTION B: Video Mode Descriptor
// ======================================================================

// @brief Complete video mode descriptor for the VModeGen system.
// Developers fill this struct and pass it to BGI_register_mode().
typedef struct BGI_VideoMode {
    // Descriptor identity
    char         mode_name[64];      // < 7-bit ASCII name, null-terminated
    uint32_t     mode_id;            // < Unique numeric mode identifier

    // Active display dimensions
    uint32_t     width;              // < Horizontal resolution in pixels
    uint32_t     height;             // < Vertical resolution in pixels

    // Color configuration
    uint8_t      bits_per_pixel;     // < 1, 2, 4, 8, 15, 16, 24, or 32
    uint32_t     palette_size;       // < Number of palette entries (0=direct)
    uint32_t    *palette;            // < ARGB8888 palette table (calloc'd)

    // Memory layout
    BGI_MemLayout mem_layout;        // < How VRAM bytes map to pixels

    // Text/character cell config (0 for pure graphics modes)
    uint8_t      cell_width;         // < Character cell width in pixels
    uint8_t      cell_height;        // < Character cell height in pixels

    // Display geometry
    float        aspect_ratio;       // < Physical aspect ratio (4.0f/3.0f)
    float        refresh_hz;         // < Target vertical refresh rate in Hz

    // Split-screen text window configuration
    uint8_t      text_window_cols;   // < Text window columns (e.g. 40, 80)
    uint8_t      text_window_rows;   // < Text window rows (e.g. 4, 8)
    bool         text_window_enabled;// < True if split-screen text active
} BGI_VideoMode;

// ======================================================================
// SECTION C: Standard BGI Color Constants
// ======================================================================

#define BGI_BLACK         0
#define BGI_BLUE          1
#define BGI_GREEN         2
#define BGI_CYAN          3
#define BGI_RED           4
#define BGI_MAGENTA       5
#define BGI_BROWN         6
#define BGI_LIGHTGRAY     7
#define BGI_DARKGRAY      8
#define BGI_LIGHTBLUE     9
#define BGI_LIGHTGREEN   10
#define BGI_LIGHTCYAN    11
#define BGI_LIGHTRED     12
#define BGI_LIGHTMAGENTA 13
#define BGI_YELLOW       14
#define BGI_WHITE        15

// Extended 24-bit direct color encoding (values > 15 are treated as RGB)
#define BGI_COLOR_RGB(r,g,b) \
    ((uint32_t)(0x01000000u | (((uint32_t)(r) & 0xFFu) << 16) | \
                              (((uint32_t)(g) & 0xFFu) <<  8) | \
                              ((uint32_t)(b) & 0xFFu)))
#define BGI_IS_PALETTE_COLOR(c) ((c) >= 0 && (c) <= 255)
#define BGI_IS_RGB_COLOR(c)     (((c) & 0xFF000000u) != 0)

// ======================================================================
// SECTION D: Line Style Constants
// ======================================================================

#define BGI_SOLID_LINE    0
#define BGI_DOTTED_LINE   1
#define BGI_CENTER_LINE   2
#define BGI_DASHED_LINE   3
#define BGI_USERBIT_LINE  4

// ======================================================================
// SECTION E: Fill Style Constants
// ======================================================================

#define BGI_EMPTY_FILL    0
#define BGI_SOLID_FILL    1

// ======================================================================
// SECTION F: Putimage Operation Constants
// ======================================================================

#define BGI_COPY_PUT 0
#define BGI_XOR_PUT  1
#define BGI_OR_PUT   2
#define BGI_AND_PUT  3
#define BGI_NOT_PUT  4

// ======================================================================
// SECTION G: Pre-Registered Heritage Mode IDs
// ======================================================================

#define BGI_MODE_TEXT_80x25     0
#define BGI_MODE_CGA_1        101  // < CGA SCREEN 1: 320x200, 4 colors
#define BGI_MODE_CGA_2        102  // < CGA SCREEN 2: 640x200, 2 colors
#define BGI_MODE_TANDY_3      103  // < Tandy SCREEN 3: 160x200, 16 colors
#define BGI_MODE_EGA_9        109  // < EGA SCREEN 9: 640x350, 16 colors
#define BGI_MODE_EGA_10       110  // < EGA SCREEN 10: 640x350, 2 colors
#define BGI_MODE_VGA_12       112  // < VGA SCREEN 12: 640x480, 16 colors
#define BGI_MODE_VGA_13       113  // < VGA Mode 13h: 320x200, 256 colors
#define BGI_MODE_HGC          120  // < Hercules: 720x348, 2 colors
#define BGI_MODE_MDA          121  // < IBM MDA: 720x350, 2 colors

// Home Computer Modes
#define BGI_MODE_C64_HI       200  // < C64 Hi-Res: 320x200, 16 colors
#define BGI_MODE_C64_MC       201  // < C64 Multicolor: 160x200, 4 colors
#define BGI_MODE_ZX_SPEC      210  // < ZX Spectrum: 256x192, 16 colors
#define BGI_MODE_ATARI8_8     220  // < Atari 8-bit GR.8: 320x192, 2 col
#define BGI_MODE_ATARI8_7     221  // < Atari 8-bit GR.7: 160x96, 4 col
#define BGI_MODE_ATARI_ST_L   225  // < Atari ST Low: 320x200, 16 colors
#define BGI_MODE_ATARI_ST_M   226  // < Atari ST Medium: 640x200, 4 col
#define BGI_MODE_ATARI_ST_H   227  // < Atari ST High: 640x400, 2 colors
#define BGI_MODE_APPLE2_HI    230  // < Apple II Hi-Res: 280x192, 6 col
#define BGI_MODE_AMIGA_320    240  // < Amiga OCS 320x256, 32 colors
#define BGI_MODE_AMIGA_640    241  // < Amiga OCS 640x256, 16 colors
#define BGI_MODE_MSX1         250  // < MSX1 TMS9918: 256x212, 16 colors
#define BGI_MODE_AMSTRAD_0    260  // < Amstrad CPC Mode 0: 160x200, 16c
#define BGI_MODE_BBC_M1       270  // < BBC Micro Mode 1: 160x256, 8 col

// Game Console Modes
#define BGI_MODE_NES_NTSC     300  // < NES PPU NTSC: 256x240, 54 colors
#define BGI_MODE_SNES         310  // < SNES PPU: 256x224, 32768 colors
#define BGI_MODE_GENESIS_N    320  // < Sega Genesis NTSC: 320x224, 512c
#define BGI_MODE_SMS_NTSC     330  // < Master System NTSC: 256x192, 64c
#define BGI_MODE_GBC          340  // < Game Boy Color: 160x144, 56 col
#define BGI_MODE_GG           341  // < Game Gear: 160x144, 32 colors
#define BGI_MODE_TG16         350  // < TurboGrafx-16: 256x239, 482 col
#define BGI_MODE_INTELLI      360  // < Intellivision: 159x96, 16 colors
#define BGI_MODE_COLECO       361  // < ColecoVision: 256x192, 16 colors
#define BGI_MODE_ATARI_2600   370  // < Atari 2600 TIA: 160x192, 128 col
#define BGI_MODE_ATARI_7800   371  // < Atari 7800 MARIA: 160x240, 256c
#define BGI_MODE_GB           380  // < Game Boy: 160x144, 4 colors

// Maximum number of registered modes
#define BGI_MAX_MODES 128

// ======================================================================
// SECTION H: BGI Context State
// ======================================================================

// @brief Opaque BGI graphics context holding all state.
typedef struct BGI_Context {
    // Framebuffer
    uint32_t    *framebuffer;       // < 32-bit ARGB8888 master canvas
    int          fb_width;          // < Current framebuffer width
    int          fb_height;         // < Current framebuffer height

    // Indexed VRAM (for 8bpp and lower modes)
    uint8_t     *indexed_vram;      // < Indexed pixel buffer (8bpp max)

    // Palette
    uint32_t     palette[256];      // < Current palette (ARGB8888)
    int          palette_size;      // < Number of active palette entries

    // Drawing state
    int          fg_color;          // < Current foreground color index
    int          bg_color;          // < Current background color index
    int          fill_color;        // < Current fill color index
    int          fill_style;        // < Current fill pattern style
    int          line_style;        // < Current line style
    int          line_thickness;    // < Current line thickness

    // Cursor position (current point)
    int          cp_x;              // < Current position X
    int          cp_y;              // < Current position Y

    // Viewport / clipping
    int          vp_left;           // < Viewport left edge
    int          vp_top;            // < Viewport top edge
    int          vp_right;          // < Viewport right edge
    int          vp_bottom;         // < Viewport bottom edge
    bool         vp_clip;           // < Clipping enabled

    // Text state
    int          text_font;         // < Active font index
    int          text_direction;    // < 0=horizontal, 1=vertical
    int          text_size;         // < Text magnification factor
    int          text_window_cols;  // < Active split-screen text columns
    int          text_window_rows;  // < Active split-screen text rows
    bool         text_window_enabled;// < True if split-screen text active

    // Mode management
    BGI_VideoMode modes[BGI_MAX_MODES]; // < Registered mode descriptors
    int           mode_count;       // < Number of registered modes
    int           active_mode;      // < Handle of currently active mode
    BGI_MemLayout active_layout;    // < Memory layout of active mode

    // Initialization flag
    bool          initialized;      // < True after BGI_init() succeeds
} BGI_Context;

// ======================================================================
// SECTION I: Public API — Lifecycle
// ======================================================================

// Initialize the BGI subsystem and register all heritage modes.
int  BGI_init(BGI_Context *ctx);

// Shutdown BGI and release all VRAM buffers.
void BGI_shutdown(BGI_Context *ctx);

// Register a custom video mode. Returns mode handle >= 0, or -1 on error.
int  BGI_register_mode(BGI_Context *ctx, const BGI_VideoMode *mode);

// Activate a registered mode by handle. Allocates VRAM.
int  BGI_set_mode(BGI_Context *ctx, int mode_handle);

// Activate a registered mode by mode_id constant.
int  BGI_set_mode_by_id(BGI_Context *ctx, uint32_t mode_id);

// Create and activate a custom resolution mode dynamically.
int  BGI_create_custom_mode(BGI_Context *ctx, int width, int height, int bpp, int text_cols, int text_rows, float fps);

// Configure split-screen text window dimensions and toggle.
void BGI_set_split_text_window(BGI_Context *ctx, int cols, int rows, bool enabled);

// Get the current framebuffer pointer for blitting.
const uint32_t *BGI_get_framebuffer(const BGI_Context *ctx);

// Get current framebuffer dimensions.
void BGI_get_dimensions(const BGI_Context *ctx, int *w, int *h);

// ======================================================================
// SECTION J: Public API — Drawing Primitives
// ======================================================================

// Write a single pixel to the framebuffer.
void BGI_putpixel(BGI_Context *ctx, int x, int y, int color);

// Read a single pixel from the framebuffer. Returns color index or ARGB.
int  BGI_getpixel(const BGI_Context *ctx, int x, int y);

// Set the current foreground drawing color.
void BGI_setcolor(BGI_Context *ctx, int color);

// Set the current background color.
void BGI_setbkcolor(BGI_Context *ctx, int color);

// Set fill style and color.
void BGI_setfillstyle(BGI_Context *ctx, int style, int color);

// Set line drawing style.
void BGI_setlinestyle(BGI_Context *ctx, int style, int pattern, int thickness);

// Draw a line from (x1,y1) to (x2,y2) using Bresenham's algorithm.
void BGI_line(BGI_Context *ctx, int x1, int y1, int x2, int y2);

// Draw a circle using Bresenham midpoint algorithm.
void BGI_circle(BGI_Context *ctx, int cx, int cy, int r);

// Draw an ellipse arc. sa/ea = start/end angle in degrees.
void BGI_ellipse(BGI_Context *ctx, int cx, int cy, int sa, int ea,
                 int rx, int ry);

// Draw a filled ellipse.
void BGI_fillellipse(BGI_Context *ctx, int cx, int cy, int rx, int ry);

// Draw a filled rectangle (bar).
void BGI_bar(BGI_Context *ctx, int x1, int y1, int x2, int y2);

// Draw an unfilled rectangle outline.
void BGI_rectangle(BGI_Context *ctx, int x1, int y1, int x2, int y2);

// Iterative scanline flood fill. Never recursive.
void BGI_floodfill(BGI_Context *ctx, int seed_x, int seed_y, int border);

// Move current position without drawing.
void BGI_moveto(BGI_Context *ctx, int x, int y);

// Draw line from current position to (x,y). Update CP.
void BGI_lineto(BGI_Context *ctx, int x, int y);

// Set the drawing viewport / clipping region.
void BGI_setviewport(BGI_Context *ctx, int x1, int y1, int x2, int y2,
                     bool clip);

// Clear the active viewport to the background color.
void BGI_clearviewport(BGI_Context *ctx);

// Clear the entire framebuffer to the background color.
void BGI_cleardevice(BGI_Context *ctx);

// ======================================================================
// SECTION K: Public API — Palette
// ======================================================================

// Set a single palette entry.
void BGI_setpalette(BGI_Context *ctx, int index, uint32_t argb);

// Get a palette entry.
uint32_t BGI_getpalette(const BGI_Context *ctx, int index);

// Set the entire palette from an array.
void BGI_setallpalette(BGI_Context *ctx, const uint32_t *pal, int count);

// Resolve a BGI color index to an ARGB8888 value using the active palette.
uint32_t BGI_resolve_color(const BGI_Context *ctx, int color);

// ======================================================================
// SECTION L: Public API — Text
// ======================================================================

// Render text at (x,y) using the active font and color.
void BGI_outtextxy(BGI_Context *ctx, int x, int y, const char *text);

// Set text rendering style.
void BGI_settextstyle(BGI_Context *ctx, int font, int direction, int size);

// Get text width in pixels.
int  BGI_textwidth(const BGI_Context *ctx, const char *text);

// Get text height in pixels.
int  BGI_textheight(const BGI_Context *ctx, const char *text);

// ======================================================================
// SECTION M: Public API — Image / Blit
// ======================================================================

// Get size in bytes needed to store a rectangular image region.
size_t BGI_imagesize(int x1, int y1, int x2, int y2);

// Save a rectangular region of the framebuffer to a buffer.
void BGI_getimage(const BGI_Context *ctx, int x1, int y1, int x2, int y2,
                  void *buffer);

// Paste a saved region back to the framebuffer.
void BGI_putimage(BGI_Context *ctx, int x, int y, const void *buffer, int op);

// ======================================================================
// SECTION N: Public API — Synthesize
// ======================================================================

// @brief Synthesize the active mode's VRAM (indexed, planar, etc.) into
// the 32-bit ARGB8888 master framebuffer for host blitting.
// Call this after drawing operations and before flushing to the host.
void BGI_synthesize(BGI_Context *ctx);

// ======================================================================
// SECTION O: Heritage Mode Registration
// ======================================================================

// Register all pre-defined heritage modes (CGA, EGA, VGA, NES, etc.).
void BGI_register_heritage_modes(BGI_Context *ctx);

// ======================================================================
// SECTION P: Global BGI Context Access
// ======================================================================

// Get the global BGI context (singleton for the BASIC++ engine).
BGI_Context *BGI_get_global_context(void);

// Get built-in IBM PC 8x8 font table (256 glyphs, 8 bytes each).
const uint8_t *BGI_get_font_8x8(void);

// Set and trigger pluggable host display synchronization callback.
void bgi_set_sync_hook(void (*hook)(void));
void bgi_sync(void);

// Window Presentation and HAL Routing
bool BGI_init_window(BGI_Context *ctx, const char *title);
void BGI_shutdown_window(BGI_Context *ctx);
void BGI_present(BGI_Context *ctx);
void BGI_poll_events(BGI_Context *ctx);
bool BGI_is_window_open(const BGI_Context *ctx);

// Vector DRAW Macro Statement Engine
void BGI_draw(BGI_Context *ctx, const char *command_str);

// Split-Screen Text Window Rendering
void BGI_draw_split_text(BGI_Context *ctx, int col, int row, const char *text, int scale, int color);

#endif // DEVICE_BGI_H
