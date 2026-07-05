/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: detok.c (standalone tool)
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Standalone detokenizer command line tool.
 *    Converts GW-BASIC and QBASIC tokenized files into plain text.
 * ===================================================================== */

#include <stdio.h>
#include <string.h>
#include "../config.h"
#include "../detok/detokenizer.h"

int main(int argc, char **argv)
{
    const char *infile = NULL;
    const char *outfile = "output.bas";
    DetokFormat format = DETOK_FMT_AUTO;
    int i;

    printf("BASIC++ Detokenizer (detok) v" BASICPP_VERSION "\n");

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--format") == 0 && i + 1 < argc) {
            const char *f = argv[++i];
            if (strcmp(f, "gwbasic") == 0) format = DETOK_FMT_GWBASIC;
            else if (strcmp(f, "qbasic") == 0) format = DETOK_FMT_QBASIC;
            else if (strcmp(f, "auto") == 0) format = DETOK_FMT_AUTO;
            else {
                fprintf(stderr, "Unknown format: %s\n", f);
                return 1;
            }
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            outfile = argv[++i];
        } else if (argv[i][0] != '-') {
            infile = argv[i];
        }
    }

    if (!infile) {
        printf("Usage: detok [--format auto|gwbasic|qbasic] <input.bin> [-o <output.bas>]\n");
        return 1;
    }

    printf("Detokenizing %s -> %s...\n", infile, outfile);

    FILE *in = fopen(infile, "rb");
    if (!in) {
        perror("Failed to open input file");
        return 1;
    }

    FILE *out = fopen(outfile, "w");
    if (!out) {
        perror("Failed to open output file");
        fclose(in);
        return 1;
    }

    int ret = detokenize_stream(in, out, format);

    fclose(in);
    fclose(out);

    if (ret == 0) {
        printf("Detokenization completed successfully.\n");
    } else {
        printf("Detokenization failed.\n");
    }

    return ret;
}
