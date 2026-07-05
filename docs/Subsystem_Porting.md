# BASIC++ Standalone Subsystem Porting Guide

This guide details how to extract and port the standalone **Virtual Filesystem (VFS)** and **PRINT USING Format Engine** libraries into other C/C++ projects with zero dependencies.

---

## 1. Virtual Filesystem (VFS) Library

The VFS library provides a lightweight, pure path-resolution mapper. It supports virtual drive mounts (e.g. `A:`, `USB:`) and relative path searches across search paths (VPATH).

### Files Required
* `source/standalone/vfs_core.h`
* `source/standalone/vfs_core.c`

### APIs & Interface

```c
#include "vfs_core.h"

/* Initialize the VFS internal state */
void vfs_core_init(void);

/* Mount a virtual drive prefix (must end with ':') to a real directory path */
int vfs_core_mount(const char *prefix, const char *target);

/* Unmount a virtual drive prefix */
int vfs_core_umount(const char *prefix);

/* List all active mounts via a callback */
void vfs_core_list_mounts(void (*print_cb)(const char *prefix, const char *target));

/* Set the VPATH search string (semicolon-separated on Windows, colon on Linux) */
void vfs_core_set_vpath(const char *path);

/* Get the current VPATH search string */
const char *vfs_core_get_vpath(void);

/* Resolve a virtual path to a real path */
int vfs_core_resolve(const char *path, char *out, int out_max, int for_write);
```

### Usage Example

```c
#include "vfs_core.h"
#include <stdio.h>

void my_print_cb(const char *prefix, const char *target) {
    printf("Mount: %s -> %s\n", prefix, target);
}

int main(void) {
    char resolved[512];
    
    vfs_core_init();
    vfs_core_mount("USB:", "/mnt/usbdrv");
    
    /* List active mounts */
    vfs_core_list_mounts(my_print_cb);
    
    /* Resolve path */
    if (vfs_core_resolve("USB:data/config.ini", resolved, sizeof(resolved), 0) == 0) {
        printf("Resolved path: %s\n", resolved); // Outputs: /mnt/usbdrv/data/config.ini
    }
    
    return 0;
}
```

---

## 2. PRINT USING Format Engine

The format engine parses and formats numbers and strings using classic GW-BASIC / QBasic style print specifiers.

### Files Required
* `source/standalone/format_using_core.h`
* `source/standalone/format_using_core.c`

### APIs & Interface

```c
#include "format_using_core.h"

/* Formats a double value according to the fmt specification.
 * *pos is advanced past the parsed format specifiers. */
int format_using_numeric(FILE *fp, const char *fmt, int flen, int *pos, double value);

/* Formats a radix/base representation of a value (binary, octal, hex, etc.) */
int format_using_radix(FILE *fp, const char *fmt, int flen, int *pos, double value, int rep);

/* Formats a string field according to the fmt specification (!, \ \, &) */
int format_using_string_field(FILE *fp, const char *fmt, int flen, int *pos, const char *str, int slen);
```

### Usage Example

```c
#include "format_using_core.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    int pos = 0;
    const char *num_fmt = "###.##";
    const char *str_fmt = "\\      \\"; // GW-BASIC string field
    
    /* Print numeric value formatted */
    format_using_numeric(stdout, num_fmt, strlen(num_fmt), &pos, 123.456);
    printf("\n"); // Outputs: 123.46
    
    /* Print string value formatted */
    pos = 0;
    format_using_string_field(stdout, str_fmt, strlen(str_fmt), &pos, "Hello World", 11);
    printf("\n"); // Outputs: Hello Wo
    
    return 0;
}
```

---

## 3. Microsoft Binary Format (MBF) Math Library

The MBF Math library converts Microsoft Binary Format float values (Single Precision 4-byte and Double Precision 8-byte) to and from standard IEEE-754 double floats. This is crucial for reading/writing legacy binary file structures.

### Files Required
* `source/standalone/gw_math_mbf_core.h`
* `source/standalone/gw_math_mbf_core.c`

### APIs & Interface

```c
#include "gw_math_mbf_core.h"

/* Convert 4-byte Single Precision MBF to double */
double gw_mbf32_to_double(const uint8_t *mbf);

/* Convert double to 4-byte Single Precision MBF */
void gw_double_to_mbf32(double val, uint8_t *mbf);

/* Convert 8-byte Double Precision MBF to double */
double gw_mbf64_to_double(const uint8_t *mbf);

/* Convert double to 8-byte Double Precision MBF */
void gw_double_to_mbf64(double val, uint8_t *mbf);
```

### Usage Example

```c
#include "gw_math_mbf_core.h"
#include <stdio.h>

int main(void) {
    uint8_t mbf_single[4];
    double value = 123.45;
    
    /* Convert double to 4-byte MBF Single */
    gw_double_to_mbf32(value, mbf_single);
    
    /* Convert 4-byte MBF Single back to double */
    double restored = gw_mbf32_to_double(mbf_single);
    printf("Restored double: %f\n", restored); // Outputs: 123.450000
    
    return 0;
}
```

---

## 4. INPUT USING Format Engine

The Formatted Input engine validates keyboard or stream inputs against GW-BASIC / QBasic style validation specifiers. It also provides Microsoft-style input prompt cursor protection (preventing backspacing past the prompt).

### Files Required
* `source/standalone/format_input_core.h`
* `source/standalone/format_input_core.c`

### APIs & Interface

```c
#include "format_input_core.h"

/* Read input line from stdin with prompt backspace protection */
int input_read_protected(char *buf, int maxlen, const char *prompt);

/* Parse an input format string to populate a format spec */
void input_parse_format(const char *fmt, int flen, InputFormatSpec *spec);

/* Validate and transform (case conversion) an input string */
int input_validate(char *buf, int len, const InputFormatSpec *spec);

/* High-level validation loop (re-prompts until input matches format) */
int format_input_using(char *buf, int maxlen, const char *fmt, int flen, const char *prompt);
```

### Usage Example

```c
#include "format_input_core.h"
#include <stdio.h>

int main(void) {
    char buffer[256];
    
    /* Reads hex characters only (H format), prompt is "Enter Hex: " */
    printf("Reading format input...\n");
    int len = format_input_using(buffer, sizeof(buffer), "H", 1, "Enter Hex: ");
    
    if (len >= 0) {
        printf("Valid Hex entered: %s\n", buffer);
    }
    
    return 0;
}
```

---

## 5. Shunting-Yard RPN Evaluator

The RPN Evaluator provides a complete, standalone Forth-like stack calculator. It parses input strings, extracts numbers and operators, maintains a double-precision stack, and evaluates math/logical operations.

### Files Required
* `source/standalone/rpn_core.h`
* `source/standalone/rpn_core.c`

### APIs & Interface

```c
#include "rpn_core.h"

/* Initialize the RPN stack state */
void rpn_core_init(RpnState *s);

/* Returns 1 if RPN stack mode is active */
int rpn_core_is_active(RpnState *s);

/* Enable or disable Rpn mode */
void rpn_core_set_active(RpnState *s, int on);

/* Evaluate a line containing numbers and operators */
int rpn_core_eval_line(RpnState *s, const char *line);
```

### Usage Example

```c
#include "rpn_core.h"
#include <stdio.h>

int main(void) {
    RpnState s;
    rpn_core_init(&s);
    rpn_core_set_active(&s, 1);
    
    /* Evaluate postfix: (3 * 4) + 5 = 17 */
    printf("Evaluating RPN: 3 4 * 5 + .\n");
    rpn_core_eval_line(&s, "3 4 * 5 + .");
    
    return 0;
}
```

---

## 6. VDev2 Device Slot Registry

The Virtual Device Layer registry manages custom client-registered hardware streams, file handles, consoles, or other custom device slots. It features case-insensitive name matching, class categorizations, capability bitfields, and safe dispatch wrappers.

### Files Required
* `source/standalone/vdev_core.h`
* `source/standalone/vdev_core.c`

### APIs & Interface

```c
#include "vdev_core.h"

/* Initialize registry table */
void vdev_core_init(void);

/* Get device by ID */
VDev *vdev_core_get(int id);

/* Register a custom device at the next free user slot (VDEV_USER+) */
int vdev_core_register(VDev *dev);

/* Register a device at a specific slot (useful for built-ins) */
int vdev_core_register_at(int slot, VDev *dev);

/* Find device slot ID by name (case-insensitive) */
int vdev_core_find_by_name(const char *name);

/* Find device slot ID by class starting from search start ID */
int vdev_core_find_by_class(VDevClass dev_class, int start_id);

/* Safe dispatch helpers */
int vdev_core_putc(VDev *d, int ch);
int vdev_core_puts(VDev *d, const char *s);
int vdev_core_getc(VDev *d);
int vdev_core_gets(VDev *d, char *buf, int max);
int vdev_core_read(VDev *d, void *buf, int len);
int vdev_core_write(VDev *d, const void *buf, int len);
long vdev_core_seek(VDev *d, long offset, int whence);
int vdev_core_ioctl(VDev *d, int cmd, void *arg);
```

### Usage Example

```c
#include "vdev_core.h"
#include <stdio.h>
#include <string.h>

static int my_puts(VDev *d, const char *s) {
    (void)d;
    return printf("[CustomDevice] %s", s);
}

int main(void) {
    VDev dev;
    memset(&dev, 0, sizeof(dev));
    dev.name = "MYDEV:";
    dev.dev_class = VDCLASS_CUSTOM;
    dev.dev_caps = VDCAP_WRITE;
    dev.dev_puts = my_puts;
    
    vdev_core_init();
    int slot = vdev_core_register(&dev);
    
    if (slot >= 0) {
        VDev *retrieved = vdev_core_get(slot);
        vdev_core_puts(retrieved, "Hello through VDev!\n");
    }
    
    return 0;
}
```

---

## 7. Memory Pools Subsystem

The Memory Pools subsystem provides generic, bump-allocated contiguous memory pools with 8-byte alignment guarantees. It avoids dynamic runtime heap fragmentation by pre-allocating blocks of memory.

### Files Required
* `source/standalone/memory_core.h`
* `source/standalone/memory_core.c`

### APIs & Interface

```c
#include "memory_core.h"

/* Allocate and initialize a single memory pool of given size */
int mem_pool_init(MemoryPool *pool, long size);

/* Release the memory pool base block */
void mem_pool_free(MemoryPool *pool);

/* Bump-allocate nbytes from the pool (with 8-byte alignment) */
void *mem_pool_alloc(MemoryPool *pool, long nbytes);

/* Reset the pool used watermark back to zero (instant reclaim) */
void mem_pool_reset(MemoryPool *pool);

/* Return remaining bytes in the pool */
long mem_pool_available(MemoryPool *pool);
```

### Usage Example

```c
#include "memory_core.h"
#include <stdio.h>

int main(void) {
    MemoryPool pool;
    
    /* Initialize pool with 1024 bytes */
    if (mem_pool_init(&pool, 1024) == 0) {
        /* Allocate a chunk */
        int *arr = (int *)mem_pool_alloc(&pool, 10 * sizeof(int));
        if (arr != NULL) {
            arr[0] = 42;
            printf("Allocated first element: %d\n", arr[0]);
        }
        
        printf("Available bytes: %ld\n", mem_pool_available(&pool));
        
        /* Reclaim memory instant pool reset */
        mem_pool_reset(&pool);
        printf("Available bytes after reset: %ld\n", mem_pool_available(&pool));
        
        mem_pool_free(&pool);
    }
    
    return 0;
}
```

---

## 8. Compacting String Pool & Garbage Collector

The GC String Pool subsystem manages variable-length dynamic string allocations without runtime heap fragmentation. It uses a single continuous pre-allocated pool and compacts it via a callback-driven mark-sweep garbage collector that updates pointer targets dynamically.

### Files Required
* `source/standalone/stringpool_core.h`
* `source/standalone/stringpool_core.c`

### APIs & Interface

```c
#include "stringpool_core.h"

/* Initialize the string pool with a pre-allocated block size */
int strpool_core_init(StringPool *pool, long size);

/* Release the string pool resources */
void strpool_core_free(StringPool *pool);

/* Allocate a string of a given length from the pool */
char *strpool_core_alloc(StringPool *pool, int length);

/* Callback type used during garbage collection mark phase */
typedef void (*GcMarkCallback)(void *context, void (*add_ref)(void *ref_context, char **ptr, const char *data, int length), void *ref_context);

/* Compact string pool, calling mark_cb to register live references and updating pointer targets */
int strpool_core_compact(StringPool *pool, GcMarkCallback mark_cb, void *context);
```

### Usage Example

```c
#include "stringpool_core.h"
#include <stdio.h>
#include <string.h>

static char *my_global_str = NULL;

void my_gc_mark(void *context, void (*add_ref)(void *ref_context, char **ptr, const char *data, int length), void *ref_context) {
    (void)context;
    if (my_global_str != NULL) {
        add_ref(ref_context, &my_global_str, my_global_str, strlen(my_global_str));
    }
}

int main(void) {
    StringPool pool;
    if (strpool_core_init(&pool, 1024) == 0) {
        my_global_str = strpool_core_alloc(&pool, 12);
        strcpy(my_global_str, "Hello World");
        
        printf("Allocated: %s\n", my_global_str);
        
        /* Perform garbage collection and compaction */
        strpool_core_compact(&pool, my_gc_mark, NULL);
        printf("Post-GC: %s\n", my_global_str);
        
        strpool_core_free(&pool);
    }
    return 0;
}
```

---

## 9. Table-Driven Tokenizer / Lexer Engine

The standalone tokenizer scans source buffers into distinct lexical tokens. It supports zero-copy string lengths, decimal/float formats, operators, identifiers, and custom punctuation filters.

### Files Required
* `source/standalone/lexer_core.h`
* `source/standalone/lexer_core.c`

### APIs & Interface

```c
#include "lexer_core.h"

/* Token types enum */
typedef enum {
    LEX_TOK_EOF = 0,
    LEX_TOK_NUMBER,
    LEX_TOK_FLOAT_LIT,
    LEX_TOK_IMAGINARY,
    LEX_TOK_STRING,
    LEX_TOK_IDENTIFIER,
    LEX_TOK_OPERATOR,
    LEX_TOK_PUNCTUATION,
    LEX_TOK_ERROR
} LexCoreTokenType;

/* Token representation */
typedef struct {
    LexCoreTokenType type;
    const char *text;
    int length;
    double number_value;
    long integer_value;
    int pos;
} LexCoreToken;

/* Scan next token from source line at *pos, advancing pos */
LexCoreToken lexer_core_next_token(const char *source, int *pos, int length);
```

### Usage Example

```c
#include "lexer_core.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    const char *src = "X = 14 + 3.14";
    int pos = 0;
    int len = strlen(src);
    
    LexCoreToken tok;
    do {
        tok = lexer_core_next_token(src, &pos, len);
        if (tok.type != LEX_TOK_EOF) {
            printf("Token: type=%d, text='%.*s'\n", tok.type, tok.length, tok.text);
        }
    } while (tok.type != LEX_TOK_EOF && tok.type != LEX_TOK_ERROR);
    
    return 0;
}
```

---

## 10. Virtual Console Buffer Character-Grid Renderer

The console buffer subsystem emulates a classic character grid terminal framebuffer, supporting independent character, foreground color, and background color matrices, alongside boundary-safe scrolling and cursor placement.

### Files Required
* `source/standalone/console_core.h`
* `source/standalone/console_core.c`

### APIs & Interface

```c
#include "console_core.h"

/* Initialize the console grid to the given width (cols) and height (rows) */
void console_grid_init(ConsoleGrid *grid, int cols, int rows);

/* Clear the console grid character array with empty spaces using a specific background color */
void console_grid_clear(ConsoleGrid *grid, unsigned char bg_color);

/* Write a single character at the current cursor position with fg/bg colors, advancing cursor */
void console_grid_write_char(ConsoleGrid *grid, char c, unsigned char fg, unsigned char bg);

/* Scroll the console viewport upward by one line, filling bottom row with spaces */
void console_grid_scroll(ConsoleGrid *grid, unsigned char fg, unsigned char bg);

/* Relocate the cursor to specific column (x) and row (y) coordinates */
void console_grid_move_cursor(ConsoleGrid *grid, int x, int y);
```

### Usage Example

```c
#include "console_core.h"
#include <stdio.h>

int main(void) {
    ConsoleGrid grid;
    console_grid_init(&grid, 80, 25);
    console_grid_clear(&grid, 0); // clear with color index 0 (black)
    
    console_grid_write_char(&grid, 'H', 7, 0); // white on black
    console_grid_write_char(&grid, 'i', 7, 0);
    
    printf("Char at (0,0): %c\n", grid.chars[0][0]);
    printf("Char at (1,0): %c\n", grid.chars[0][1]);
    
    return 0;
}
```

---

## 11. Security Sandbox Capabilities Gates Engine

The isolated security engine manages system capabilities via a central operations permission matrix. It supports custom configuration adjustments, dynamic security level settings, and path/network port validation gates.

### Files Required
* `source/standalone/security_core.h`
* `source/standalone/security_core.c`

### APIs & Interface

```c
#include "security_core.h"

/* Initialize sandbox gates to specific SecLevel preset */
void security_core_init(SecLevel level);

/* Get and set active security level */
SecLevel security_core_get_level(void);
void security_core_set_level(SecLevel level);

/* Check if an operation is allowed under the given security level */
int security_core_check_op(SecLevel level, SecOperation op);

/* Check if path accesses are within the sandbox directory constraints */
int security_core_check_path(SecLevel level, const char *path);

/* Check if a TCP/UDP port connection is permitted */
int security_core_check_port(SecLevel level, int port);
```

### Usage Example

```c
#include "security_core.h"
#include <stdio.h>

int main(void) {
    security_core_init(SEC_STANDARD);
    
    /* Check if FILE_WRITE is allowed in SEC_STANDARD */
    if (security_core_check_op(SEC_STANDARD, SECOP_FILE_WRITE)) {
        printf("Write operation permitted.\n");
    } else {
        printf("Write operation BLOCKED.\n");
    }
    
    return 0;
}
```

---

## 12. MML Music Parser & Synthesizer Core

The Music Macro Language (MML) parser processes classic BASIC music macro strings (e.g., `PLAY "C D E F G A B"`) and translates them into play/sleep callback commands dynamically.

### Files Required
* `source/standalone/mml_core.h`
* `source/standalone/mml_core.c`

### APIs & Interface

```c
#include "mml_core.h"

/* Callbacks supplied by the client to implement host sound and timing */
typedef void (*MmlPlayCallback)(void *user_data, int note, int octave, int duration, int tempo);
typedef void (*MmlSleepCallback)(void *user_data, int ms);

/* Parses the MML string and triggers note and sleep callbacks */
void mml_core_play(const char *mml_string,
                   MmlPlayCallback play_cb,
                   MmlSleepCallback sleep_cb,
                   void *user_data);
```

### Usage Example

```c
#include "mml_core.h"
#include <stdio.h>

void my_play_cb(void *user_data, int note, int octave, int duration, int tempo) {
    printf("PLAY: Note=%d Octave=%d Duration=%d ms Tempo=%d\n", note, octave, duration, tempo);
}

void my_sleep_cb(void *user_data, int ms) {
    printf("SLEEP: %d ms\n", ms);
}

int main(void) {
    mml_core_play("O4 L4 C D E F G", my_play_cb, my_sleep_cb, NULL);
    return 0;
}
```

---

## 13. Matrix Mathematics Core

The matrix math library implements portable addition, subtraction, scalar math, transposition, multiplication, and Gauss-Jordan matrix inversion operating on generic, primitive C arrays.

### Files Required
* `source/standalone/matrix_core.h`
* `source/standalone/matrix_core.c`

### APIs & Interface

```c
#include "matrix_core.h"

/* Matrix Operations */
void matrix_core_add(const double *a, const double *b, double *c, int rows, int cols);
void matrix_core_sub(const double *a, const double *b, double *c, int rows, int cols);
void matrix_core_scalar(const double *a, double scalar, double *c, int rows, int cols, char op);
void matrix_core_transpose(const double *a, double *c, int rows, int cols);
void matrix_core_multiply(const double *a, int a_rows, int a_cols,
                           const double *b, int b_rows, int b_cols,
                           double *c);
int matrix_core_invert(const double *a, double *c, int n);
```

### Usage Example

```c
#include "matrix_core.h"
#include <stdio.h>

int main(void) {
    double a[4] = { 1.0, 2.0, 3.0, 4.0 };
    double b[4] = { 5.0, 6.0, 7.0, 8.0 };
    double c[4];
    
    matrix_core_add(a, b, c, 2, 2);
    printf("C[0]=%f, C[3]=%f\n", c[0], c[3]); // Outputs: C[0]=6.000000, C[3]=12.000000
    
    return 0;
}
```

---

## 14. Software Rasterizer Engine

The graphics core implements standard software drawing algorithms on a generic byte-buffer canvas, including line drawing, circles, viewport clipping, flood fill, and ANSI terminal rendering.

### Files Required
* `source/standalone/graphics_core.h`
* `source/standalone/graphics_core.c`

### APIs & Interface

```c
#include "graphics_core.h"

/* Core functions */
void graphics_core_clear(GraphicsCanvas *canvas, int color);
void graphics_core_pset(GraphicsCanvas *canvas, int x, int y, int color);
int graphics_core_point(const GraphicsCanvas *canvas, int x, int y);

/* Vector primitives */
void graphics_core_line(GraphicsCanvas *canvas, int x1, int y1, int x2, int y2, int color);
void graphics_core_box(GraphicsCanvas *canvas, int x1, int y1, int x2, int y2, int color, int filled);
void graphics_core_circle(GraphicsCanvas *canvas, int cx, int cy, int r, int color);

/* Flood Fill algorithm using a client-supplied stack buffer */
void graphics_core_paint(GraphicsCanvas *canvas, int x, int y, int fill_color, int border_color,
                          GraphicsCorePoint *stack, int stack_capacity);

/* ANSI TTY half-block renderer */
void graphics_core_render_ansi(const GraphicsCanvas *canvas,
                               const int *palette, int max_colors,
                               GraphicsCorePrintStrCallback print_str,
                               GraphicsCorePrintCharCallback print_char,
                               void *user_data);
```

---

## 15. Mock BIOS Emulator Core

The mock BIOS subsystem simulates the BDA (BIOS Data Area), IVT (Interrupt Vector Table), ROM signatures, hardware ports (PIT, CMOS), and standard x86 BIOS interrupts (INT 10h, INT 16h, INT 21h).

### Files Required
* `source/standalone/mock_bios_core.h`
* `source/standalone/mock_bios_core.c`

### APIs & Interface

```c
#include "mock_bios_core.h"

/* Initialize mock BIOS structures in target memory */
void mock_bios_init_mem(MockBiosContext *ctx, uint8_t *mem_segment, size_t mem_size, MockBiosModel model);

/* Emulate port reads/writes */
uint8_t mock_bios_in(MockBiosContext *ctx, uint16_t port);
void mock_bios_out(MockBiosContext *ctx, uint16_t port, uint8_t val);

/* Emulate interrupt calls */
void mock_bios_interrupt(MockBiosContext *ctx, uint8_t int_num);
```

---

## 16. Bytecode Serialization Core

The bytecode serialization subsystem implements little-endian packing helpers, floating-point translation, and CRC-16 checksum calculation for compiling and loading `.BPP` container cache payloads.

### Files Required
* `source/standalone/bytecode_core.h`
* `source/standalone/bytecode_core.c`

### APIs & Interface

```c
#include "standalone/bytecode_core.h"

/* Pack/Unpack values in little-endian format */
void bytecode_core_write_le16(unsigned char *buf, unsigned int val);
unsigned int bytecode_core_read_le16(const unsigned char *buf);
void bytecode_core_write_le32(unsigned char *buf, unsigned long val);
unsigned long bytecode_core_read_le32(const unsigned char *buf);
void bytecode_core_write_le64(unsigned char *buf, unsigned long long val);
unsigned long long bytecode_core_read_le64(const unsigned char *buf);

/* Pack/Unpack double precision floats */
void bytecode_core_write_double(unsigned char *buf, double val);
double bytecode_core_read_double(const unsigned char *buf);

/* CRC-16 integrity check */
unsigned int bytecode_core_crc16(unsigned int seed, const unsigned char *data, int len);
```

---

## 17. BPE Execution Archiver Core

The archive core implements little-endian reading and writing of execution package chunks and directory structure signatures.

### Files Required
* `source/standalone/archive_core.h`
* `source/standalone/archive_core.c`

### APIs & Interface

```c
#include "standalone/archive_core.h"

/* Chunk directory read/write helpers */
void archive_core_write_le16(unsigned char *buf, unsigned int val);
unsigned int archive_core_read_le16(const unsigned char *buf);
void archive_core_write_le32(unsigned char *buf, unsigned long val);
unsigned long archive_core_read_le32(const unsigned char *buf);

/* CCITT CRC-16 utility */
unsigned int archive_core_crc16(unsigned int seed, const unsigned char *data, int len);
```

---

## 18. Configuration File Parser Core

The configuration file core scans standard INI style key=value files, trims whitespace, and triggers events through a generic callback interface.

### Files Required
* `source/standalone/config_file_core.h`
* `source/standalone/config_file_core.c`

### APIs & Interface

```c
#include "standalone/config_file_core.h"

/* Parse configuration file stream */
int config_file_core_load(const char *exe_path, ConfigFileCoreKvCb kv_cb, void *user_data, char *out_filepath, int max_filepath);
int config_file_core_load_path(const char *path, ConfigFileCoreKvCb kv_cb, void *user_data);

/* Helpers */
char *config_file_core_strip_whitespace(char *s);
int config_file_core_ci_equal(const char *a, const char *b);
void config_file_core_get_name(const char *exe_path, char *out_name, int max_len);
```

---

## 19. Variable Scope Stack Core

The scope stack core manages nesting call frame snapshots, isolates local variables, and replicates shared variables using callback interfaces to get/set variables.

### Files Required
* `source/standalone/scope_stack_core.h`
* `source/standalone/scope_stack_core.c`

### APIs & Interface

```c
#include "standalone/scope_stack_core.h"

/* Stack management */
void scope_stack_core_init(ScopeStack *ss);
void scope_stack_core_free(ScopeStack *ss);
int scope_stack_core_push(ScopeStack *ss, 
                          const BValue *variables, 
                          const BValue *string_vars,
                          const void *named_vars_src, 
                          int named_count,
                          size_t named_var_size,
                          int mode, int sub_index, int return_idx);

int scope_stack_core_pop(ScopeStack *ss,
                         BValue *variables,
                         BValue *string_vars,
                         void *named_vars_dest,
                         int *named_count_ptr,
                         size_t named_var_size,
                         ScopeStackGetNamedFn get_named_cb,
                         ScopeStackSetNamedFn set_named_cb,
                         void *cb_user_data);
```

---

## 20. Transactional Disk Journaling Core

The transaction journal tracks atomic file changes, saving original contents before overwrite operations, and provides a platform-safe rollback engine.

### Files Required
* `source/standalone/txn_core.h`
* `source/standalone/txn_core.c`

### APIs & Interface

```c
#include "standalone/txn_core.h"

/* Initialize the journal state */
void txn_core_init(TxnJournal *j);

/* Begin a transaction */
int txn_core_begin(TxnJournal *j, int mode);

/* Commit changes */
int txn_core_commit(TxnJournal *j);

/* Rollback file modifications in reverse sequence */
int txn_core_rollback(TxnJournal *j, const TxnCoreCallbacks *cbs, void *user_data, int line_num);

/* Log data write details before overwrite */
int txn_core_journal_write(TxnJournal *j, const TxnCoreCallbacks *cbs, void *user_data, int chan, long pos, int len, int line_num);
```

---

## 21. String Manipulation Algorithms Core

The string algorithms core contains conversion functions and location indexing utilities.

### Files Required
* `source/standalone/string_algo_core.h`
* `source/standalone/string_algo_core.c`

### APIs & Interface

```c
#include "standalone/string_algo_core.h"

/* Convert numbers to hex/octal/binary representations */
int string_algo_core_hex(unsigned long val, char *dest, int max_len);
int string_algo_core_oct(unsigned long val, char *dest, int max_len);
int string_algo_core_bin(unsigned long val, char *dest, int max_len);

/* Substring position index matching */
int string_algo_core_instr(const char *haystack, int hl, const char *needle, int nl, int start_off);
```

---

## 22. Linear Congruential Generator RNG Core

The random number generator core handles seed advancing and float scaling logic.

### Files Required
* `source/standalone/rand_core.h`
* `source/standalone/rand_core.c`

### APIs & Interface

```c
#include "standalone/rand_core.h"

/* Advance LCG seed state */
void rand_core_advance(uint64_t *seed);

/* Return random double in range [0, 1) */
double rand_core_float(uint64_t *seed);

/* Return random integer in range [1, max] */
long rand_core_int(uint64_t *seed, long max);
```

---

## 23. Keyword Override Mapping Registry Core

The override core manages dynamic keyword replacement tables.

### Files Required
* `source/standalone/override_core.h`
* `source/standalone/override_core.c`

### APIs & Interface

```c
#include "standalone/override_core.h"

/* Manage key-value override redirects */
void override_core_init(OverrideCoreEntry *table, int num_ids);
int override_core_set(OverrideCoreEntry *table, int num_ids, int id, const char *text, int is_protected);
const char *override_core_get(const OverrideCoreEntry *table, int num_ids, int id);
void override_core_clear(OverrideCoreEntry *table, int num_ids, int id);
```

---

## 24. Error Message Registry Core

The error message registry core maps numeric error codes to human-readable string values and manages customizable registrations.

### Files Required
* `source/standalone/error_registry_core.h`
* `source/standalone/error_registry_core.c`

### APIs & Interface

```c
#include "standalone/error_registry_core.h"

/* Manage numeric error registries */
void error_registry_core_init(ErrorCoreEntry *table, int *count, int max_entries);
int error_registry_core_register(ErrorCoreEntry *table, int *count, int max_entries, int error_code, const char *syntax_name);
const char *error_registry_core_lookup(const ErrorCoreEntry *table, int count, int error_code);
```

---

## 25. Keyword Dynamic Attribute Registry Core

The keyword dynamic properties core enables key-value metadata configuration per keyword.

### Files Required
* `source/standalone/keyword_props_core.h`
* `source/standalone/keyword_props_core.c`

### APIs & Interface

```c
#include "standalone/keyword_props_core.h"

/* Query and set keyword configuration attributes */
void keyword_props_core_init(KwPropSet *table, int num_kws);
int keyword_props_core_set(KwPropSet *table, int num_kws, int kw, const char *name, const char *value);
const char *keyword_props_core_get(const KwPropSet *table, int num_kws, int kw, const char *name);
int keyword_props_core_is_on(const KwPropSet *table, int num_kws, int kw, const char *name);
int keyword_props_core_get_int(const KwPropSet *table, int num_kws, int kw, const char *name, int default_val);
void keyword_props_core_remove(KwPropSet *table, int num_kws, int kw, const char *name);
```

---

## 26. Language Translation Mapping Presets Core

The alias language core holds GWBASIC/QBASIC multilingual keyword translation pack definitions.

### Files Required
* `source/standalone/alias_lang_core.h`
* `source/standalone/alias_lang_core.c`

### APIs & Interface

```c
#include "standalone/alias_lang_core.h"

/* Load and query language pack details */
const AliasLangPack *alias_lang_core_find(const char *code);
const AliasLangPack *alias_lang_core_get_all(int *out_count);
```

---

## 27. Legacy Device Alias Mapping Core

The device alias core resolves logical name redirections and holds platform legacy preset arrays.

### Files Required
* `source/standalone/device_alias_core.h`
* `source/standalone/device_alias_core.c`

### APIs & Interface

```c
#include "standalone/device_alias_core.h"

/* Manage device alias mapping and resolutions */
void device_alias_core_init(DeviceAlias *table, int *count, int max_aliases);
int device_alias_core_set(DeviceAlias *table, int *count, int max_aliases, const char *alias, const char *target, int direction, int dialect);
const DeviceAlias *device_alias_core_resolve(const DeviceAlias *table, int count, const char *name);
int device_alias_core_remove(DeviceAlias *table, int *count, const char *alias);
int device_alias_core_set_active(DeviceAlias *table, int count, const char *alias, int active);
int device_alias_core_load_preset(DeviceAlias *table, int *count, int max_aliases, int dialect_id);
```

---

## 28. Virtual Console Core

The Virtual Console Core provides the default implementation for virtualized console input and output (CON: and ERR:), separated from the device registry dispatch.

### Files Required
* `source/standalone/console/console_core.h`
* `source/standalone/console/console_core.c`

### APIs & Interface

```c
#include "standalone/console/console_core.h"

/* Initialize the virtual console settings */
void console_core_init(ConsoleState *cs);

/* Write a single character or string to the console buffer */
void console_core_write_char(ConsoleState *cs, char c);
void console_core_write_str(ConsoleState *cs, const char *s);

/* Read a character from console keyboard queue */
int console_core_read_char(ConsoleState *cs);
```

---

## 29. Virtual Terminal Core

The Virtual Terminal Core implements standard TUI control, ANSI escape sequence parsing, and window sizing abstraction.

### Files Required
* `source/standalone/vt/vt_core.h`
* `source/standalone/vt/vt_core.c`

### APIs & Interface

```c
#include "standalone/vt/vt_core.h"

/* Initialize the virtual terminal buffer state */
void vt_core_init(VtState *vt);

/* Handle incoming character streams containing potential ANSI escape sequences */
void vt_core_process_char(VtState *vt, char c);

/* Query terminal screen columns and rows */
void vt_core_get_dimensions(const VtState *vt, int *cols, int *rows);
```

---

## 30. Object/OOP Model Core

The Object Model Core manages UDT definitions, namespace routing, field offset lookups, and static class variable allocations.

### Files Required
* `source/standalone/object/object_core.h`
* `source/standalone/object/object_core.c`

### APIs & Interface

```c
#include "standalone/object/object_core.h"

/* Look up offset and attributes of a field in a UDT */
int object_core_find_field(const UserTypeDef *utd, const char *name, int len);

/* Retrieve reference to a class field value by instance pointer */
BValue *object_core_get_field_ref(Instance *inst, int field_idx);
```

---

## 31. Standalone Compiler Library & CLI

The Compiler Library compiles abstract syntax trees built by the parser into target C17 or Python source files, packaged with detokenization mappings.

### Files Required
* `source/codegen/ast.c`
* `source/codegen/codegen.c`
* `source/codegen/target.c`
* `source/codegen/bytecode.c`
* `source/codegen/detok.c`
* `source/progmgmt/compiler.c`
* `source/standalone/bppc.c`

### APIs & Interface

```c
#include "compiler.h"
#include "codegen.h"

/* Compile a BASIC source file into standard target C or Python code */
int compiler_compile(const char *src_path, const char *out_path, TargetType target);

/* Detokenize a compiled binary line for display */
int detok_line(const uint8_t *tokens, int len, char *out_buf, int max_len);
```
