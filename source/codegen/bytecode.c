/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: bytecode.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Stored program editor commands (RENUM, DELETE), compiler driver pipeline (BASIC-to-C), and bytecode serializers.
 *
 * 2. WHAT TO EXPECT:
 *    Code generator constructs self-contained C89 files. Bytecode serializes code to files.
 *
 * 3. WHAT CAN BE CHANGED:
 *    C89 codegen shims, editor warnings, target language mapping layout.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    AST translation loops, bytecode file format specs.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Verify target C compiler settings. If transpiled C file has compilation warnings, check codegen expressions mapping.
 * ===================================================================== */

 // ---
 // BASIC++ Interpreter - bytecode.c
 // ---
 //
 // Bytecode format (.BPP) serializer/deserializer
 //
 // IMPLEMENTATION:
 // The .bpp format stores program lines in a binary container
 // with a 16-byte version-tagged header. Each line is stored as:
 // [2-byte line number LE] [2-byte text length LE] [text bytes]
 //
 // The source text is stored verbatim (not further tokenized),
 // because the interpreter's Lexer re-tokenizes on execution.
 // This keeps the format simple and maximally compatible.
 //
 // Binary I/O uses fread/fwrite with unsigned char buffers and
 // manual byte packing to avoid struct alignment issues across
 // compilers. All multi-byte values are little-endian.
 //
//
// HOW TO EXTEND:
//   Adding support for a new statement in code generation:
//   1. Add the AST node type in ast.h.
//   2. Add the emit case in this file's switch statement.
//   3. Generate the corresponding C code output.
 // ---

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bytecode.h"
#include "dialect.h"
#include "config.h"
#include "pcode.h"
#include "bytecode_bpp.h"
#include "runtime.h"
#include "../console.h"

// --- Little-endian helpers ---
 // Manual byte packing avoids endianness and alignment issues.
static void write_le16(unsigned char *buf, unsigned int val)
{
 buf[0] = (unsigned char)(val & 0xFF);
 buf[1] = (unsigned char)((val >> 8) & 0xFF);
}

static unsigned int read_le16(const unsigned char *buf)
{
 return (unsigned int)buf[0] |
 ((unsigned int)buf[1] << 8);
}

static void write_le32(unsigned char *buf, unsigned long val)
{
    buf[0] = (unsigned char)(val & 0xFF);
    buf[1] = (unsigned char)((val >> 8) & 0xFF);
    buf[2] = (unsigned char)((val >> 16) & 0xFF);
    buf[3] = (unsigned char)((val >> 24) & 0xFF);
}

static unsigned long read_le32(const unsigned char *buf)
{
    return (unsigned long)buf[0] |
           ((unsigned long)buf[1] << 8) |
           ((unsigned long)buf[2] << 16) |
           ((unsigned long)buf[3] << 24);
}

static void write_le64(unsigned char *buf, unsigned long long val)
{
    buf[0] = (unsigned char)(val & 0xFF);
    buf[1] = (unsigned char)((val >> 8) & 0xFF);
    buf[2] = (unsigned char)((val >> 16) & 0xFF);
    buf[3] = (unsigned char)((val >> 24) & 0xFF);
    buf[4] = (unsigned char)((val >> 32) & 0xFF);
    buf[5] = (unsigned char)((val >> 40) & 0xFF);
    buf[6] = (unsigned char)((val >> 48) & 0xFF);
    buf[7] = (unsigned char)((val >> 56) & 0xFF);
}

static unsigned long long read_le64(const unsigned char *buf)
{
    return (unsigned long long)buf[0] |
           ((unsigned long long)buf[1] << 8) |
           ((unsigned long long)buf[2] << 16) |
           ((unsigned long long)buf[3] << 24) |
           ((unsigned long long)buf[4] << 32) |
           ((unsigned long long)buf[5] << 40) |
           ((unsigned long long)buf[6] << 48) |
           ((unsigned long long)buf[7] << 56);
}

static void write_double(unsigned char *buf, double val)
{
    union {
        double d;
        unsigned long long u;
    } temp;
    temp.d = val;
    write_le64(buf, temp.u);
}

static double read_double(const unsigned char *buf)
{
    union {
        double d;
        unsigned long long u;
    } temp;
    temp.u = read_le64(buf);
    return temp.d;
}

static unsigned int bpp_crc16_update(unsigned int seed, const unsigned char *data, int len)
{
    unsigned int crc = seed;
    int i, j;
    for (i = 0; i < len; i++) {
        crc ^= (unsigned int)data[i] << 8;
        for (j = 0; j < 8; j++) {
            if (crc & 0x8000)
                crc = (crc << 1) ^ 0x1021;
            else
                crc = crc << 1;
            crc &= 0xFFFF;
        }
    }
    return crc;
}

static int get_operand_category(unsigned char op)
{
    switch (op) {
        case PCODE_NOP:
        case PCODE_HALT:
        case PCODE_STOP:
        case PCODE_REM:
        case PCODE_DATA:
        case PCODE_LINE:
        case PCODE_POP:
        case PCODE_PUSH_ZERO:
        case PCODE_PUSH_ONE:
        case PCODE_LOAD_STRVAR:
        case PCODE_STORE_STRVAR:
        case PCODE_LOAD_AT:
        case PCODE_STORE_AT:
        case PCODE_ADD:
        case PCODE_SUB:
        case PCODE_MUL:
        case PCODE_DIV:
        case PCODE_MOD:
        case PCODE_POW:
        case PCODE_NEG:
        case PCODE_CONCAT:
        case PCODE_CMP_EQ:
        case PCODE_CMP_NE:
        case PCODE_CMP_LT:
        case PCODE_CMP_GT:
        case PCODE_CMP_LE:
        case PCODE_CMP_GE:
        case PCODE_AND:
        case PCODE_OR:
        case PCODE_NOT:
        case PCODE_RETURN:
        case PCODE_PRINT_NL:
        case PCODE_PRINT_TAB:
        case PCODE_PRINT_SPC:
        case PCODE_RESTORE:
            return 0; // No operand

        case PCODE_PUSH_INT:
        case PCODE_LOAD_VAR:
        case PCODE_STORE_VAR:
        case PCODE_ON_GOTO:
        case PCODE_ON_GOSUB:
        case PCODE_INPUT_VAR:
        case PCODE_INPUT_STRVAR:
        case PCODE_FUNC1:
        case PCODE_FUNC2:
        case PCODE_FUNC3:
        case PCODE_FOR_INIT:
        case PCODE_READ_NUM:
        case PCODE_READ_STR:
            return 1; // Integer (ival)

        case PCODE_PUSH_FLOAT:
            return 2; // Float (fval)

        case PCODE_JUMP:
        case PCODE_JUMP_FALSE:
        case PCODE_JUMP_TRUE:
        case PCODE_GOSUB:
        case PCODE_FOR_CHECK:
        case PCODE_NEXT:
            return 3; // Offset (offset)

        case PCODE_PUSH_STRING:
        case PCODE_INPUT_PROMPT:
            return 4; // String reference

        case PCODE_LOAD_NAMED:
        case PCODE_STORE_NAMED:
        case PCODE_LOAD_DIM:
        case PCODE_STORE_DIM:
        case PCODE_DIM_ALLOC:
            return 5; // Array reference

        default:
            return 0;
    }
}

// --- Global Detokenizer Hook ---
DetokenizerFn g_custom_detokenizer = NULL;

void bytecode_set_detokenizer(DetokenizerFn fn)
{
    g_custom_detokenizer = fn;
}

// --- bpp_save - Serialize program store to .bpp file. ---
int bpp_save(const ProgramStore *prog, const char *filename)
{
    FILE *fp;
    PCodeProgram pcode;
    BppNewHeader header;
    int i;
    unsigned int payload_crc = 0xFFFF;

    if (!prog || prog->count == 0) {
        printf("No program to save.\n");
        return -1;
    }

    if (!filename || filename[0] == '\0') {
        printf("No filename specified.\n");
        return -1;
    }

    // 1. Compile program store to temporary PCodeProgram
    pcode_emit_init(&pcode);
    if (pcode_compile((ProgramStore *)prog, &pcode) != 0) {
        printf("BSAVE: Compilation failed.\n");
        pcode_free(&pcode);
        return -1;
    }

    fp = fopen(filename, "wb");
    if (!fp) {
        printf("Cannot open '%s' for writing.\n", filename);
        pcode_free(&pcode);
        return -1;
    }

    // Compute source text checksum (CRC-16 stored as uint32)
    unsigned int src_crc = 0xFFFF;
    for (i = 0; i < prog->count; i++) {
        const char *txt = prog->lines[i].text;
        src_crc = bpp_crc16_update(src_crc, (const unsigned char *)txt, (int)strlen(txt));
    }

    // Build Header
    memset(&header, 0, sizeof(BppNewHeader));
    memcpy(header.magic, BPP_NEW_MAGIC, 4);
    header.format_ver = BPP_NEW_FORMAT_VER;
    header.vm_major = 4;
    header.vm_minor = 2;
    header.vm_patch = 6;
    write_le16(header.instruction_ver, 1);
    header.dialect_id = (unsigned char)dialect_get_config()->id;
    write_le16(header.dependency_count, 0);
    write_le16(header.flags, 0);
    write_le32(header.instr_count, (unsigned long)pcode.count);
    write_le32(header.str_pool_size, (unsigned long)pcode.str_used);
    write_le32(header.on_table_size, (unsigned long)pcode.on_table_count);
    write_le32(header.source_checksum, src_crc);
    write_le16(header.crc_checksum, 0); // patched later

    // Write header placeholder
    if (fwrite(&header, 1, sizeof(BppNewHeader), fp) != sizeof(BppNewHeader)) {
        printf("Write error on header of '%s'.\n", filename);
        fclose(fp);
        pcode_free(&pcode);
        return -1;
    }

    // Pack instructions
    for (i = 0; i < pcode.count; i++) {
        const PCodeInstr *inst = &pcode.instrs[i];
        unsigned char record[40];
        int rec_len = 0;

        record[0] = inst->op;
        int cat = get_operand_category(inst->op);
        record[1] = (unsigned char)cat;
        rec_len = 2;

        if (cat == 1) {
            write_le64(&record[2], (unsigned long long)inst->operand.u.ival);
            rec_len += 8;
        } else if (cat == 2) {
            write_double(&record[2], inst->operand.u.fval);
            rec_len += 8;
        } else if (cat == 3) {
            write_le32(&record[2], (unsigned long)inst->operand.u.offset);
            rec_len += 4;
        } else if (cat == 4) {
            write_le32(&record[2], (unsigned long)inst->operand.u.str.idx);
            write_le32(&record[6], (unsigned long)inst->operand.u.str.len);
            rec_len += 8;
        } else if (cat == 5) {
            memset(&record[2], 0, 33);
            strncpy((char *)&record[2], inst->operand.u.dim.name, 32);
            write_le32(&record[35], (unsigned long)inst->operand.u.dim.ndims);
            rec_len += 37;
        }

        if (fwrite(record, 1, (size_t)rec_len, fp) != (size_t)rec_len) {
            printf("Write error at instruction %d.\n", i);
            fclose(fp);
            pcode_free(&pcode);
            return -1;
        }
        payload_crc = bpp_crc16_update(payload_crc, record, rec_len);
    }

    // Write string pool
    if (pcode.str_used > 0) {
        if (fwrite(pcode.str_pool, 1, (size_t)pcode.str_used, fp) != (size_t)pcode.str_used) {
            printf("Write error at string pool.\n");
            fclose(fp);
            pcode_free(&pcode);
            return -1;
        }
        payload_crc = bpp_crc16_update(payload_crc, (const unsigned char *)pcode.str_pool, pcode.str_used);
    }

    // Write line map
    for (i = 0; i < pcode.count; i++) {
        unsigned char map_buf[4];
        write_le32(map_buf, (unsigned long)pcode.line_map[i]);
        if (fwrite(map_buf, 1, 4, fp) != 4) {
            printf("Write error at line map.\n");
            fclose(fp);
            pcode_free(&pcode);
            return -1;
        }
        payload_crc = bpp_crc16_update(payload_crc, map_buf, 4);
    }

    // Write ON tables
    for (i = 0; i < pcode.on_table_count; i++) {
        unsigned char tbl_buf[4];
        write_le32(tbl_buf, (unsigned long)pcode.on_tables[i]);
        if (fwrite(tbl_buf, 1, 4, fp) != 4) {
            printf("Write error at ON tables.\n");
            fclose(fp);
            pcode_free(&pcode);
            return -1;
        }
        payload_crc = bpp_crc16_update(payload_crc, tbl_buf, 4);
    }

    // Patch header CRC-16
    write_le16(header.crc_checksum, payload_crc);
    fseek(fp, 0, SEEK_SET);
    if (fwrite(&header, 1, sizeof(BppNewHeader), fp) != sizeof(BppNewHeader)) {
        printf("Write error while patching BPP header.\n");
    }

    fclose(fp);
    pcode_free(&pcode);

    printf("BSAVE: Saved %d instructions to '%s' (%s)\n",
           (int)read_le32(header.instr_count),
           filename, dialect_get_config()->short_name);
    return 0;
}

// --- bpp_load - Deserialize .bpp file into program store. ---
int bpp_load(ProgramStore *prog, const char *filename, void *rt_ptr)
{
    FILE *fp;
    unsigned char magic_buf[4];
    RuntimeState *rt = (RuntimeState *)rt_ptr;

    if (!filename || filename[0] == '\0') {
        printf("No filename specified.\n");
        return -1;
    }

    fp = fopen(filename, "rb");
    if (!fp) {
        printf("Cannot open '%s' for reading.\n", filename);
        return -1;
    }

    // Read first 4 bytes to check magic
    if (fread(magic_buf, 1, 4, fp) != 4) {
        printf("'%s' is too short - not a BPP file.\n", filename);
        fclose(fp);
        return -1;
    }

    fseek(fp, 0, SEEK_SET);

    // If it's old magic "BPP\x1A" (Ctrl-Z), load as V1 text lines format!
    if (magic_buf[0] == 'B' && magic_buf[1] == 'P' && magic_buf[2] == 'P' && magic_buf[3] == '\x1A') {
        // Fall back to original V1 reader
        unsigned char header[16];
        unsigned int line_count;
        int i;

        if (fread(header, 1, 16, fp) != 16) {
            fclose(fp);
            return -1;
        }

        line_count = read_le16(&header[8]);
        if ((int)line_count > prog->capacity) {
            printf("'%s' has %u lines - exceeds capacity (%d).\n", filename, line_count, prog->capacity);
            fclose(fp);
            return -1;
        }

        program_clear(prog);

        for (i = 0; i < (int)line_count; i++) {
            unsigned char rec[4];
            unsigned int linenum, textlen;
            char text_buf[MAX_LINE_LENGTH + 1];

            if (fread(rec, 1, 4, fp) != 4) {
                fclose(fp);
                return -1;
            }

            linenum = read_le16(&rec[0]);
            textlen = read_le16(&rec[2]);

            if (textlen > MAX_LINE_LENGTH) {
                fclose(fp);
                return -1;
            }

            if (textlen > 0) {
                if (fread(text_buf, 1, (size_t)textlen, fp) != (size_t)textlen) {
                    fclose(fp);
                    return -1;
                }
            }
            text_buf[textlen] = '\0';

            if (program_insert(prog, (int)linenum, text_buf) != 0) {
                fclose(fp);
                return -1;
            }
        }

        fclose(fp);
        printf("BLOAD (V1): %u lines from '%s'\n", line_count, filename);

        // Optionally switch dialect to match the file
        int file_dialect = (int)header[5];
        int curr_dialect = (int)dialect_get_config()->id;
        if (file_dialect != curr_dialect && file_dialect >= 0 && file_dialect < DIALECT_COUNT) {
            dialect_init((DialectId)file_dialect);
            dialect_apply();
            printf("Dialect: %s [%s]\n", dialect_get_config()->name, dialect_get_config()->short_name);
        }

        if (rt) {
            if (rt->loaded_pcode) {
                pcode_free((PCodeProgram *)rt->loaded_pcode);
                free(rt->loaded_pcode);
                rt->loaded_pcode = NULL;
                rt->has_loaded_pcode = 0;
            }
            rt->bytecode_only = 0;
        }
        return 0;
    }

    // Otherwise, check if new magic "BPP\x1B"
    if (magic_buf[0] != 'B' || magic_buf[1] != 'P' || magic_buf[2] != 'P' || magic_buf[3] != '\x1B') {
        if (g_custom_detokenizer != NULL) {
            printf("Checking custom detokenizer for '%s'...\n", filename);
        }
        printf("'%s' is not a .bpp file (bad magic).\n", filename);
        fclose(fp);
        return -1;
    }

    // Load V2 compiled bytecode format
    BppNewHeader header;
    if (fread(&header, 1, sizeof(BppNewHeader), fp) != sizeof(BppNewHeader)) {
        printf("Failed to read V2 BPP header.\n");
        fclose(fp);
        return -1;
    }

    // Validate VM version compatibility
    if (header.vm_major != 4) {
        printf("Incompatible VM version: %d (expected 4)\n", (int)header.vm_major);
        fclose(fp);
        return -1;
    }

    int file_dialect = (int)header.dialect_id;
    int curr_dialect = (int)dialect_get_config()->id;
    if (file_dialect != curr_dialect && file_dialect >= 0 && file_dialect < DIALECT_COUNT) {
        dialect_init((DialectId)file_dialect);
        dialect_apply();
        printf("Dialect changed to: %s [%s]\n", dialect_get_config()->name, dialect_get_config()->short_name);
    }

    int instr_count = (int)read_le32(header.instr_count);
    int str_pool_size = (int)read_le32(header.str_pool_size);
    int on_table_size = (int)read_le32(header.on_table_size);
    unsigned int source_checksum = read_le32(header.source_checksum);
    unsigned int expected_crc = read_le16(header.crc_checksum);

    // Read dependencies (dependency_count)
    int dep_count = (int)read_le16(header.dependency_count);
    if (dep_count > 0) {
        fseek(fp, dep_count * (int)sizeof(BppDependency), SEEK_CUR);
    }

    // Allocate dynamic PCodeProgram structure
    PCodeProgram *pcode = (PCodeProgram *)malloc(sizeof(PCodeProgram));
    if (!pcode) {
        fclose(fp);
        return -1;
    }
    pcode->instrs = (PCodeInstr *)malloc((size_t)instr_count * sizeof(PCodeInstr));
    pcode->count = instr_count;
    pcode->capacity = instr_count;
    pcode->str_pool = str_pool_size > 0 ? (char *)malloc((size_t)str_pool_size) : NULL;
    pcode->str_used = str_pool_size;
    pcode->str_capacity = str_pool_size;
    pcode->line_map = (int *)malloc((size_t)instr_count * sizeof(int));
    pcode->on_tables = on_table_size > 0 ? (int *)malloc((size_t)on_table_size * sizeof(int)) : NULL;
    pcode->on_table_count = on_table_size;
    pcode->on_table_capacity = on_table_size;

    unsigned int payload_crc = 0xFFFF;

    // Read and unpack instructions
    for (int i = 0; i < instr_count; i++) {
        unsigned char record[40];
        PCodeInstr *inst = &pcode->instrs[i];

        if (fread(record, 1, 2, fp) != 2) {
            free(pcode->instrs);
            if (pcode->str_pool) free(pcode->str_pool);
            free(pcode->line_map);
            if (pcode->on_tables) free(pcode->on_tables);
            free(pcode);
            fclose(fp);
            return -1;
        }

        inst->op = record[0];
        int cat = record[1];
        int rec_len = 2;

        if (cat == 1) {
            if (fread(&record[2], 1, 8, fp) != 8) goto read_err;
            inst->operand.u.ival = (long)read_le64(&record[2]);
            rec_len += 8;
        } else if (cat == 2) {
            if (fread(&record[2], 1, 8, fp) != 8) goto read_err;
            inst->operand.u.fval = read_double(&record[2]);
            rec_len += 8;
        } else if (cat == 3) {
            if (fread(&record[2], 1, 4, fp) != 4) goto read_err;
            inst->operand.u.offset = (int)read_le32(&record[2]);
            rec_len += 4;
        } else if (cat == 4) {
            if (fread(&record[2], 1, 8, fp) != 8) goto read_err;
            inst->operand.u.str.idx = (int)read_le32(&record[2]);
            inst->operand.u.str.len = (int)read_le32(&record[6]);
            rec_len += 8;
        } else if (cat == 5) {
            if (fread(&record[2], 1, 37, fp) != 37) goto read_err;
            memcpy(inst->operand.u.dim.name, &record[2], 32);
            inst->operand.u.dim.name[32] = '\0';
            inst->operand.u.dim.ndims = (int)read_le32(&record[35]);
            rec_len += 37;
        }

        payload_crc = bpp_crc16_update(payload_crc, record, rec_len);
    }

    // Read string pool
    if (str_pool_size > 0) {
        if (fread(pcode->str_pool, 1, (size_t)str_pool_size, fp) != (size_t)str_pool_size) goto read_err;
        payload_crc = bpp_crc16_update(payload_crc, (const unsigned char *)pcode->str_pool, str_pool_size);
    }

    // Read line map
    for (int i = 0; i < instr_count; i++) {
        unsigned char map_buf[4];
        if (fread(map_buf, 1, 4, fp) != 4) goto read_err;
        pcode->line_map[i] = (int)read_le32(map_buf);
        payload_crc = bpp_crc16_update(payload_crc, map_buf, 4);
    }

    // Read ON tables
    for (int i = 0; i < on_table_size; i++) {
        unsigned char tbl_buf[4];
        if (fread(tbl_buf, 1, 4, fp) != 4) goto read_err;
        pcode->on_tables[i] = (int)read_le32(tbl_buf);
        payload_crc = bpp_crc16_update(payload_crc, tbl_buf, 4);
    }

    fclose(fp);

    // Validate payload CRC
    if (payload_crc != expected_crc) {
        printf("BLOAD: Bytecode integrity CRC check failed!\n");
        free(pcode->instrs);
        if (pcode->str_pool) free(pcode->str_pool);
        free(pcode->line_map);
        if (pcode->on_tables) free(pcode->on_tables);
        free(pcode);
        return -1;
    }

    // Locate source file (.bas) adjacent to the .bpp file
    char src_name[260];
    int len = (int)strlen(filename);
    if (len >= 4) {
        strcpy(src_name, filename);
        strcpy(&src_name[len - 4], ".bas");
    } else {
        strcpy(src_name, "");
    }

    extern int fileio_load(ProgramStore *store, const char *filename);

    int source_loaded = 0;
    if (src_name[0] != '\0') {
        FILE *src_fp = fopen(src_name, "r");
        if (src_fp) {
            fclose(src_fp);
            program_clear(prog);
            if (fileio_load(prog, src_name) == 0) {
                unsigned int check_src_crc = 0xFFFF;
                for (int i = 0; i < prog->count; i++) {
                    const char *txt = prog->lines[i].text;
                    check_src_crc = bpp_crc16_update(check_src_crc, (const unsigned char *)txt, (int)strlen(txt));
                }
                if (check_src_crc == source_checksum) {
                    source_loaded = 1;
                } else {
                    printf("Warning: Cached bytecode does not match source file. Re-compiling on demand.\n");
                    free(pcode->instrs);
                    if (pcode->str_pool) free(pcode->str_pool);
                    free(pcode->line_map);
                    if (pcode->on_tables) free(pcode->on_tables);
                    free(pcode);
                    return 0; // fallback to compile-on-demand
                }
            }
        }
    }

    if (rt) {
        if (rt->loaded_pcode) {
            pcode_free((PCodeProgram *)rt->loaded_pcode);
            free(rt->loaded_pcode);
            rt->loaded_pcode = NULL;
            rt->has_loaded_pcode = 0;
        }

        rt->loaded_pcode = pcode;
        rt->has_loaded_pcode = 1;

        if (!source_loaded) {
            printf("BLOAD: Source file not found or checksum mismatch. Entering orphaned/obfuscated execution mode.\n");
            program_clear(prog);
            rt->bytecode_only = 1;
        } else {
            rt->bytecode_only = 0;
            printf("BLOAD: Loaded source '%s' and cached bytecode.\n", src_name);
        }
    }

    return 0;

read_err:
    printf("Read error while parsing BPP file.\n");
    free(pcode->instrs);
    if (pcode->str_pool) free(pcode->str_pool);
    free(pcode->line_map);
    if (pcode->on_tables) free(pcode->on_tables);
    free(pcode);
    fclose(fp);
    return -1;
}
