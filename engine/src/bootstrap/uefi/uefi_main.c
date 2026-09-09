// FILENAME: uefi_main.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: UEFI Firmware Image (\EFI\BOOT\BOOTX64.EFI)
// NEEDS: bootstrap/uefi_main.h, basicpp_sys.h
// Standalone Bare-Metal UEFI Firmware ROM BASIC Environment.
//

#include "bootstrap/uefi_main.h"
#include "basicpp_sys.h"

#define ROM_MAX_LINES 64
#define ROM_LINE_LEN  128

typedef struct {
    int  line_num;
    char text[ROM_LINE_LEN];
} RomLine;

static RomLine s_rom_program[ROM_MAX_LINES];
static int     s_rom_line_count = 0;

static void uefi_put_str(EFI_SYSTEM_TABLE *st, const char *s) {
    if (!st || !st->ConOut || !s) return;
    CHAR16 wbuf[ROM_LINE_LEN + 4];
    int widx = 0;
    for (int i = 0; s[i] && widx < ROM_LINE_LEN; i++) {
        if (s[i] == '\n') {
            wbuf[widx++] = (CHAR16)'\r';
            wbuf[widx++] = (CHAR16)'\n';
        } else {
            wbuf[widx++] = (CHAR16)(uint8_t)s[i];
        }
    }
    wbuf[widx] = 0;
    st->ConOut->OutputString(st->ConOut, wbuf);
}

static char uefi_read_char(EFI_SYSTEM_TABLE *st) {
    if (!st || !st->ConIn) return 0;
    EFI_INPUT_KEY key;
    while (true) {
        EFI_STATUS status = st->ConIn->ReadKeyStroke(st->ConIn, &key);
        if (status == EFI_SUCCESS) {
            if (key.UnicodeChar != 0) {
                return (char)(key.UnicodeChar & 0x7F);
            }
        }
    }
}

static void uefi_read_line(EFI_SYSTEM_TABLE *st, char *buf, int max_len) {
    int idx = 0;
    while (idx < max_len - 1) {
        char c = uefi_read_char(st);
        if (c == '\r' || c == '\n') {
            uefi_put_str(st, "\n");
            break;
        } else if (c == '\b' || c == 0x7F) {
            if (idx > 0) {
                idx--;
                uefi_put_str(st, "\b \b");
            }
        } else if (c >= 32 && c <= 126) {
            buf[idx++] = c;
            char echo[2] = {c, '\0'};
            uefi_put_str(st, echo);
        }
    }
    buf[idx] = '\0';
}

static uint64_t uefi_parse_val(const char **p) {
    while (**p == ' ' || **p == '\t') (*p)++;
    uint64_t v = 0;
    if (**p == '&' && ((*p)[1] == 'H' || (*p)[1] == 'h')) {
        *p += 2;
        while ((**p >= '0' && **p <= '9') || (**p >= 'A' && **p <= 'F') ||
               (**p >= 'a' && **p <= 'f')) {
            v = (v << 4) + ((**p >= 'a') ? (**p - 'a' + 10) :
                            ((**p >= 'A') ? (**p - 'A' + 10) : (**p - '0')));
            (*p)++;
        }
    } else {
        while (**p >= '0' && **p <= '9') {
            v = v * 10 + (**p - '0');
            (*p)++;
        }
    }
    return v;
}

static void uefi_exec_print(EFI_SYSTEM_TABLE *st, const char *p) {
    while (*p == ' ' || *p == '\t') p++;
    if (*p == '"') {
        p++;
        char out[ROM_LINE_LEN];
        int o = 0;
        while (*p && *p != '"' && o < ROM_LINE_LEN - 1) out[o++] = *p++;
        out[o] = '\0';
        uefi_put_str(st, out);
    } else {
        uint64_t val = uefi_parse_val(&p);
        char buf[32];
        sys_itoa((sys_i64)val, buf, 10);
        uefi_put_str(st, buf);
    }
    uefi_put_str(st, "\n");
}

static void uefi_exec_poke(EFI_SYSTEM_TABLE *st, const char *p) {
    uint64_t addr = uefi_parse_val(&p);
    while (*p == ' ' || *p == ',' || *p == '\t') p++;
    uint64_t val = uefi_parse_val(&p);
    sys_mem_write8((uintptr_t)addr, (sys_u8)val);
    uefi_put_str(st, "Ok\n");
}

static void uefi_exec_peek(EFI_SYSTEM_TABLE *st, const char *p) {
    while (*p == '(' || *p == '[' || *p == ' ') p++;
    uint64_t addr = uefi_parse_val(&p);
    sys_u8 val = sys_mem_read8((uintptr_t)addr);
    char buf[32];
    sys_itoa((sys_i64)val, buf, 10);
    uefi_put_str(st, buf);
    uefi_put_str(st, "\n");
}

static void uefi_store_line(int num, const char *text) {
    for (int i = 0; i < s_rom_line_count; i++) {
        if (s_rom_program[i].line_num == num) {
            if (*text == '\0') {
                for (int j = i; j < s_rom_line_count - 1; j++) {
                    s_rom_program[j] = s_rom_program[j + 1];
                }
                s_rom_line_count--;
            } else {
                sys_memcpy(s_rom_program[i].text, text, ROM_LINE_LEN);
            }
            return;
        }
    }
    if (s_rom_line_count < ROM_MAX_LINES && *text != '\0') {
        int pos = s_rom_line_count++;
        while (pos > 0 && s_rom_program[pos - 1].line_num > num) {
            s_rom_program[pos] = s_rom_program[pos - 1];
            pos--;
        }
        s_rom_program[pos].line_num = num;
        sys_memcpy(s_rom_program[pos].text, text, ROM_LINE_LEN);
    }
}

static void uefi_cmd_list(EFI_SYSTEM_TABLE *st) {
    for (int i = 0; i < s_rom_line_count; i++) {
        char nbuf[16];
        sys_itoa((sys_i64)s_rom_program[i].line_num, nbuf, 10);
        uefi_put_str(st, nbuf);
        uefi_put_str(st, " ");
        uefi_put_str(st, s_rom_program[i].text);
        uefi_put_str(st, "\n");
    }
    uefi_put_str(st, "Ok\n");
}

static void uefi_exec_stmt(EFI_SYSTEM_TABLE *st, const char *cmd) {
    while (*cmd == ' ' || *cmd == '\t') cmd++;
    if (*cmd == '\0') return;
    if (cmd[0] == 'P' && cmd[1] == 'R' && cmd[2] == 'I' && cmd[3] == 'N' && cmd[4] == 'T') {
        uefi_exec_print(st, cmd + 5);
    } else if (cmd[0] == '?' && (cmd[1] == ' ' || cmd[1] == '"' || cmd[1] == '&' || (cmd[1] >= '0' && cmd[1] <= '9'))) {
        uefi_exec_print(st, cmd + 1);
    } else if (cmd[0] == 'P' && cmd[1] == 'O' && cmd[2] == 'K' && cmd[3] == 'E') {
        uefi_exec_poke(st, cmd + 4);
    } else if (cmd[0] == 'P' && cmd[1] == 'E' && cmd[2] == 'E' && cmd[3] == 'K') {
        uefi_exec_peek(st, cmd + 4);
    } else if (cmd[0] == 'C' && cmd[1] == 'L' && cmd[2] == 'S') {
        if (st && st->ConOut) st->ConOut->ClearScreen(st->ConOut);
    } else if (cmd[0] == 'R' && cmd[1] == 'E' && cmd[2] == 'S' && cmd[3] == 'E' && cmd[4] == 'T') {
        if (st && st->RuntimeServices && st->RuntimeServices->ResetSystem) {
            st->RuntimeServices->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, NULL);
        }
    } else if (cmd[0] == 'H' && cmd[1] == 'E' && cmd[2] == 'L' && cmd[3] == 'P') {
        uefi_put_str(st, "Commands: PRINT, POKE, PEEK, MEM[a]=v, CLS, RUN, LIST, NEW, RESET, EXIT\nOk\n");
    } else {
        uefi_put_str(st, "Syntax Error\n");
    }
}

static void uefi_cmd_run(EFI_SYSTEM_TABLE *st) {
    for (int i = 0; i < s_rom_line_count; i++) {
        uefi_exec_stmt(st, s_rom_program[i].text);
    }
    uefi_put_str(st, "Ok\n");
}

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    (void)ImageHandle;
    if (!SystemTable || !SystemTable->ConOut) return EFI_SUCCESS;
    SystemTable->ConOut->ClearScreen(SystemTable->ConOut);
    uefi_put_str(SystemTable, "BASIC++ Standalone ROM Edition v6.5.2\n");
    uefi_put_str(SystemTable, "Bare-Metal Freestanding UEFI Firmware Target\n");
    uefi_put_str(SystemTable, "Cold Boot Memory: 16 MB Available.\n\nOk\n");

    char line[ROM_LINE_LEN];
    while (true) {
        uefi_put_str(SystemTable, "> ");
        uefi_read_line(SystemTable, line, ROM_LINE_LEN);
        const char *p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p >= '0' && *p <= '9') {
            int num = (int)uefi_parse_val(&p);
            while (*p == ' ' || *p == '\t') p++;
            uefi_store_line(num, p);
        } else if (p[0] == 'R' && p[1] == 'U' && p[2] == 'N' && (p[3] == '\0' || p[3] == ' ')) {
            uefi_cmd_run(SystemTable);
        } else if (p[0] == 'L' && p[1] == 'I' && p[2] == 'S' && p[3] == 'T') {
            uefi_cmd_list(SystemTable);
        } else if (p[0] == 'N' && p[1] == 'E' && p[2] == 'W') {
            s_rom_line_count = 0;
            uefi_put_str(SystemTable, "Ok\n");
        } else if (p[0] == 'E' && p[1] == 'X' && p[2] == 'I' && p[3] == 'T') {
            break;
        } else {
            uefi_exec_stmt(SystemTable, p);
        }
    }
    return EFI_SUCCESS;
}
