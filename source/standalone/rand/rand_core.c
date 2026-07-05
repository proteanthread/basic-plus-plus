/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: rand_core.c
 * Subsystem: Linear Congruential LCG Random Generator
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Provides portable random number sequences using standard LCG.
 *
 * 2. WHAT TO EXPECT:
 *    Computes uniform float/double results from seeds.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Seed multiplier parameters.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Linear Congruential equation logic.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If random returns repeating patterns, change seed.
 * ===================================================================== */

/* =====================================================================
 * PORTABLE STANDALONE RANDOM NUMBER GENERATOR CORE
 * File: rand_core.c
 * ===================================================================== */

#include "rand_core.h"

void rand_core_advance(uint64_t *seed)
{
    if (seed) {
        *seed = (*seed * 1103515245UL + 12345UL) & 0x7FFFFFFFUL;
    }
}

double rand_core_float(uint64_t *seed)
{
    if (!seed) return 0.0;
    rand_core_advance(seed);
    if (*seed == 0) {
        *seed = 1;
    }
    return (double)*seed / 2147483648.0;
}

long rand_core_int(uint64_t *seed, long max)
{
    if (!seed) return 1;
    rand_core_advance(seed);
    if (max <= 0) {
        return 1;
    }
    return (long)(*seed % (unsigned long)max) + 1;
}
