// FILENAME: bgi_palette.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (context.c)
// NEEDED BY: libhardware (bios_pc.c)
// NEEDED BY: libkernel (bgi_gfx.h, bgi_gfx.c, bgi_palette.c)
// NEEDS: platform, memory
// Implements virtual device and graphics rendering logic for bgi_palette.
//
// ---- Includes ----

#ifndef DEVICE_BGI_PALETTE_H
#define DEVICE_BGI_PALETTE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define BGI_PALETTE_SIZE_MAX 256

// @brief VGA DAC hardware port state machine for ports 0x3C8 and 0x3C9.
typedef struct BgiDacState {
    uint8_t write_index;  // < Port 0x3C8 PEL write address (0-255)
    uint8_t read_index;   // < Port 0x3C7 PEL read address (0-255)
    uint8_t rgb_seq;      // < Write sequence step: 0=Red, 1=Green, 2=Blue
    uint8_t pending_r;    // < Staged 6-bit Red component (0-63)
    uint8_t pending_g;    // < Staged 6-bit Green component (0-63)
} BgiDacState;

// @brief Centralized Palette State Structure.
typedef struct BgiPalette {
    uint32_t    colors[BGI_PALETTE_SIZE_MAX]; // < 256 ARGB8888 colors
    uint8_t     dac_r[BGI_PALETTE_SIZE_MAX];  // < Raw 6-bit Red values (0-63)
    uint8_t     dac_g[BGI_PALETTE_SIZE_MAX];  // < Raw 6-bit Green values (0-63)
    uint8_t     dac_b[BGI_PALETTE_SIZE_MAX];  // < Raw 6-bit Blue values (0-63)
    BgiDacState dac_state;                    // < Port I/O state machine
    int         num_colors;                   // < Active entries (4, 16, or 256)
    bool        initialized;                  // < Set true after init
} BgiPalette;

// @brief Initialize palette context with default IBM CGA/EGA/VGA heritage colors.
// @param pal Pointer to BgiPalette structure to initialize.
void bgi_palette_init(BgiPalette *pal);

// @brief Set a single palette entry using 8-bit RGB components (0-255).
void bgi_palette_set_color(BgiPalette *pal, uint8_t index, uint8_t r, uint8_t g, uint8_t b);

// @brief Set a single palette entry using 6-bit VGA DAC components (0-63).
void bgi_palette_set_dac(BgiPalette *pal, uint8_t index, uint8_t r6, uint8_t g6, uint8_t b6);

// @brief Get ARGB8888 color value for a given palette index.
uint32_t bgi_palette_get_argb(const BgiPalette *pal, uint8_t index);

// @brief Handle I/O port writes to VGA DAC ports 0x3C8 and 0x3C9.
// @param pal Pointer to palette context.
// @param port I/O port number (0x3C8 or 0x3C9).
// @param val Byte value written to the port.
void bgi_palette_handle_port_write(BgiPalette *pal, uint16_t port, uint8_t val);

// @brief Get global palette context instance.
BgiPalette* bgi_palette_get_global(void);

#endif // DEVICE_BGI_PALETTE_H
