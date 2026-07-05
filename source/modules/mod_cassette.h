/**
 * BASIC++
 * Subsystem: Decoupled Cassette Tape FSK Encoder/Decoder
 */
#ifndef MOD_CASSETTE_H
#define MOD_CASSETTE_H

// Opaque forward declaration to keep decoupled
typedef struct RuntimeState RuntimeState;

void cassette_save(const char *filename, RuntimeState *rt, int line_num);
void cassette_load(const char *filename, RuntimeState *rt, int line_num);

#endif
