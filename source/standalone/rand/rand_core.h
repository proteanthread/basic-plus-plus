/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: rand_core.h
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
 * File: rand_core.h
 * ===================================================================== */

#ifndef BASICPP_STANDALONE_RAND_CORE_H
#define BASICPP_STANDALONE_RAND_CORE_H

#include <stdint.h>

void rand_core_advance(uint64_t *seed);
double rand_core_float(uint64_t *seed);
long rand_core_int(uint64_t *seed, long max);

#endif // BASICPP_STANDALONE_RAND_CORE_H
