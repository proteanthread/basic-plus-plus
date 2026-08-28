# C17 API Reference: Virtual Console Display (`device/vcon.h`)

## 1. Subsystem Overview & Responsibilities

The Virtual Console Display Subsystem (`device/vcon.h`, implemented in `engine/src/device/vcon.c`) manages up to 8 virtual text consoles (`VCON_MAX_CONSOLES`), character/attribute cell matrices (`25x80`), ANSI escape sequence parsing, cursor tracking (`LOCATE`, `CSRLIN`, `POS`), scrolling viewports (`VIEW PRINT`), and function key display bars (`KEY ON`/`OFF`) for the BASIC++ v6.5.2 engine.

Key architectural responsibilities include:
- **Character & Attribute Grid**: Maintains independent 25-row by 80-column text matrices storing 7-bit ASCII characters and 8-bit color attributes (foreground and background).
- **Persistent Cursor Tracking**: Authoritative source for active cursor position (`vcon_get_cursor()`); statement handlers must read cursor state from `VConContext` rather than assuming local cached defaults.
- **Scrolling Text Windows (`VIEW PRINT`)**: Constrains text output and line scrolling between custom top and bottom row boundaries (`vcon_set_view_print()`).
- **Function Key Labels (`KEY ON` / `KEY OFF`)**: Manages the 25th row soft-key label display bar for function keys F1 to F10.

## 2. Header Inclusion & Prerequisites

```c
#include "device/vcon.h"
#include <stdint.h>
#include <stdbool.h>
```

## 3. Data Structures & Types

```c
#define VCON_MAX_CONSOLES 8
#define VCON_ROWS         25
#define VCON_COLS         80

/* Single Virtual Console State Descriptor */
typedef struct {
    char     grid[VCON_ROWS][VCON_COLS];        /* Character matrix */
    uint8_t  attribs[VCON_ROWS][VCON_COLS];     /* Color attribute matrix */
    int      cursor_row;                        /* 1-based cursor row */
    int      cursor_col;                        /* 1-based cursor column */
    uint8_t  current_color;                     /* Active color attribute */
    bool     ansi_state;                        /* Inside ANSI sequence */
    char     ansi_buf[32];                      /* ANSI escape buffer */
    int      ansi_len;                          /* Buffer byte count */
} BppVirtualConsole;

/* Opaque Virtual Console Context */
typedef struct VConContext VConContext;
```

## 4. Function Prototypes & Operational Contracts

### Context Lifecycle & Console Selection
```c
VConContext *vcon_init(void);
void         vcon_shutdown(VConContext *ctx);

bool vcon_select(VConContext *ctx, int index);
int  vcon_get_active_index(VConContext *ctx);
void vcon_clear(VConContext *ctx, int index);
```

### Cursor & Screen Matrix Operations
```c
void vcon_write_char(VConContext *ctx, int index, int c);
void vcon_locate(VConContext *ctx, int index, int row, int col);
void vcon_get_cursor(VConContext *ctx, int index, int *row, int *col);

int  vcon_get_char_at(VConContext *ctx, int index, int row, int col);
int  vcon_get_attr_at(VConContext *ctx, int index, int row, int col);
void vcon_set_color(VConContext *ctx, int index, int fg, int bg);
void vcon_clear_screen(VConContext *ctx, int index, int mode);
void vcon_set_view_print(VConContext *ctx, int index, int top, int bottom);
void vcon_set_width(VConContext *ctx, int index, int cols);
```

### Soft-Key Label Display (`KEY ON` / `KEY OFF`)
```c
void        vcon_set_key_labels_visible(VConContext *ctx, bool visible);
bool        vcon_get_key_labels_visible(VConContext *ctx);
void        vcon_set_key_label(VConContext *ctx, int key_idx, const char *text);
const char *vcon_get_key_label(VConContext *ctx, int key_idx);
```

## 5. Architectural Invariants

- **Read-Back Requirement**: Handlers depending on persistent cursor state must query `vcon_get_cursor()`.
- **Pure 7-Bit ASCII**: Console characters strictly adhere to pure 7-bit ASCII encoding.

## 6. Code Example: Locating Cursor and Printing Text

```c
#include "device/vcon.h"

void print_status_banner(VConContext *vcon) {
    int active = vcon_get_active_index(vcon);
    vcon_locate(vcon, active, 1, 1);
    vcon_set_color(vcon, active, 15, 1); /* White on Blue */
    
    const char *title = "BASIC++ Standard Edition v6.5.2";
    for (const char *p = title; *p; p++) {
        vcon_write_char(vcon, active, *p);
    }
}
```
