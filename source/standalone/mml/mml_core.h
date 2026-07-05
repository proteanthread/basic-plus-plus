/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: mml_core.h
 * Subsystem: Sound Music Macro Language Composer
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Parses MML sound strings into pitch/frequency arrays.
 *
 * 2. WHAT TO EXPECT:
 *    Translates macros (notes, tempo, octave) to sound events.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Macro syntax keys, note limits.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    MML syntax scanner.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If notes skip or sound wrong, verify macro bounds.
 * ===================================================================== */

/* =====================================================================
 * PORTABLE MML MUSIC PARSER
 * File: mml_core.h
 * ===================================================================== */

#ifndef STANDALONE_MML_CORE_H
#define STANDALONE_MML_CORE_H

typedef struct MmlState {
    int octave;
    int note_len;
    int tempo;
} MmlState;

/* Client-supplied tone callback: freq_hz is the note pitch (0 for rest), dur_ms is the time */
typedef void (*MmlSoundCallback)(void *user_data, int freq_hz, int dur_ms);

/* Client-supplied sleep callback for pauses (rests) */
typedef void (*MmlSleepCallback)(void *user_data, int dur_ms);

/* Parses and plays MML music macro language string.
 * Returns 0 on success, or non-zero on syntax error. */
int mml_core_play(const char *mml_str, int length,
                  MmlSoundCallback sound_cb,
                  MmlSleepCallback sleep_cb,
                  void *user_data);

#endif /* STANDALONE_MML_CORE_H */
