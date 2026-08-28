// FILENAME: hal_sdl2.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (hal_hosted.c)
// NEEDS: libcore (alloc.h, alloc.c, hal.h, hal_sdl2.h, math.h)
// NEEDS: libcore (memops.h, memops.c)
// NEEDS: libengine (math.c)
// NEEDS: libplatform (platform.h)
// SDL2 driver implementations for HAL video presentation and audio output.
//
// ---- Includes ----

#include "hal/hal_sdl2.h"
#include "hal/hal.h"
#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"
#include "runtime/math/math.h"
#include "platform/platform.h"

#if defined(BASIC_SDL_BUILD) && !defined(NO_SDL2)
#include <SDL.h>

static SDL_Window   *s_sdl_window = NULL;
static SDL_Renderer *s_sdl_renderer = NULL;
static SDL_Texture  *s_sdl_texture = NULL;
static int           s_current_width = 0;
static int           s_current_height = 0;
static bool          s_video_initialized = false;
static bool          s_window_open = false;

static SDL_AudioDeviceID s_audio_dev = 0;
static bool              s_audio_initialized = false;
static uint8_t           s_audio_volume = 80;

#define AUDIO_SAMPLE_RATE 44100

// ======================================================================
// SDL2 Video Implementation
// ======================================================================

bool sdl2_video_init(int width, int height, int bpp, const char *title) {
    (void)bpp;
    if (width <= 0 || height <= 0) return false;

    if (!s_video_initialized) {
        if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
            return false;
        }
        s_video_initialized = true;
    }

    if (s_sdl_texture) {
        SDL_DestroyTexture(s_sdl_texture);
        s_sdl_texture = NULL;
    }
    if (s_sdl_renderer) {
        SDL_DestroyRenderer(s_sdl_renderer);
        s_sdl_renderer = NULL;
    }
    if (s_sdl_window) {
        SDL_DestroyWindow(s_sdl_window);
        s_sdl_window = NULL;
    }

    const char *win_title = title ? title : "BASIC++ Graphics Display";
    s_sdl_window = SDL_CreateWindow(win_title,
                                    SDL_WINDOWPOS_CENTERED,
                                    SDL_WINDOWPOS_CENTERED,
                                    width, height,
                                    SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!s_sdl_window) {
        return false;
    }

    s_sdl_renderer = SDL_CreateRenderer(s_sdl_window, -1,
                                        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!s_sdl_renderer) {
        s_sdl_renderer = SDL_CreateRenderer(s_sdl_window, -1, SDL_RENDERER_SOFTWARE);
    }
    if (!s_sdl_renderer) {
        SDL_DestroyWindow(s_sdl_window);
        s_sdl_window = NULL;
        return false;
    }

    s_sdl_texture = SDL_CreateTexture(s_sdl_renderer,
                                      SDL_PIXELFORMAT_ARGB8888,
                                      SDL_TEXTUREACCESS_STREAMING,
                                      width, height);
    if (!s_sdl_texture) {
        SDL_DestroyRenderer(s_sdl_renderer);
        s_sdl_renderer = NULL;
        SDL_DestroyWindow(s_sdl_window);
        s_sdl_window = NULL;
        return false;
    }

    s_current_width = width;
    s_current_height = height;
    s_window_open = true;
    return true;
}

void sdl2_video_shutdown(void) {
    if (s_sdl_texture) {
        SDL_DestroyTexture(s_sdl_texture);
        s_sdl_texture = NULL;
    }
    if (s_sdl_renderer) {
        SDL_DestroyRenderer(s_sdl_renderer);
        s_sdl_renderer = NULL;
    }
    if (s_sdl_window) {
        SDL_DestroyWindow(s_sdl_window);
        s_sdl_window = NULL;
    }
    if (s_video_initialized) {
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        s_video_initialized = false;
    }
    s_window_open = false;
    s_current_width = 0;
    s_current_height = 0;
}

void sdl2_video_present(const void *pixels, int width, int height, int pitch) {
    if (!s_window_open || !s_sdl_renderer || !s_sdl_texture || !pixels) return;

    if (width != s_current_width || height != s_current_height) {
        // Recreate texture on size change
        SDL_DestroyTexture(s_sdl_texture);
        s_sdl_texture = SDL_CreateTexture(s_sdl_renderer,
                                          SDL_PIXELFORMAT_ARGB8888,
                                          SDL_TEXTUREACCESS_STREAMING,
                                          width, height);
        if (!s_sdl_texture) return;
        s_current_width = width;
        s_current_height = height;
    }

    int src_pitch = (pitch > 0) ? pitch : (width * (int)sizeof(uint32_t));
    SDL_UpdateTexture(s_sdl_texture, NULL, pixels, src_pitch);
    SDL_RenderClear(s_sdl_renderer);
    SDL_RenderCopy(s_sdl_renderer, s_sdl_texture, NULL, NULL);
    SDL_RenderPresent(s_sdl_renderer);
}

void sdl2_video_set_palette(const uint32_t *palette, int count) {
    (void)palette;
    (void)count;
}

void sdl2_video_poll_events(void) {
    if (!s_video_initialized) return;

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            s_window_open = false;
        }
    }
}

bool sdl2_video_is_window_open(void) {
    return s_window_open && (s_sdl_window != NULL);
}

// ======================================================================
// SDL2 Audio Implementation
// ======================================================================

bool sdl2_audio_init(void) {
    if (s_audio_initialized) return true;

    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        return false;
    }

    SDL_AudioSpec wanted, have;
    runtime_memset(&wanted, 0, sizeof(wanted));
    wanted.freq = AUDIO_SAMPLE_RATE;
    wanted.format = AUDIO_S16SYS;
    wanted.channels = 1;
    wanted.samples = 2048;

    s_audio_dev = SDL_OpenAudioDevice(NULL, 0, &wanted, &have, 0);
    if (s_audio_dev == 0) {
        return false;
    }

    SDL_PauseAudioDevice(s_audio_dev, 0); // Unpause
    s_audio_initialized = true;
    return true;
}

void sdl2_audio_shutdown(void) {
    if (s_audio_initialized) {
        if (s_audio_dev != 0) {
            SDL_CloseAudioDevice(s_audio_dev);
            s_audio_dev = 0;
        }
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        s_audio_initialized = false;
    }
}

void sdl2_audio_beep(void) {
    sdl2_audio_tone(880, 150); // 880 Hz standard beep
}

void sdl2_audio_tone(uint32_t frequency_hz, uint32_t duration_ms) {
    if (!s_audio_initialized) {
        if (!sdl2_audio_init()) return;
    }

    if (frequency_hz == 0 || duration_ms == 0) {
        platform_sleep_ms(duration_ms);
        return;
    }

    size_t total_samples = (size_t)((double)AUDIO_SAMPLE_RATE * ((double)duration_ms / 1000.0));
    if (total_samples == 0) return;

    int16_t *buf = (int16_t *)runtime_malloc(total_samples * sizeof(int16_t));
    if (!buf) return;

    double phase = 0.0;
    double phase_inc = 2.0 * 3.14159265358979323846 * (double)frequency_hz / (double)AUDIO_SAMPLE_RATE;
    double amplitude = 32767.0 * ((double)s_audio_volume / 100.0);

    for (size_t i = 0; i < total_samples; ++i) {
        buf[i] = (int16_t)(runtime_sin(phase) * amplitude);
        phase += phase_inc;
        if (phase >= 2.0 * 3.14159265358979323846) {
            phase -= 2.0 * 3.14159265358979323846;
        }
    }

    SDL_QueueAudio(s_audio_dev, buf, (Uint32)(total_samples * sizeof(int16_t)));
    runtime_free(buf);
}

void sdl2_audio_stop(void) {
    if (s_audio_dev != 0) {
        SDL_ClearQueuedAudio(s_audio_dev);
    }
}

void sdl2_audio_set_volume(uint8_t volume) {
    s_audio_volume = (volume > 100) ? 100 : volume;
}

#else // !defined(BASIC_SDL_BUILD) || defined(NO_SDL2)

bool sdl2_video_init(int width, int height, int bpp, const char *title) { (void)width; (void)height; (void)bpp; (void)title; return false; }
void sdl2_video_shutdown(void) {}
void sdl2_video_present(const void *pixels, int width, int height, int pitch) { (void)pixels; (void)width; (void)height; (void)pitch; }
void sdl2_video_set_palette(const uint32_t *palette, int count) { (void)palette; (void)count; }
void sdl2_video_poll_events(void) {}
bool sdl2_video_is_window_open(void) { return false; }

bool sdl2_audio_init(void) { return false; }
void sdl2_audio_shutdown(void) {}
void sdl2_audio_beep(void) { platform_sound_beep(); }
void sdl2_audio_tone(uint32_t frequency_hz, uint32_t duration_ms) { platform_sound_tone(frequency_hz, duration_ms); }
void sdl2_audio_stop(void) { platform_sound_stop(); }
void sdl2_audio_set_volume(uint8_t volume) { (void)volume; }

#endif // BASIC_SDL_BUILD

void hal_init_sdl2(void) {
    HalContext *hal = hal_get();
    if (!hal) return;

    hal->video.init = sdl2_video_init;
    hal->video.shutdown = sdl2_video_shutdown;
    hal->video.present_framebuffer = sdl2_video_present;
    hal->video.set_palette = sdl2_video_set_palette;
    hal->video.poll_events = sdl2_video_poll_events;
    hal->video.is_window_open = sdl2_video_is_window_open;

    hal->audio.init = sdl2_audio_init;
    hal->audio.shutdown = sdl2_audio_shutdown;
    hal->audio.beep = sdl2_audio_beep;
    hal->audio.tone = sdl2_audio_tone;
    hal->audio.stop = sdl2_audio_stop;
    hal->audio.set_volume = sdl2_audio_set_volume;
}
