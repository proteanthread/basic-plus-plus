/**
 * BASIC++
 * Subsystem: Decoupled Audio Player
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mod_audio_player.h"

#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
#endif

void audio_player_play(const char *filename) {
#ifdef _WIN32
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "open \"%s\" type mpegvideo alias bpp_audio", filename);
    mciSendStringA(cmd, NULL, 0, NULL);
    mciSendStringA("play bpp_audio", NULL, 0, NULL);
#else
    char cmd[512];
    if (strstr(filename, ".mp3") || strstr(filename, ".MP3")) {
        snprintf(cmd, sizeof(cmd), "mpg123 -q \"%s\" &", filename);
    } else {
        snprintf(cmd, sizeof(cmd), "aplay -q \"%s\" &", filename);
    }
    system(cmd);
#endif
}

void audio_player_stop(void) {
#ifdef _WIN32
    mciSendStringA("stop bpp_audio", NULL, 0, NULL);
    mciSendStringA("close bpp_audio", NULL, 0, NULL);
#else
    system("pkill mpg123");
    system("pkill aplay");
#endif
}

void audio_player_motor_state(int state) {
    if (state == 0) {
        audio_player_stop();
    } else {
        FILE *fp = fopen("TAPE.MP3", "rb");
        if (fp) {
            fclose(fp);
            audio_player_play("TAPE.MP3");
            return;
        }
        fp = fopen("TAPE.WAV", "rb");
        if (fp) {
            fclose(fp);
            audio_player_play("TAPE.WAV");
            return;
        }
    }
}
