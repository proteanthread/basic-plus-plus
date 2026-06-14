/*
 * dialect_gwbs.c -- GW-BASIC / BASICA (Microsoft, 1983)
 *
 * The default dialect for running David Ahl's games. 104/104
 * BASIC Computer Games compile successfully in GWBS mode.
 *
 * "Ok" prompt (lowercase k is correct). Max line 65529 because
 * 65530-65535 were reserved internally by the original.
 *
 * Graphics gaps: CIRCLE needs arc and aspect ratio params
 * (parser currently handles the simple case). PAINT needs a
 * real flood-fill in gfxbuf.c -- stack-based scanline fill
 * would work. WINDOW/VIEW are viewport transforms that would
 * sit between user coords and gfxbuf coords.
 *
 * Sound: PLAY and SOUND work on Windows (PC speaker API).
 * On Linux/Mac they're no-ops for now. A vdev audio backend
 * could make these portable.
 */

#include "dialect.h"

static const DialectConfig gwbs_config = {
    DIALECT_GW_BASIC,
    "GW-BASIC",
    ':', 1, 1, 1, 1, 0, 0, 1, 1, 0,
    65529, 0, 1, 1, 0, 1, 1, 1, 1, 1,
    "Ok", 14, 1, 1, 1,
    "GWBS", DFLAG_GWBS
};

void dialect_register_gwbs(void)
{
    dialect_register(&gwbs_config);
}
