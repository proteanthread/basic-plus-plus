#include "trans.h"
#include "../codegen.h"

int trans_emit_c89(FILE *out, AstLine *lines, int line_count, ProgramStore *program, const TargetConfig *target)
{
    // Reuse existing C89 emitter
    return codegen_emit(out, lines, line_count, program, target);
}
