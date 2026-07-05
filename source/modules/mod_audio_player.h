/**
 * BASIC++
 * Subsystem: Decoupled Audio Player (MOTOR command playback)
 */
#ifndef MOD_AUDIO_PLAYER_H
#define MOD_AUDIO_PLAYER_H

void audio_player_play(const char *filename);
void audio_player_stop(void);
void audio_player_motor_state(int state);

#endif
