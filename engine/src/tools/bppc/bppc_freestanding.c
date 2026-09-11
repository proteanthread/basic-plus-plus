// FILENAME: bppc_freestanding.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (bppc_transpile.c)
// NEEDS: libcore (bppc_internal.h, bppc_freestanding.h), libplatform
// Implements freestanding and bare-metal UEFI code generator for bppc.
//
// ---- Includes ----

#include "tools/bppc_freestanding.h"
#include "tools/bppc_internal.h"
#include "platform/platform.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"

#define bpp_fprintf(f, ...) platform_file_printf((void *)(f), __VA_ARGS__)

bool bppc_is_freestanding_or_uefi(void) {
    return (g_target_dialect == TARGET_FREESTANDING || g_target_dialect == TARGET_UEFI);
}

// Emits freestanding C17 and UEFI protocol headers with zero libc dependencies.
void bppc_emit_freestanding_headers(void *out, bool is_uefi) {
    bpp_fprintf(out, "// ============================================================================\n");
    bpp_fprintf(out, "// Freestanding C17 Systems Target (Zero Libc Dependencies)\n");
    bpp_fprintf(out, "// Target: %s\n", is_uefi ? "64-bit UEFI Application (BOOTX64.EFI)" : "Freestanding Bare-Metal C17");
    bpp_fprintf(out, "// ============================================================================\n\n");

    bpp_fprintf(out, "#include <stdint.h>\n");
    bpp_fprintf(out, "#include <stdbool.h>\n");
    bpp_fprintf(out, "#include <stddef.h>\n\n");

    // Sized Ordinal Primitive Types
    bpp_fprintf(out, "// --- Sized Ordinal Primitives & Typed Pointers ---\n");
    bpp_fprintf(out, "typedef uint8_t   U8;\n");
    bpp_fprintf(out, "typedef int8_t    I8;\n");
    bpp_fprintf(out, "typedef uint16_t  U16;\n");
    bpp_fprintf(out, "typedef int16_t   I16;\n");
    bpp_fprintf(out, "typedef uint32_t  U32;\n");
    bpp_fprintf(out, "typedef int32_t   I32;\n");
    bpp_fprintf(out, "typedef uint64_t  U64;\n");
    bpp_fprintf(out, "typedef int64_t   I64;\n");
    bpp_fprintf(out, "typedef uintptr_t USIZE;\n");
    bpp_fprintf(out, "typedef intptr_t  ISIZE;\n\n");

    // Volatile Hardware Memory and Port Accessors
    bpp_fprintf(out, "// --- Hardware Memory-Mapped I/O & Port Accessors ---\n");
    bpp_fprintf(out, "#define BPP_MEM8(addr)   (*(volatile uint8_t  *)(uintptr_t)(addr))\n");
    bpp_fprintf(out, "#define BPP_MEM16(addr)  (*(volatile uint16_t *)(uintptr_t)(addr))\n");
    bpp_fprintf(out, "#define BPP_MEM32(addr)  (*(volatile uint32_t *)(uintptr_t)(addr))\n");
    bpp_fprintf(out, "#define BPP_MEM64(addr)  (*(volatile uint64_t *)(uintptr_t)(addr))\n");
    bpp_fprintf(out, "#define BPP_PORT8(port)  (*(volatile uint8_t  *)(uintptr_t)(port))\n");
    bpp_fprintf(out, "#define BPP_PORT16(port) (*(volatile uint16_t *)(uintptr_t)(port))\n\n");

    if (is_uefi) {
        bpp_fprintf(out, "// --- 64-bit UEFI Firmware Protocol Interface ---\n");
        bpp_fprintf(out, "#if defined(_MSC_VER)\n");
        bpp_fprintf(out, "#define EFIAPI __cdecl\n");
        bpp_fprintf(out, "#elif defined(__GNUC__) || defined(__clang__)\n");
        bpp_fprintf(out, "#define EFIAPI __attribute__((ms_abi))\n");
        bpp_fprintf(out, "#else\n");
        bpp_fprintf(out, "#define EFIAPI\n");
        bpp_fprintf(out, "#endif\n\n");

        bpp_fprintf(out, "typedef void *EFI_HANDLE;\n");
        bpp_fprintf(out, "typedef uint64_t EFI_STATUS;\n");
        bpp_fprintf(out, "#define EFI_SUCCESS 0ULL\n\n");

        bpp_fprintf(out, "typedef struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;\n");
        bpp_fprintf(out, "struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL {\n");
        bpp_fprintf(out, "    EFI_STATUS (EFIAPI *Reset)(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, bool ExtendedVerification);\n");
        bpp_fprintf(out, "    EFI_STATUS (EFIAPI *OutputString)(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, const int16_t *String);\n");
        bpp_fprintf(out, "    EFI_STATUS (EFIAPI *TestString)(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, const int16_t *String);\n");
        bpp_fprintf(out, "    EFI_STATUS (EFIAPI *QueryMode)(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, size_t ModeNumber, size_t *Columns, size_t *Rows);\n");
        bpp_fprintf(out, "    EFI_STATUS (EFIAPI *SetMode)(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, size_t ModeNumber);\n");
        bpp_fprintf(out, "    EFI_STATUS (EFIAPI *SetAttribute)(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, size_t Attribute);\n");
        bpp_fprintf(out, "    EFI_STATUS (EFIAPI *ClearScreen)(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This);\n");
        bpp_fprintf(out, "    EFI_STATUS (EFIAPI *SetCursorPosition)(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, size_t Column, size_t Row);\n");
        bpp_fprintf(out, "    EFI_STATUS (EFIAPI *EnableCursor)(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, bool Visible);\n");
        bpp_fprintf(out, "    void *Mode;\n");
        bpp_fprintf(out, "};\n\n");

        bpp_fprintf(out, "typedef struct {\n");
        bpp_fprintf(out, "    char _hdr[44];\n");
        bpp_fprintf(out, "    int16_t *FirmwareVendor;\n");
        bpp_fprintf(out, "    uint32_t FirmwareRevision;\n");
        bpp_fprintf(out, "    EFI_HANDLE ConsoleInHandle;\n");
        bpp_fprintf(out, "    void *ConIn;\n");
        bpp_fprintf(out, "    EFI_HANDLE ConsoleOutHandle;\n");
        bpp_fprintf(out, "    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *ConOut;\n");
        bpp_fprintf(out, "    EFI_HANDLE StandardErrorHandle;\n");
        bpp_fprintf(out, "    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *StdErr;\n");
        bpp_fprintf(out, "    void *RuntimeServices;\n");
        bpp_fprintf(out, "    void *BootServices;\n");
        bpp_fprintf(out, "    size_t NumberOfTableEntries;\n");
        bpp_fprintf(out, "    void *ConfigurationTable;\n");
        bpp_fprintf(out, "} EFI_SYSTEM_TABLE;\n\n");

        bpp_fprintf(out, "static EFI_SYSTEM_TABLE *g_SystemTable = NULL;\n");
        bpp_fprintf(out, "static EFI_HANDLE g_ImageHandle = NULL;\n\n");

        bpp_fprintf(out, "static void bpp_uefi_print(const char *s) {\n");
        bpp_fprintf(out, "    if (!g_SystemTable || !g_SystemTable->ConOut || !s) return;\n");
        bpp_fprintf(out, "    int16_t u16[512];\n");
        bpp_fprintf(out, "    size_t i = 0;\n");
        bpp_fprintf(out, "    for (; s[i] && i < 510; i++) u16[i] = (int16_t)(unsigned char)s[i];\n");
        bpp_fprintf(out, "    u16[i] = 0;\n");
        bpp_fprintf(out, "    g_SystemTable->ConOut->OutputString(g_SystemTable->ConOut, u16);\n");
        bpp_fprintf(out, "}\n\n");

        bpp_fprintf(out, "static void bpp_uefi_cls(void) {\n");
        bpp_fprintf(out, "    if (g_SystemTable && g_SystemTable->ConOut) {\n");
        bpp_fprintf(out, "        g_SystemTable->ConOut->ClearScreen(g_SystemTable->ConOut);\n");
        bpp_fprintf(out, "    }\n");
        bpp_fprintf(out, "}\n\n");
    }
}

// Emits the program entry point: efi_main for UEFI, or _start for bare metal.
void bppc_emit_freestanding_entry(void *out, bool is_uefi) {
    if (is_uefi) {
        bpp_fprintf(out, "EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {\n");
        bpp_fprintf(out, "    g_ImageHandle = ImageHandle;\n");
        bpp_fprintf(out, "    g_SystemTable = SystemTable;\n\n");
    } else {
        bpp_fprintf(out, "void _start(void) {\n");
    }
}

// Emits the program exit sequence: returns EFI_SUCCESS or enters halt loop.
void bppc_emit_freestanding_exit(void *out, bool is_uefi) {
    if (is_uefi) {
        bpp_fprintf(out, "\n    return EFI_SUCCESS;\n}\n");
    } else {
        bpp_fprintf(out, "\n    while (1) { ; }\n}\n");
    }
}

// Emits print statements for freestanding and UEFI modes.
void bppc_emit_freestanding_print(void *out, const char *args, bool is_uefi) {
    if (!args) return;
    char text[512] = {0};
    const char *p = bppc_trim((char *)args);

    if (p[0] == '"') {
        size_t len = runtime_strlen(p);
        if (len >= 2 && p[len - 1] == '"') {
            runtime_memcpy(text, p + 1, len - 2);
            text[len - 2] = '\0';
        } else {
            runtime_strcpy(text, p + 1);
        }
    } else {
        runtime_strncpy(text, p, sizeof(text) - 1);
    }

    if (is_uefi) {
        bpp_fprintf(out, "    bpp_uefi_print(\"%s\\r\\n\");\n", text);
    } else {
        bpp_fprintf(out, "    // Freestanding print (UART/VRAM buffer): %s\n", text);
    }
}
