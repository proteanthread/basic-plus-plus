// FILENAME: dev_tape.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libdevice, vdev.c
// NEEDS: libkernel (vdev.h)
// Header definitions for Tape & Cassette Audio Synthesizer Bridge (CAS1..8:, TAP1..8:, C:, MAG1..8:).
//
// ---- Includes ----

#ifndef DEVICE_DEV_TAPE_H
#define DEVICE_DEV_TAPE_H

#include "device/vdev.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    TAPE_MOD_KANSAS_CITY = 1, // 300 baud FSK (1200/2400 Hz)
    TAPE_MOD_TRS80_500   = 2, // TRS-80 Model I 500 baud FSK
    TAPE_MOD_TRS80_1500  = 3, // TRS-80 Model III/4 1500 baud FSK
    TAPE_MOD_ATARI       = 4, // Atari 8-bit FSK (3990/5327 Hz)
    TAPE_MOD_COMMODORE   = 5, // Commodore PET/64 pulse durations
    TAPE_MOD_RAW_PCM     = 6  // Direct PCM samples
} TapeModulation;

// @brief Creates a concrete VDev instance for tape/cassette devices.
VDev dev_tape_create(const char *name);

#ifdef __cplusplus
}
#endif

#endif // DEVICE_DEV_TAPE_H
