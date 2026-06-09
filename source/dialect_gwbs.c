/*
 * dialect_gwbs.c -- GW-BASIC (IBM PC)
 *
 * Microsoft, 1983. Shipped with MS-DOS 1.0 through 5.0.
 * The standard BASIC for the IBM PC era. 104/104 David Ahl
 * BASIC Computer Games compile in this dialect mode.
 *
 * "Ok" prompt (lowercase k). Max line 65529 (65530-65535 reserved).
 * 14-column print zones. WHILE/WEND, ON ERROR, SOUND, PLAY, DRAW.
 *
 * TODO: CIRCLE with arc/aspect parameters
 * TODO: PAINT flood fill
 * TODO: WINDOW/VIEW viewport mapping
 */

#include "dialect.h"

static const DialectConfig gwbs_config = {
    DIALECT_GW_BASIC,
    "GW-BASIC",
    ':', 1, 1, 1, 1, 0, 0, 1, 1, 0,
    65529, 0, 1, 1, 1, 1, 1, 1, 1, 1,
    "Ok", 14, 1, 1, 1,
    "GWBS", DFLAG_GWBS
};

void dialect_register_gwbs(void)
{
    dialect_register(&gwbs_config);
}
