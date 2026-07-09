/**
 * BASIC++
 * Subsystem: Decoupled Cassette Tape FSK Encoder/Decoder
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include "mod_cassette.h"
#include "runtime.h"
#include "../errors.h"
#include "../fileio.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// WAV Header struct
#pragma pack(push, 1)
typedef struct {
    char riff_tag[4];
    uint32_t riff_length;
    char wave_tag[4];
    char fmt_tag[4];
    uint32_t fmt_length;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
    char data_tag[4];
    uint32_t data_length;
} WavHeader;
#pragma pack(pop)

// IBM PC Cassette: 1000 Hz = 1 bit (1ms), 2000 Hz = 0 bit (0.5ms)
#define SAMPLE_RATE 44100
#define FREQ_0 2000
#define FREQ_1 1000

static void write_wav_bit(uint8_t **out, uint32_t *len, uint32_t *cap, int bit) {
    int freq = bit ? FREQ_1 : FREQ_0;
    int samples = SAMPLE_RATE / freq;
    
    // Ensure capacity
    if (*len + samples > *cap) {
        *cap = (*cap * 2) + samples;
        *out = (uint8_t*)realloc(*out, *cap);
    }
    
    // Write one full sine wave cycle
    for (int i = 0; i < samples; i++) {
        double t = (double)i / (double)samples;
        double val = sin(t * 2.0 * M_PI);
        uint8_t pcm = (uint8_t)((val + 1.0) * 127.5);
        (*out)[(*len)++] = pcm;
    }
}

static void write_wav_byte(uint8_t **out, uint32_t *len, uint32_t *cap, uint8_t byte) {
    // 1 start bit (0), 8 data bits (LSB first), 1 stop bit (1)
    write_wav_bit(out, len, cap, 0); // Start bit
    for (int i = 0; i < 8; i++) {
        write_wav_bit(out, len, cap, (byte >> i) & 1);
    }
    write_wav_bit(out, len, cap, 1); // Stop bit
}

static void encode_fsk_wav(const char *filename, const uint8_t *data, size_t data_len) {
    uint32_t cap = 1024 * 1024;
    uint8_t *pcm = (uint8_t*)malloc(cap);
    uint32_t pcm_len = 0;

    // Leader: 256 bits of '1'
    for (int i = 0; i < 256; i++) {
        write_wav_bit(&pcm, &pcm_len, &cap, 1);
    }

    // Sync bit: '0'
    write_wav_bit(&pcm, &pcm_len, &cap, 0);

    // Data payload
    for (size_t i = 0; i < data_len; i++) {
        write_wav_byte(&pcm, &pcm_len, &cap, data[i]);
    }

    // Trailer: 256 bits of '1'
    for (int i = 0; i < 256; i++) {
        write_wav_bit(&pcm, &pcm_len, &cap, 1);
    }

    WavHeader head = {
        {'R','I','F','F'}, pcm_len + 36, {'W','A','V','E'},
        {'f','m','t',' '}, 16, 1, 1, SAMPLE_RATE, SAMPLE_RATE, 1, 8,
        {'d','a','t','a'}, pcm_len
    };

    FILE *f = fopen(filename, "wb");
    if (f) {
        fwrite(&head, 1, sizeof(WavHeader), f);
        fwrite(pcm, 1, pcm_len, f);
        fclose(f);
    }
    free(pcm);
}

// Simple zero-crossing DSP to measure pulse width and extract bits
static uint8_t* decode_fsk_wav(const char *filename, size_t *out_len) {
    FILE *f = fopen(filename, "rb");
    if (!f) return NULL;

    WavHeader head;
    if (fread(&head, 1, sizeof(WavHeader), f) != sizeof(WavHeader)) {
        fclose(f); return NULL;
    }

    if (strncmp(head.riff_tag, "RIFF", 4) != 0 || head.bits_per_sample != 8) {
        fclose(f); return NULL;
    }

    uint8_t *pcm = (uint8_t*)malloc(head.data_length);
    if (!pcm) { fclose(f); return NULL; }
    size_t read_bytes = fread(pcm, 1, head.data_length, f);
    fclose(f);

    // Decoded data buffer
    uint32_t cap = 65536;
    uint8_t *data = (uint8_t*)malloc(cap);
    size_t data_len = 0;

    int current_bit = 0;
    int bits_read = 0;
    uint8_t current_byte = 0;
    int state = 0; // 0=seeking leader, 1=reading data
    int last_cross = 0;
    int last_val = pcm[0] > 128 ? 1 : 0;
    int ones_count = 0;

    for (size_t i = 1; i < read_bytes; i++) {
        int val = pcm[i] > 128 ? 1 : 0;
        if (val != last_val && val == 1) { // Positive zero-crossing
            int pulse_len = (int)i - last_cross;
            last_cross = (int)i;

            int bit = -1;
            // Expected lengths: 1000Hz = 44 samples, 2000Hz = 22 samples
            if (pulse_len > 33 && pulse_len < 66) bit = 1;
            else if (pulse_len > 11 && pulse_len <= 33) bit = 0;

            if (bit != -1) {
                if (state == 0) {
                    if (bit == 1) ones_count++;
                    else if (bit == 0 && ones_count > 100) {
                        state = 1; // Found sync bit
                        bits_read = 0;
                    } else {
                        ones_count = 0;
                    }
                } else {
                    // Reading byte: Start bit (0), 8 Data bits, Stop bit (1)
                    if (bits_read == 0) {
                        if (bit != 0) { // framing error
                            state = 0; ones_count = 0;
                        }
                    } else if (bits_read >= 1 && bits_read <= 8) {
                        current_byte |= (bit << (bits_read - 1));
                    } else if (bits_read == 9) {
                        if (bit == 1) {
                            if (data_len >= cap) {
                                cap *= 2;
                                data = (uint8_t*)realloc(data, cap);
                            }
                            data[data_len++] = current_byte;
                        } else {
                            state = 0; ones_count = 0; // framing error
                        }
                        current_byte = 0;
                        bits_read = -1;
                    }
                    bits_read++;
                }
            }
        }
        last_val = val;
    }

    free(pcm);
    *out_len = data_len;
    return data;
}

void cassette_save(const char *filename, RuntimeState *rt, int line_num) {
    if (strstr(filename, ".WAV") || strstr(filename, ".wav")) {
        // Serialize program to a temporary text file
        fileio_save(&rt->memory->program, "~tape_tmp.bas");
        
        FILE *tmp = fopen("~tape_tmp.bas", "rb");
        if (tmp) {
            fseek(tmp, 0, SEEK_END);
            size_t size = ftell(tmp);
            fseek(tmp, 0, SEEK_SET);
            
            uint8_t *buf = (uint8_t*)malloc(size);
            if (buf) {
                fread(buf, 1, size, tmp);
                encode_fsk_wav(filename, buf, size);
                free(buf);
            }
            fclose(tmp);
        }
        remove("~tape_tmp.bas");
    } else {
        // Normal save
        extern void pi_parse_save_cmd(Lexer *lex, RuntimeState *rt, int line_num);
        // Direct route to core fileio
        fileio_save(&rt->memory->program, filename);
    }
}

void cassette_load(const char *filename, RuntimeState *rt, int line_num) {
    if (strstr(filename, ".WAV") || strstr(filename, ".wav")) {
        size_t size = 0;
        uint8_t *buf = decode_fsk_wav(filename, &size);
        printf("DEBUG_HOW: File mod_cassette.c Line 227\n"); if (!buf) { error_raise(ERR_HOW, line_num); return; }
        
        FILE *tmp = fopen("~tape_tmp.bas", "wb");
        if (tmp) {
            fwrite(buf, 1, size, tmp);
            fclose(tmp);
            fileio_load(&rt->memory->program, "~tape_tmp.bas");
            remove("~tape_tmp.bas");
        }
        free(buf);
    } else {
        // Normal load
        fileio_load(&rt->memory->program, filename);
    }
}
