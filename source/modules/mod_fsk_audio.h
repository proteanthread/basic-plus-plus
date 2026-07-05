#ifndef MOD_FSK_AUDIO_H
#define MOD_FSK_AUDIO_H

#include "runtime.h"

// Initialize FSK audio subsystem (if needed)
void fsk_audio_init(void);

// MOTOR [ON|OFF] - play or stop TAPE.WAV/TAPE.MP3
void fsk_audio_motor(int state);

// CSAVE - Save program store to WAV format using FSK
// Returns 0 on success, non-zero on error.
int fsk_audio_csave(const char *filename, RuntimeState *rt);

// CLOAD - Load program store from WAV format using FSK
// Returns 0 on success, non-zero on error.
int fsk_audio_cload(const char *filename, RuntimeState *rt);

#endif
