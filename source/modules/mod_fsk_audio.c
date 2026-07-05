#include "mod_fsk_audio.h"
#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define TokenType Win_TokenType
#include <windows.h>
#include <mmsystem.h>
#undef TokenType
#pragma comment(lib, "winmm.lib")
#else
#include <unistd.h>
#endif

// WAV FSK configuration
#define FSK_SAMPLE_RATE 44100
#define FSK_FREQ_1 1000 // 1000 Hz = 1.0ms period
#define FSK_FREQ_0 2000 // 2000 Hz = 0.5ms period
#define FSK_AMP 127

// External Memory API for program_insert
extern void program_clear(ProgramStore *store);
extern int program_insert(ProgramStore *store, double line_number, const char *full_text);

// Check if a file exists
static int file_exists(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (f) {
        fclose(f);
        return 1;
    }
    return 0;
}

void fsk_audio_init(void) {
}

void fsk_audio_motor(int state) {
    if (state == 0) {
#ifdef _WIN32
        mciSendStringA("stop TAPE", NULL, 0, NULL);
        mciSendStringA("close TAPE", NULL, 0, NULL);
#else
        system("killall aplay > /dev/null 2>&1");
        system("killall mpg123 > /dev/null 2>&1");
#endif
        return;
    }

    const char *file_to_play = NULL;
    if (file_exists("TAPE.WAV")) file_to_play = "TAPE.WAV";
    else if (file_exists("TAPE.MP3")) file_to_play = "TAPE.MP3";
    else if (file_exists("tape.wav")) file_to_play = "tape.wav";
    else if (file_exists("tape.mp3")) file_to_play = "tape.mp3";

    if (!file_to_play) return;

#ifdef _WIN32
    mciSendStringA("stop TAPE", NULL, 0, NULL);
    mciSendStringA("close TAPE", NULL, 0, NULL);
    char cmd[256];
    sprintf(cmd, "open \"%s\" type mpegvideo alias TAPE", file_to_play);
    mciSendStringA(cmd, NULL, 0, NULL);
    mciSendStringA("play TAPE", NULL, 0, NULL);
#else
    char cmd[256];
    if (strstr(file_to_play, ".MP3") || strstr(file_to_play, ".mp3")) {
        sprintf(cmd, "mpg123 \"%s\" > /dev/null 2>&1 &", file_to_play);
    } else {
        sprintf(cmd, "aplay \"%s\" > /dev/null 2>&1 &", file_to_play);
    }
    system(cmd);
#endif
}

// ---------------------------------------------------------
// TOKENIZER / DETOKENIZER
// ---------------------------------------------------------
static int fsk_tokenize_line(const char *source, uint8_t *out_buf, int max_len) {
    Lexer lex;
    lexer_init(&lex, source);
    int len = 0;
    
    while (lex.current.type != TOK_EOF && lex.current.type != TOK_CR) {
        if (len >= max_len - 10) break;
        
        if (lex.current.type == TOK_KEYWORD) {
            uint8_t kw = (uint8_t)(lex.current.value.keyword);
            if (kw < 128) {
                out_buf[len++] = 128 + kw;
            }
        } else if (lex.current.type == TOK_NUMBER) {
            char buf[64];
            sprintf(buf, "%ld", lex.current.value.num_value);
            for(int i=0; buf[i]; i++) out_buf[len++] = buf[i];
        } else if (lex.current.type == TOK_FLOAT_LIT) {
            char buf[64];
            sprintf(buf, "%g", lex.current.value.fval);
            for(int i=0; buf[i]; i++) out_buf[len++] = buf[i];
        } else if (lex.current.type == TOK_STRING) {
            out_buf[len++] = '"';
            for(int i=0; i<lex.current.str_length; i++) out_buf[len++] = lex.current.str_start[i];
            out_buf[len++] = '"';
        } else {
            if (lex.current.str_length > 0) {
                for(int i=0; i<lex.current.str_length; i++) {
                    out_buf[len++] = lex.current.str_start[i];
                }
            } else {
                switch(lex.current.type) {
                    case TOK_PLUS: out_buf[len++] = '+'; break;
                    case TOK_MINUS: out_buf[len++] = '-'; break;
                    case TOK_STAR: out_buf[len++] = '*'; break;
                    case TOK_SLASH: out_buf[len++] = '/'; break;
                    case TOK_EQUALS: out_buf[len++] = '='; break;
                    case TOK_LT: out_buf[len++] = '<'; break;
                    case TOK_GT: out_buf[len++] = '>'; break;
                    case TOK_LPAREN: out_buf[len++] = '('; break;
                    case TOK_RPAREN: out_buf[len++] = ')'; break;
                    case TOK_COMMA: out_buf[len++] = ','; break;
                    case TOK_SEMICOLON: out_buf[len++] = ';'; break;
                    case TOK_COLON: out_buf[len++] = ':'; break;
                    case TOK_HASH: out_buf[len++] = '#'; break;
                    case TOK_CARET: out_buf[len++] = '^'; break;
                    default: break; // unknown
                }
            }
        }
        
        out_buf[len++] = ' ';
        lexer_next(&lex);
    }
    
    if (len > 0 && out_buf[len-1] == ' ') len--;
    out_buf[len] = 0;
    return len + 1;
}

static void fsk_detokenize_line(const uint8_t *in_buf, int in_len, char *out_str) {
    int out_pos = 0;
    for (int i = 0; i < in_len && in_buf[i] != 0; i++) {
        uint8_t b = in_buf[i];
        if (b >= 128) {
            int kw = b - 128;
            const char *name = lexer_keyword_name(kw);
            if (name) {
                int nlen = (int)strlen(name);
                memcpy(&out_str[out_pos], name, nlen);
                out_pos += nlen;
            } else {
                out_str[out_pos++] = '?';
            }
        } else {
            out_str[out_pos++] = b;
        }
    }
    out_str[out_pos] = '\0';
}

// ---------------------------------------------------------
// FSK DSP ENCODER
// ---------------------------------------------------------

static void push_fsk_bit(uint8_t **pcm, int bit) {
    int freq = bit ? FSK_FREQ_1 : FSK_FREQ_0;
    int samples_per_cycle = FSK_SAMPLE_RATE / freq;
    int half = samples_per_cycle / 2;
    for (int i = 0; i < half; i++) *(*pcm)++ = 128 + FSK_AMP;
    for (int i = half; i < samples_per_cycle; i++) *(*pcm)++ = 128 - FSK_AMP;
}

static void push_fsk_byte(uint8_t **pcm, uint8_t b) {
    push_fsk_bit(pcm, 0); // start
    for (int i = 0; i < 8; i++) {
        push_fsk_bit(pcm, (b >> i) & 1);
    }
    push_fsk_bit(pcm, 1); // stop
}

int fsk_audio_csave(const char *filename, RuntimeState *rt) {
    char out_file[256];
    if (!filename || filename[0] == '\0') {
        strcpy(out_file, "TAPE.TAP");
    } else if (strchr(filename, '.') == NULL) {
        snprintf(out_file, sizeof(out_file), "%s.TAP", filename);
    } else {
        snprintf(out_file, sizeof(out_file), "%s", filename);
    }
    
    int is_wav = 0;
    char up_ext[16] = {0};
    const char *ext = strrchr(out_file, '.');
    if (ext) {
        strncpy(up_ext, ext, sizeof(up_ext)-1);
        for(int i=0; up_ext[i]; i++) up_ext[i] = toupper((unsigned char)up_ext[i]);
        if (strcmp(up_ext, ".WAV") == 0) is_wav = 1;
    }
    
    FILE *f = fopen(out_file, "wb");
    if (!f) return -1;
    
    if (is_wav) {
        // Allocate a large PCM buffer (10MB max for now)
        int max_pcm = 10 * 1024 * 1024;
        uint8_t *pcm_data = malloc(max_pcm);
        if (!pcm_data) { fclose(f); return -1; }
        
        uint8_t *pcm_ptr = pcm_data;
        
        // Write Sync Leader (256 zero bits)
        for (int i=0; i<256; i++) push_fsk_bit(&pcm_ptr, 0);
        // Write Sync Byte (0x16)
        push_fsk_byte(&pcm_ptr, 0x16);
        
        // Serialize program
        for (int i = 0; i < rt->program->count; i++) {
            uint8_t tok_buf[1024];
            int tok_len = fsk_tokenize_line(rt->program->lines[i].text, tok_buf, sizeof(tok_buf));
            int line_num = (int)rt->program->lines[i].line_number;
            
            push_fsk_byte(&pcm_ptr, line_num & 0xFF);
            push_fsk_byte(&pcm_ptr, (line_num >> 8) & 0xFF);
            push_fsk_byte(&pcm_ptr, tok_len & 0xFF);
            for (int j = 0; j < tok_len; j++) push_fsk_byte(&pcm_ptr, tok_buf[j]);
        }
        push_fsk_byte(&pcm_ptr, 0);
        push_fsk_byte(&pcm_ptr, 0);
        
        int pcm_len = (int)(pcm_ptr - pcm_data);
        
        // Write RIFF WAV Header
        uint32_t sample_rate = FSK_SAMPLE_RATE;
        uint32_t byte_rate = sample_rate;
        uint16_t block_align = 1;
        uint16_t bits_per_sample = 8;
        
        fwrite("RIFF", 1, 4, f);
        uint32_t chunk_size = 36 + pcm_len;
        fwrite(&chunk_size, 4, 1, f);
        fwrite("WAVEfmt ", 1, 8, f);
        uint32_t subchunk1_size = 16;
        fwrite(&subchunk1_size, 4, 1, f);
        uint16_t audio_format = 1;
        fwrite(&audio_format, 2, 1, f);
        uint16_t num_channels = 1;
        fwrite(&num_channels, 2, 1, f);
        fwrite(&sample_rate, 4, 1, f);
        fwrite(&byte_rate, 4, 1, f);
        fwrite(&block_align, 2, 1, f);
        fwrite(&bits_per_sample, 2, 1, f);
        fwrite("data", 1, 4, f);
        uint32_t subchunk2_size = pcm_len;
        fwrite(&subchunk2_size, 4, 1, f);
        
        fwrite(pcm_data, 1, pcm_len, f);
        free(pcm_data);
    } else {
        // Write IBM PC CAS format for untokenized (.TAP)
        // 1. Write 256 zero bytes (Leader)
        uint8_t leader[256];
        memset(leader, 0x00, 256);
        fwrite(leader, 1, 256, f);
        
        // 2. Write Sync Byte
        uint8_t sync = 0x16;
        fwrite(&sync, 1, 1, f);
        
        // 3. Payload and Checksum
        uint8_t checksum = 0;
        
        for (int i = 0; i < rt->program->count; i++) {
            char line_buf[1024];
            int len = snprintf(line_buf, sizeof(line_buf), "%s\r\n", 
                               rt->program->lines[i].text);
            
            for (int j = 0; j < len; j++) {
                uint8_t b = (uint8_t)line_buf[j];
                fwrite(&b, 1, 1, f);
                checksum = (checksum + b) & 0xFF;
            }
        }
        
        // 4. EOF marker (Ctrl-Z) and checksum
        uint8_t eof = 0x1A;
        fwrite(&eof, 1, 1, f);
        checksum = (checksum + eof) & 0xFF;
        
        // Write the two's complement of the checksum so the total sum is 0
        uint8_t final_checksum = (256 - checksum) & 0xFF;
        fwrite(&final_checksum, 1, 1, f);
    }
    
    fclose(f);
    return 0;
}

// ---------------------------------------------------------
// FSK DSP DECODER
// ---------------------------------------------------------
static void fsk_insert_program_line(RuntimeState *rt, int line_num, const uint8_t *tok_buf, int tok_len) {
    char ascii_buf[1024];
    fsk_detokenize_line(tok_buf, tok_len, ascii_buf);
    program_insert(rt->program, line_num, ascii_buf);
}

int fsk_audio_cload(const char *filename, RuntimeState *rt) {
    char in_file[256];
    if (!filename || filename[0] == '\0') {
        if (file_exists("TAPE.TAP")) strcpy(in_file, "TAPE.TAP");
        else strcpy(in_file, "TAPE.WAV");
    } else if (strchr(filename, '.') == NULL) {
        snprintf(in_file, sizeof(in_file), "%s.TAP", filename);
        if (!file_exists(in_file)) {
            snprintf(in_file, sizeof(in_file), "%s.WAV", filename);
        }
    } else {
        snprintf(in_file, sizeof(in_file), "%s", filename);
    }
    
    int is_wav = 0;
    char up_ext[16] = {0};
    const char *ext = strrchr(in_file, '.');
    if (ext) {
        strncpy(up_ext, ext, sizeof(up_ext)-1);
        for(int i=0; up_ext[i]; i++) up_ext[i] = toupper((unsigned char)up_ext[i]);
        if (strcmp(up_ext, ".WAV") == 0) is_wav = 1;
    }

    FILE *f = fopen(in_file, "rb");
    if (!f) return -1;
    
    program_clear(rt->program);
    rt->next_index = -1;
    rt->current_index = 0;
    
    if (!is_wav) {
        // Read IBM PC CAS format for untokenized (.TAP)
        
        // 1. Skip 256 byte leader (or just scan until we see 0x16)
        int b;
        while ((b = fgetc(f)) != EOF) {
            if (b == 0x16) break;
        }
        
        if (b != 0x16) {
            fclose(f);
            return -1;
        }
        
        // 2. Read Payload and verify checksum
        uint8_t checksum = 0;
        char line_buf[1024];
        int line_len = 0;
        
        while ((b = fgetc(f)) != EOF) {
            checksum = (checksum + b) & 0xFF;
            
            if (b == 0x1A) {
                // EOF marker found. The next byte is the final checksum byte.
                int final_c = fgetc(f);
                if (final_c != EOF) {
                    checksum = (checksum + final_c) & 0xFF;
                }
                break;
            }
            
            if (b == '\r') continue;
            
            if (b == '\n') {
                line_buf[line_len] = '\0';
                
                // Parse line number
                int line_num = 0;
                char *ptr = line_buf;
                while (*ptr == ' ') ptr++;
                while (*ptr >= '0' && *ptr <= '9') {
                    line_num = line_num * 10 + (*ptr - '0');
                    ptr++;
                }
                while (*ptr == ' ') ptr++; // skip space after line num
                
                if (line_num > 0) {
                    program_insert(rt->program, line_num, line_buf);
                }
                line_len = 0;
                continue;
            }
            
            if ((size_t)line_len < sizeof(line_buf) - 1) {
                line_buf[line_len++] = (char)b;
            }
        }
        
        fclose(f);
        return (checksum == 0) ? 0 : -1;
    }
    
    // Read WAV FSK
    uint8_t header[44];
    if (fread(header, 1, 44, f) != 44) { fclose(f); return -1; }
    
    fseek(f, 0, SEEK_END);
    int file_len = ftell(f);
    fseek(f, 44, SEEK_SET);
    int pcm_len = file_len - 44;
    
    uint8_t *pcm_data = malloc(pcm_len);
    if (!pcm_data || fread(pcm_data, 1, pcm_len, f) != (size_t)pcm_len) {
        if(pcm_data) free(pcm_data);
        fclose(f);
        return -1;
    }
    fclose(f);
    
    // Very simple byte extractor from FSK (assumes perfect timing)
    int pcm_idx = 0;
    
    // Helper macro to read one bit
    #define READ_BIT(out_bit) do { \
        int samples = 0; \
        int start_val = pcm_data[pcm_idx] > 128 ? 1 : 0; \
        int cross_count = 0; \
        while (pcm_idx < pcm_len) { \
            int val = pcm_data[pcm_idx] > 128 ? 1 : 0; \
            if (val != start_val) { \
                cross_count++; \
                start_val = val; \
                if (cross_count == 2) break; \
            } \
            samples++; \
            pcm_idx++; \
        } \
        out_bit = (samples > 30) ? 1 : 0; \
    } while(0)

    // Find Sync Byte 0x16 using a sliding window
    int found_sync = 0;
    int b = 0;
    while (pcm_idx < pcm_len) {
        int bit = 0;
        READ_BIT(bit);
        
        b = ((b >> 1) | (bit << 7)) & 0xFF;
        
        if (b == 0x16) { 
            found_sync = 1; 
            int dummy;
    (void)dummy;
            READ_BIT(dummy); // Consume stop bit of the sync byte
            break; 
        }
    }
    
    if (found_sync) {
        while (pcm_idx < pcm_len) {
            
            int b_val;
            // Start bit
            READ_BIT(b_val);
            
            int line_num = 0;
            // read low
            int low = 0; for (int i=0; i<8; i++) { READ_BIT(b_val); low |= (b_val << i); }
            READ_BIT(b_val); // stop
            
            READ_BIT(b_val); // start
            int high = 0; for (int i=0; i<8; i++) { READ_BIT(b_val); high |= (b_val << i); }
            READ_BIT(b_val); // stop
            
            line_num = low | (high << 8);
            if (line_num == 0) break;
            
            READ_BIT(b_val); // start
            int tok_len = 0; for (int i=0; i<8; i++) { READ_BIT(b_val); tok_len |= (b_val << i); }
            READ_BIT(b_val); // stop
            
            uint8_t tok_buf[256];
            for (int j = 0; j < tok_len; j++) {
                READ_BIT(b_val); // start
                int tok_val = 0; for (int i=0; i<8; i++) { READ_BIT(b_val); tok_val |= (b_val << i); }
                READ_BIT(b_val); // stop
                tok_buf[j] = tok_val;
            }
            
            fsk_insert_program_line(rt, line_num, tok_buf, tok_len);
        }
    }
    
    if (pcm_data) free(pcm_data);
    return 0;
}
