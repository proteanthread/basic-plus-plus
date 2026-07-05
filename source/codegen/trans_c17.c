#include "trans.h"
#include "../codegen.h"

int trans_emit_c17(FILE *out, AstLine *lines, int line_count, ProgramStore *program, const TargetConfig *target)
{
    // Delegate to the existing C code generator which produces compliant C code.
    // In strict C17 mode, we can add specific C17 features or #pragmas here.
    return codegen_emit(out, lines, line_count, program, target);
}
