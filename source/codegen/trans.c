#include "trans.h"

int trans_emit(FILE *out, AstLine *lines, int line_count,
               ProgramStore *program, const TargetConfig *target, TransTarget lang)
{
    switch (lang) {
        case TRANS_TARGET_C17:
            return trans_emit_c17(out, lines, line_count, program, target);
        case TRANS_TARGET_C89:
            return trans_emit_c89(out, lines, line_count, program, target);
        case TRANS_TARGET_PY3:
            return trans_emit_py3(out, lines, line_count, program, target);
        case TRANS_TARGET_PAS:
            return trans_emit_pas(out, lines, line_count, program, target);
        case TRANS_TARGET_F90:
            return trans_emit_f90(out, lines, line_count, program, target);
        default:
            fprintf(stderr, "trans: unknown target language\n");
            return -1;
    }
}
