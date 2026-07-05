#include "sound_core.h"
#include <math.h>

/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: sound_core.c
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

int sound_core_pitch_to_freq(double pitch)
{
    double freq = 440.0 * pow(2.0, (pitch - 9.0) / 12.0);
    int freq_hz = (int)(freq + 0.5);
    if (freq_hz < 37) freq_hz = 37;
    if (freq_hz > 32767) freq_hz = 32767;
    return freq_hz;
}

int sound_core_ticks_to_ms(int ticks)
{
    return ticks * 55;
}
