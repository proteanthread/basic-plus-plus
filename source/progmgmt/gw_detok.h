/* =====================================================================
 * gw_detok.h
 * Native GW-BASIC Detokenizer interface for BASIC++
 * ===================================================================== */
#ifndef GW_DETOK_H
#define GW_DETOK_H

#include <stdbool.h>
#include "../memory.h"

// Returns true if the file was successfully detokenized and loaded into the store
bool gw_detok_load(ProgramStore *store, const char *filename);

// Writes a detokenized GW-BASIC file out to a text file
bool gw_detok_to_file(const char *in_filename, const char *out_filename);

#endif /* GW_DETOK_H */
