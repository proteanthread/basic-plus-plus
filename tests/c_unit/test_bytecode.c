/**
 * @file test_bytecode.c
 * @brief Bytecode format (.BPP) serialization and pluggable detokenizer unit tests.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Verifies BLOAD/BSAVE serialization formats and custom detokenizer callback logic.
 * - Why it exists: Prevents regressions in file loading, saving, and format compatibility layers.
 * - Why it works this way: It programmatically constructs and parses BPP programs, registers a detokenizer, and checks outputs.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Test filenames, assertions, dummy detokenizer rules.
 * - What cannot be changed: Endian-safe checks, memory cleanups.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: File system is writable.
 * - Portability concerns: Endianness is handled safely. C17 standard compliant.
 */

#include "test_harness.h"
#include "bpp_boot.h"
#include "bpp_vm.h"
#include "bpp_lexer.h"
#include "bpp_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Forward declarations of inner functions tested */
BppError vm_load_program_file(VMContext *vm, const char *filename);
BppError vm_bload_program_file_bpp(VMContext *vm, const char *filename);
BppError stmt_bsave_handler(VMContext *vm, LexerContext *lex);
BppError stmt_bload_handler(VMContext *vm, LexerContext *lex);

static int dummy_detokenizer(const unsigned char *data, int len, char *out_text, int max_out) {
    /* Translate raw file starting with 0xBB signature */
    if (len > 0 && data[0] == 0xBB) {
        int wlen = snprintf(out_text, max_out, "10 PRINT \"DECODED BY CUSTOM DETOKENIZER\"\n20 END\n");
        return wlen;
    }
    return -1;
}

static bool test_pluggable_detokenizer(void) {
    BootConfig config;
    memset(&config, 0, sizeof(config));
    BootContext *boot = boot_execute(&config);
    ASSERT_TRUE(boot != NULL, "Failed to execute boot sequence");
    VMContext *vm = boot->vm;

    /* Register detokenizer */
    bytecode_set_detokenizer(dummy_detokenizer);
    ASSERT_TRUE(bytecode_get_detokenizer() == dummy_detokenizer, "Detokenizer setter/getter mismatch");

    /* Write dummy detok file starting with 0xBB signature */
    const char *test_file = "tests/c_unit/temp_detok.bas";
    FILE *fp = fopen(test_file, "wb");
    ASSERT_TRUE(fp != NULL, "Failed to create temp detok file");
    fputc(0xBB, fp);
    fputc('A', fp);
    fclose(fp);

    /* Try to load via detokenizer path in vm_bload_program_file_bpp */
    BppError err = vm_bload_program_file_bpp(vm, test_file);
    ASSERT_EQ_INT(err.code, 0, "BLOAD with custom detokenizer failed");

    /* Verify memory program matches */
    MemoryContext *mem = vm_get_mem(vm);
    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &count);
    ASSERT_EQ_INT((int)count, 2, "Wrong program line count loaded");
    ASSERT_EQ_STR(lines[0].text, "PRINT \"DECODED BY CUSTOM DETOKENIZER\"", "First line content mismatch");

    /* Cleanups */
    remove(test_file);
    bytecode_set_detokenizer(NULL);
    boot_shutdown(boot);
    return true;
}

static bool test_bytecode_roundtrip(void) {
    BootConfig config;
    memset(&config, 0, sizeof(config));
    BootContext *boot = boot_execute(&config);
    ASSERT_TRUE(boot != NULL, "Failed to execute boot sequence");
    VMContext *vm = boot->vm;
    MemoryContext *mem = boot->mem;

    mem_program_insert(mem, 10.0, "PRINT \"ROUNDTRIP\"");
    mem_program_insert(mem, 20.0, "END");

    /* Setup filename */
    const char *test_bpp = "tests/c_unit/temp_roundtrip.bpp";
    remove(test_bpp);

    /* Create dummy lexer to pass string filename to BSAVE */
    LexerContext *lex = lex_init(mem, " \"tests/c_unit/temp_roundtrip.bpp\" ");
    ASSERT_TRUE(lex != NULL, "Lexer init failed");

    /* Call handler directly */
    BppError err = stmt_bsave_handler(vm, lex);
    ASSERT_EQ_INT(err.code, 0, "BSAVE handler failed");
    lex_shutdown(lex);

    /* Verify file exists and has BPP magic */
    FILE *fp = fopen(test_bpp, "rb");
    ASSERT_TRUE(fp != NULL, "BPP file was not created");
    unsigned char magic[4];
    size_t r = fread(magic, 1, 4, fp);
    ASSERT_EQ_INT((int)r, 4, "Magic too short");
    ASSERT_EQ_INT(magic[0], 'B', "Magic 0 mismatch");
    ASSERT_EQ_INT(magic[1], 'P', "Magic 1 mismatch");
    ASSERT_EQ_INT(magic[2], 'P', "Magic 2 mismatch");
    ASSERT_EQ_INT(magic[3], 0x1A, "Magic 3 mismatch");
    fclose(fp);

    /* Reset VM memory and verify loading back */
    mem_program_clear(mem);
    
    LexerContext *lex2 = lex_init(mem, " \"tests/c_unit/temp_roundtrip.bpp\" ");
    err = stmt_bload_handler(vm, lex2);
    ASSERT_EQ_INT(err.code, 0, "BLOAD handler failed");
    lex_shutdown(lex2);

    /* Verify loaded lines */
    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &count);
    ASSERT_EQ_INT((int)count, 2, "BLOAD program line count mismatch");
    ASSERT_EQ_STR(lines[0].text, "PRINT \"ROUNDTRIP\"", "BLOAD first line content mismatch");

    /* Cleanups */
    remove(test_bpp);
    boot_shutdown(boot);
    return true;
}

TestEntry bytecode_test_suite[] = {
    {"pluggable_detokenizer", test_pluggable_detokenizer},
    {"bytecode_roundtrip",    test_bytecode_roundtrip},
    {NULL, NULL}
};
