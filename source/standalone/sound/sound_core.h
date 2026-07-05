/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: sound_core.h
 * Subsystem: Standalone Sound & Waveform Synthesizer
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Calculates retro sound pitch frequencies and clock tick durations.
 *
 * 2. WHAT TO EXPECT:
 *    Stable mathematical pitch calculations.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Reference base frequency (default 440 Hz).
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Semitone mathematical formulas.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Ensure inputs are within valid bounds to avoid frequency clipping.
 * ===================================================================== */

#ifndef STANDALONE_SOUND_CORE_H
#define STANDALONE_SOUND_CORE_H

int sound_core_pitch_to_freq(double pitch);
int sound_core_ticks_to_ms(int ticks);

#endif /* STANDALONE_SOUND_CORE_H */
