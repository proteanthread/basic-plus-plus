/**
 * Original GW-BASIC Interpreter Port (C17)
 * 
 * -----------------------------------------------------------------------------
 * MAINTENANCE & EXTENSION GUIDELINES
 * -----------------------------------------------------------------------------
 * 1. WHAT CAN BE CHANGED:
 *    - Console sizing, output formatting, or ANSI color/escape sequences.
 *    - Logic inside statement handlers to optimize standard BASIC behaviors.
 *    - Math functions (tuning logic for trigonometric or random values).
 * 
 * 2. WHAT CANNOT BE CHANGED:
 *    - Keyword/token byte mapping tables (essential for loading tokenized BAS binaries).
 *    - Segmented memory layout simulation structures.
 *    - Core mathematical parsing precedence chain (eval descent hierarchy).
 * 
 * 3. EXPECTED BEHAVIOR:
 *    - SDL2 graphics screen emulation, pixel plotting, video memory mapping, and keyboard polling.
 *    - CGA/EGA/VGA framebuffers, palette color conversions, and audio synth engines.
 * 
 * 4. WHAT TO DO IF SOMETHING BREAKS:
 *    - Check variable tables, default variable type states, and stack pointers.
 *    - Cross-reference token layouts with original detokenization specifications.
 *    - Run diagnostic verification script to identify isolated error line numbers.
 * -----------------------------------------------------------------------------
 */
#include "sdl2_emu.h"

#ifndef NO_SDL2
#include "../boot.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <signal.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define TokenType WinTokenType
#include <windows.h>
#undef TokenType
#endif

#ifndef NO_SDL2
#include <SDL.h>
#endif
#include "../mod_legacy_compat.h"
#include "../platform.h"

static const uint32_t gw_base_palette_NTSC[16] = {
    0x000000FF, // 0: Black
    0x0000AAFF, // 1: Blue
    0x00AA00FF, // 2: Green
    0x00AAAAFF, // 3: Cyan
    0xAA0000FF, // 4: Red
    0xAA00AAFF, // 5: Magenta
    0xAA5500FF, // 6: Brown (Dark Yellow)
    0xAAAAAAFF, // 7: Light Gray
    0x555555FF, // 8: Dark Gray
    0x5555FFFF, // 9: Light Blue
    0x55FF55FF, // 10: Light Green
    0x55FFFFFF, // 11: Light Cyan
    0xFF5555FF, // 12: Light Red
    0xFF55FFFF, // 13: Light Magenta
    0xFFFF55FF, // 14: Yellow
    0xFFFFFFFF  // 15: White
};

uint32_t GW_PALETTE[256];


static const uint32_t *gw_get_base_palette(void) {
    return gw_base_palette_NTSC;
}

static int g_is_atari_graphics = 0;
static int g_atari_graphics_mode = 0;

static int g_current_mode = 0;
static int g_cga_bg_color = 0;
static int g_cga_palette = 1;

static int g_sdl_active = 0;
static uint32_t *g_pixels = NULL;
static int g_tex_width = 640;
static int g_tex_height = 400;
static int g_grid_cols = 80;
static int g_grid_rows = 25;
static int g_cursor_x = 0;
static int g_cursor_y = 0;
static uint32_t g_text_fg = 0xFFFFFFFF; // White
static uint32_t g_text_bg = 0x000000FF; // Black
static int g_machine_type = 0; // 0=VGA, 1=HGC, 2=Tandy, 3=PCjr, 4=Plantronics, 5=AT&T, 6=Amstrad, 7=PC98
static int g_mouse_x = 0;
static int g_mouse_y = 0;
static int g_mouse_down = 0;
static int g_pen_was_down = 0;
static int g_pen_last_x = 0;
static int g_pen_last_y = 0;
#define MAX_GRID_ROWS 40
#define MAX_GRID_COLS 100
static char g_screen_chars[MAX_GRID_ROWS][MAX_GRID_COLS];

static int g_scroll_start = 0;
static int g_scroll_lines = 25;
static int g_show_fn_keys = 0;
static int g_mono_mode = 0;

#ifndef NO_SDL2
// SDL Window / Renderer Globals
static SDL_Window *g_window = NULL;
static SDL_Renderer *g_renderer = NULL;
static SDL_Texture *g_texture = NULL;

// Keyboard buffer
static int g_key_buffer[64];
static int g_key_head = 0;
static int g_key_tail = 0;

// Audio synthesis state
static SDL_AudioDeviceID g_audio_device = 0;
static double g_audio_phase = 0.0;
static float g_audio_frequency = 0.0f;
static SDL_mutex *g_audio_mutex = NULL;

// Music thread state
static SDL_Thread *g_music_thread = NULL;
static SDL_mutex *g_music_mutex = NULL;
static int g_music_stop_requested = 0;
static char *g_music_mml = NULL;

static void audio_callback(void *userdata, Uint8 *stream, int len) {
    (void)userdata;
    int16_t *buffer = (int16_t *)stream;
    int samples = len / 2;
    
    SDL_LockMutex(g_audio_mutex);
    float freq = g_audio_frequency;
    SDL_UnlockMutex(g_audio_mutex);
    
    if (freq <= 0.0f) {
        memset(stream, 0, len);
        return;
    }
    
    double sample_rate = 44100.0;
    double phase_step = freq / sample_rate;
    
    for (int i = 0; i < samples; i++) {
        g_audio_phase += phase_step;
        if (g_audio_phase >= 1.0) {
            g_audio_phase -= 1.0;
        }
        buffer[i] = (g_audio_phase < 0.5) ? 3000 : -3000;
    }
}
#endif

int gw_sdl2_init(int width, int height, const char *title, int fullscreen) {
    if (g_pixels != NULL) {
        return 0; // Already initialized
    }
    static int atexit_registered = 0;
    if (!atexit_registered) {
        atexit(gw_sdl2_cleanup);
        atexit_registered = 1;
    }
    g_tex_width = width;
    g_tex_height = height;
    g_pixels = (uint32_t *)calloc(width * height, sizeof(uint32_t));
    for (int i = 0; i < 16; i++) {
        GW_PALETTE[i] = gw_get_base_palette()[i];
    }

#ifndef NO_SDL2
    boot_log(BOOT_DEBUG, "SDL2: Initializing subsystem video and audio...");
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        boot_log(BOOT_LOG, "SDL2 ERROR: SDL_Init failed: %s", SDL_GetError());
        free(g_pixels);
        g_pixels = NULL;
        return -1;
    }
    
    boot_log(BOOT_DEBUG, "SDL2: Video driver initialized: %s", SDL_GetCurrentVideoDriver());
    
    Uint32 flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
    if (fullscreen) {
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        boot_log(BOOT_DEBUG, "SDL2: Fullscreen requested");
    }
    
    boot_log(BOOT_DEBUG, "SDL2: Creating window '%s' (%dx%d)...", title, width, height);
    g_window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                                width, height, flags);
    if (!g_window) {
        boot_log(BOOT_LOG, "SDL2 ERROR: SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        free(g_pixels);
        g_pixels = NULL;
        return -1;
    }
    
    const char *driver = SDL_GetCurrentVideoDriver();
    if (driver && strcmp(driver, "dummy") == 0) {
        boot_log(BOOT_DEBUG, "SDL2: Dummy video driver detected, forcing software renderer");
        g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_SOFTWARE);
    } else {
        boot_log(BOOT_DEBUG, "SDL2: Creating accelerated renderer...");
        g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (!g_renderer) {
            boot_log(BOOT_DEBUG, "SDL2: Accelerated renderer failed, attempting software fallback...");
            g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_SOFTWARE);
        }
    }
    if (!g_renderer) {
        boot_log(BOOT_LOG, "SDL2 ERROR: SDL_CreateRenderer failed: %s", SDL_GetError());
        SDL_DestroyWindow(g_window);
        g_window = NULL;
        SDL_Quit();
        free(g_pixels);
        g_pixels = NULL;
        return -1;
    }
    
    boot_log(BOOT_DEBUG, "SDL2: Creating streaming texture...");
    g_texture = SDL_CreateTexture(g_renderer, SDL_PIXELFORMAT_RGBA8888, 
                                  SDL_TEXTUREACCESS_STREAMING, width, height);
    if (!g_texture) {
        boot_log(BOOT_LOG, "SDL2 ERROR: SDL_CreateTexture failed: %s", SDL_GetError());
        SDL_DestroyRenderer(g_renderer);
        g_renderer = NULL;
        SDL_DestroyWindow(g_window);
        g_window = NULL;
        SDL_Quit();
        free(g_pixels);
        g_pixels = NULL;
        return -1;
    }
    
    SDL_StartTextInput();
    
    // Setup Audio
    g_audio_mutex = SDL_CreateMutex();
    g_music_mutex = SDL_CreateMutex();
    
    SDL_AudioSpec wanted, obtained;
    SDL_zero(wanted);
    wanted.freq = 44100;
    wanted.format = AUDIO_S16SYS;
    wanted.channels = 1;
    wanted.samples = 512;
    wanted.callback = audio_callback;
    
    g_audio_device = SDL_OpenAudioDevice(NULL, 0, &wanted, &obtained, 0);
    if (g_audio_device > 0) {
        SDL_PauseAudioDevice(g_audio_device, 0);
        boot_log(BOOT_DEBUG, "SDL2: Audio obtained: freq=%d, format=0x%X, channels=%d, samples=%d",
                 obtained.freq, obtained.format, obtained.channels, obtained.samples);
    } else {
        boot_log(BOOT_DEBUG, "SDL2 WARNING: SDL_OpenAudioDevice failed: %s", SDL_GetError());
    }
    
    // Log diagnostic information
    {
        SDL_version compiled;
        SDL_version linked;
        SDL_VERSION(&compiled);
        SDL_GetVersion(&linked);
        boot_log(BOOT_DEBUG, "SDL2: Version compiled: %d.%d.%d, linked: %d.%d.%d",
                 compiled.major, compiled.minor, compiled.patch,
                 linked.major, linked.minor, linked.patch);

        SDL_RendererInfo info;
        if (SDL_GetRendererInfo(g_renderer, &info) == 0) {
            boot_log(BOOT_DEBUG, "SDL2: Renderer: %s (flags: 0x%X, max_w: %d, max_h: %d)",
                     info.name, info.flags, info.max_texture_width, info.max_texture_height);
        }

        int display_index = SDL_GetWindowDisplayIndex(g_window);
        SDL_DisplayMode mode;
        if (SDL_GetCurrentDisplayMode(display_index, &mode) == 0) {
            boot_log(BOOT_DEBUG, "SDL2: Display %d mode: %dx%d@%dHz (format: 0x%X)",
                     display_index, mode.w, mode.h, mode.refresh_rate, mode.format);
        }
    }
    
    g_sdl_active = 1;
    gw_sdl2_clear_screen(g_text_bg);
    return 0;
#else
    (void)title;
    (void)fullscreen;
    g_sdl_active = 1;
    return 0; // Success stub
#endif
}

int gw_sdl2_is_active(void) {
    return g_sdl_active;
}

void *gw_sdl2_get_window_ptr(void) {
#ifndef NO_SDL2
    return (void *)g_window;
#else
    return NULL;
#endif
}

void *gw_sdl2_get_renderer_ptr(void) {
#ifndef NO_SDL2
    return (void *)g_renderer;
#else
    return NULL;
#endif
}

uint32_t gw_sdl2_get_audio_device_id(void) {
#ifndef NO_SDL2
    return (uint32_t)g_audio_device;
#else
    return 0;
#endif
}

void gw_sdl2_cleanup(void) {
    if (!g_sdl_active) return;
    g_sdl_active = 0;
    if (!g_pixels) return;
    
#ifndef NO_SDL2
    const char *driver = SDL_GetCurrentVideoDriver();
    if (driver && strcmp(driver, "dummy") == 0) {
        if (g_pixels) {
            free(g_pixels);
            g_pixels = NULL;
        }
        return;
    }
#endif
    
    boot_log(BOOT_DEBUG, "SDL2: Cleaning up graphics and audio resources...");
#ifndef NO_SDL2
    SDL_StopTextInput();
    gw_sdl2_stop_music();
    
    if (g_audio_device > 0) {
        boot_log(BOOT_DEBUG, "SDL2: Closing audio device %d", g_audio_device);
        SDL_CloseAudioDevice(g_audio_device);
        g_audio_device = 0;
    }
    
    if (g_audio_mutex) {
        SDL_DestroyMutex(g_audio_mutex);
        g_audio_mutex = NULL;
    }
    if (g_music_mutex) {
        SDL_DestroyMutex(g_music_mutex);
        g_music_mutex = NULL;
    }
    
    if (g_texture) {
        boot_log(BOOT_DEBUG, "SDL2: Destroying streaming texture");
        SDL_DestroyTexture(g_texture);
        g_texture = NULL;
    }
    if (g_renderer) {
        boot_log(BOOT_DEBUG, "SDL2: Destroying renderer");
        SDL_DestroyRenderer(g_renderer);
        g_renderer = NULL;
    }
    if (g_window) {
        boot_log(BOOT_DEBUG, "SDL2: Destroying window");
        SDL_DestroyWindow(g_window);
        g_window = NULL;
    }
    SDL_Quit();
#endif

    if (g_pixels) {
        free(g_pixels);
        g_pixels = NULL;
    }
}

void gw_sdl2_present_force(void) {
#ifndef NO_SDL2
    extern int g_screen_lock;
    if (g_screen_lock) return;
    if (!g_renderer || !g_texture || !g_pixels) return;
    const char *driver = SDL_GetCurrentVideoDriver();
    if (driver && strcmp(driver, "dummy") == 0) {
        return;
    }
    SDL_UpdateTexture(g_texture, NULL, g_pixels, g_tex_width * sizeof(uint32_t));
    SDL_RenderClear(g_renderer);
    SDL_RenderCopy(g_renderer, g_texture, NULL, NULL);
    SDL_RenderPresent(g_renderer);
#endif
}

void gw_sdl2_present(void) {
#ifndef NO_SDL2
    static uint32_t last_present_time = 0;
    uint32_t now = SDL_GetTicks();
    if (now - last_present_time >= 16) {
        gw_sdl2_present_force();
        last_present_time = now;
    }
#endif
}

void gw_sdl2_poll_events(void) {
#ifndef NO_SDL2
    if (!g_pixels) return;
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            gw_sdl2_cleanup();
            raise(SIGINT);
            return;
        } else if (event.type == SDL_WINDOWEVENT) {
            if (event.window.event == SDL_WINDOWEVENT_EXPOSED ||
                event.window.event == SDL_WINDOWEVENT_RESIZED ||
                event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                gw_sdl2_present();
            }
        } else if (event.type == SDL_TEXTINPUT) {
            for (int i = 0; event.text.text[i] != '\0'; i++) {
                char c = event.text.text[i];
                if (c >= 32 && c <= 126) {
                    int next = (g_key_tail + 1) % 64;
                    if (next != g_key_head) {
                        g_key_buffer[g_key_tail] = c;
                        g_key_tail = next;
                    }
                }
            }
        } else if (event.type == SDL_MOUSEMOTION) {
            g_mouse_x = event.motion.x;
            g_mouse_y = event.motion.y;
        } else if (event.type == SDL_MOUSEBUTTONDOWN) {
            if (event.button.button == SDL_BUTTON_LEFT) {
                g_mouse_down = 1;
                g_pen_was_down = 1;
                g_pen_last_x = event.button.x;
                g_pen_last_y = event.button.y;
            }
        } else if (event.type == SDL_MOUSEBUTTONUP) {
            if (event.button.button == SDL_BUTTON_LEFT) {
                g_mouse_down = 0;
            }
        } else if (event.type == SDL_KEYDOWN) {
            SDL_Keycode sym = event.key.keysym.sym;
            int code = 0;
            if (sym == SDLK_RETURN || sym == SDLK_KP_ENTER) {
                code = 13;
            } else if (sym == SDLK_BACKSPACE) {
                code = 8;
            } else if (sym == SDLK_ESCAPE) {
                code = 27;
            } else if (sym == SDLK_TAB) {
                code = 9;
            }
            
            if (code > 0) {
                int next = (g_key_tail + 1) % 64;
                if (next != g_key_head) {
                    g_key_buffer[g_key_tail] = code;
                    g_key_tail = next;
                }
            }
        }
    }
#endif
}

int gw_sdl2_get_key(void) {
#ifndef NO_SDL2
    if (g_key_head == g_key_tail) return 0;
    int code = g_key_buffer[g_key_head];
    g_key_head = (g_key_head + 1) % 64;
    return code;
#else
    return 0;
#endif
}

int gw_sdl2_key_pressed(int scancode) {
#ifndef NO_SDL2
    if (!g_pixels) return 0;
    const Uint8 *state = SDL_GetKeyboardState(NULL);
    if (scancode >= 0 && scancode < SDL_NUM_SCANCODES) {
        return state[scancode] ? 1 : 0;
    }
#else
    (void)scancode;
#endif
    return 0;
}

void gw_sdl2_get_pen(int *x, int *y, int *down, int *was_down, int *last_x, int *last_y) {
#ifndef NO_SDL2
    if (x) *x = g_mouse_x;
    if (y) *y = g_mouse_y;
    if (down) *down = g_mouse_down ? -1 : 0;
    if (was_down) {
        *was_down = g_pen_was_down ? -1 : 0;
        g_pen_was_down = 0; // Reset after read like GW-BASIC
    }
    if (last_x) *last_x = g_pen_last_x;
    if (last_y) *last_y = g_pen_last_y;
#else
    if (x) *x = 0;
    if (y) *y = 0;
    if (down) *down = 0;
    if (was_down) *was_down = 0;
    if (last_x) *last_x = 0;
    if (last_y) *last_y = 0;
#endif
}

void gw_sdl2_clear(uint32_t color) {
    if (!g_pixels) return;
    int size = g_tex_width * g_tex_height;
    for (int i = 0; i < size; i++) {
        g_pixels[i] = color;
    }
}

void gw_sdl2_set_pixel(int x, int y, uint32_t color) {
    if (!g_pixels || x < 0 || x >= g_tex_width || y < 0 || y >= g_tex_height) return;
    g_pixels[y * g_tex_width + x] = color;
}

uint32_t gw_sdl2_get_pixel(int x, int y) {
    if (!g_pixels || x < 0 || x >= g_tex_width || y < 0 || y >= g_tex_height) return 0;
    return g_pixels[y * g_tex_width + x];
}

void gw_sdl2_draw_line(int x1, int y1, int x2, int y2, uint32_t color) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    
    while (1) {
        gw_sdl2_set_pixel(x1, y1, color);
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

void gw_sdl2_draw_circle(int cx, int cy, int r, uint32_t color, int fill) {
    if (fill) {
        for (int dy = -r; dy <= r; dy++) {
            for (int dx = -r; dx <= r; dx++) {
                if (dx*dx + dy*dy <= r*r) {
                    gw_sdl2_set_pixel(cx + dx, cy + dy, color);
                }
            }
        }
    } else {
        int x = r;
        int y = 0;
        int err = 0;
        
        while (x >= y) {
            gw_sdl2_set_pixel(cx + x, cy + y, color);
            gw_sdl2_set_pixel(cx + y, cy + x, color);
            gw_sdl2_set_pixel(cx - y, cy + x, color);
            gw_sdl2_set_pixel(cx - x, cy + y, color);
            gw_sdl2_set_pixel(cx - x, cy - y, color);
            gw_sdl2_set_pixel(cx - y, cy - x, color);
            gw_sdl2_set_pixel(cx + y, cy - x, color);
            gw_sdl2_set_pixel(cx + x, cy - y, color);
            
            y += 1;
            if (err <= 0) {
                err += 2*y + 1;
            } else {
                x -= 1;
                err -= 2*x + 2*y + 1;
            }
        }
    }
}

typedef struct {
    int x, y;
} FillPoint;

void gw_sdl2_paint(int start_x, int start_y, uint32_t fill_color, uint32_t border_color) {
    if (!g_pixels || start_x < 0 || start_x >= g_tex_width || start_y < 0 || start_y >= g_tex_height) return;
    
    uint32_t target_color = gw_sdl2_get_pixel(start_x, start_y);
    if (target_color == fill_color || target_color == border_color) return;
    
    int capacity = 4096;
    int size = 0;
    FillPoint *queue = (FillPoint *)malloc(capacity * sizeof(FillPoint));
    if (!queue) return;
    
    queue[size++] = (FillPoint){start_x, start_y};
    
    while (size > 0) {
        FillPoint p = queue[--size];
        
        if (p.x < 0 || p.x >= g_tex_width || p.y < 0 || p.y >= g_tex_height) continue;
        
        uint32_t current = gw_sdl2_get_pixel(p.x, p.y);
        if (current != target_color || current == fill_color || current == border_color) continue;
        
        gw_sdl2_set_pixel(p.x, p.y, fill_color);
        
        if (size + 4 >= capacity) {
            capacity *= 2;
            queue = (FillPoint *)realloc(queue, capacity * sizeof(FillPoint));
        }
        
        queue[size++] = (FillPoint){p.x + 1, p.y};
        queue[size++] = (FillPoint){p.x - 1, p.y};
        queue[size++] = (FillPoint){p.x, p.y + 1};
        queue[size++] = (FillPoint){p.x, p.y - 1};
    }
    
    free(queue);
}

void gw_sdl2_beep(void) {
#ifndef NO_SDL2
    if (g_pixels && g_audio_device > 0) {
        gw_sdl2_play_tone(800.0f, 150, 1);
    } else {
#ifdef _WIN32
        Beep(800, 150);
#else
        printf("\a");
        fflush(stdout);
#endif
    }
#else
    printf("\a");
    fflush(stdout);
#endif
}

void gw_sdl2_play_tone(float frequency, int duration_ms, int wait) {
#ifndef NO_SDL2
    if (g_pixels && g_audio_device > 0) {
        SDL_LockMutex(g_audio_mutex);
        g_audio_frequency = frequency;
        g_audio_phase = 0.0;
        SDL_UnlockMutex(g_audio_mutex);
        
        if (wait) {
            gw_sdl2_delay(duration_ms);
            SDL_LockMutex(g_audio_mutex);
            g_audio_frequency = 0.0f;
            SDL_UnlockMutex(g_audio_mutex);
        }
    } else {
        (void)wait;
#ifdef _WIN32
        if (frequency > 0) {
            Beep((DWORD)frequency, (DWORD)duration_ms);
        } else {
            Sleep((DWORD)duration_ms);
        }
#else
        if (frequency > 0) {
            printf("\a");
            fflush(stdout);
        }
        struct timespec ts;
        ts.tv_sec = duration_ms / 1000;
        ts.tv_nsec = (duration_ms % 1000) * 1000000;
        nanosleep(&ts, NULL);
#endif
    }
#else
    (void)wait;
#ifdef _WIN32
    if (frequency > 0) {
        Beep((DWORD)frequency, (DWORD)duration_ms);
    } else {
        Sleep((DWORD)duration_ms);
    }
#else
    // Linux terminal beep fallback (simple sleep)
    if (frequency > 0) {
        printf("\a");
        fflush(stdout);
    }
    struct timespec ts;
    ts.tv_sec = duration_ms / 1000;
    ts.tv_nsec = (duration_ms % 1000) * 1000000;
    nanosleep(&ts, NULL);
#endif
#endif
}

#ifndef NO_SDL2
// MML Parser Helper
static int parse_number(const char **ptr) {
    int val = 0;
    while (**ptr >= '0' && **ptr <= '9') {
        val = val * 10 + (**ptr - '0');
        (*ptr)++;
    }
    return val;
}

static int mml_thread_func(void *data) {
    (void)data;
    const char *p = g_music_mml;
    int octave = 4;
    int def_len = 4;
    int tempo = 120;
    
    while (*p && !g_music_stop_requested) {
        while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
        if (!*p) break;
        
        char cmd = *p++;
        if (cmd >= 'a' && cmd <= 'z') cmd = cmd - 32;
        
        if (cmd == 'O') {
            octave = parse_number(&p);
            if (octave < 0) octave = 0;
            if (octave > 6) octave = 6;
        } else if (cmd == '<') {
            if (octave > 0) octave--;
        } else if (cmd == '>') {
            if (octave < 6) octave++;
        } else if (cmd == 'L') {
            def_len = parse_number(&p);
            if (def_len <= 0) def_len = 4;
        } else if (cmd == 'T') {
            tempo = parse_number(&p);
            if (tempo < 32) tempo = 32;
            if (tempo > 255) tempo = 255;
        } else if (cmd == 'N') {
            int note_idx = parse_number(&p);
            int len = def_len;
            int duration_ms = (240000 / tempo) / len;
            if (note_idx <= 0 || note_idx > 84) {
                gw_sdl2_play_tone(0.0f, duration_ms, 1);
            } else {
                float freq = 440.0f * powf(2.0f, (float)(note_idx - 58) / 12.0f);
                gw_sdl2_play_tone(freq, duration_ms, 1);
            }
        } else if ((cmd >= 'A' && cmd <= 'G') || cmd == 'P') {
            char note = cmd;
            int accidental = 0;
            if (*p == '+' || *p == '#') {
                accidental = 1;
                p++;
            } else if (*p == '-') {
                accidental = -1;
                p++;
            }
            
            int len = def_len;
            if (*p >= '0' && *p <= '9') {
                len = parse_number(&p);
            }
            
            int duration_ms = (240000 / tempo) / len;
            
            if (note == 'P') {
                gw_sdl2_play_tone(0.0f, duration_ms, 1);
            } else {
                int note_offsets[] = { 9, 11, 0, 2, 4, 5, 7 };
                int offset = note_offsets[note - 'A'] + accidental;
                float freq = 440.0f * powf(2.0f, (float)(octave - 4) + (float)(offset - 9) / 12.0f);
                gw_sdl2_play_tone(freq, duration_ms, 1);
            }
        }
    }
    
    SDL_LockMutex(g_audio_mutex);
    g_audio_frequency = 0.0f;
    SDL_UnlockMutex(g_audio_mutex);
    return 0;
}
#endif

void gw_sdl2_play_mml(const char *mml_string) {
#ifndef NO_SDL2
    gw_sdl2_stop_music();
    
    SDL_LockMutex(g_music_mutex);
    g_music_stop_requested = 0;
    g_music_mml = plat_strdup(mml_string);
    g_music_thread = SDL_CreateThread(mml_thread_func, "MMLThread", NULL);
    SDL_UnlockMutex(g_music_mutex);
#else
    (void)mml_string;
#endif
}

int gw_sdl2_music_playing(void) {
#ifndef NO_SDL2
    int playing = 0;
    SDL_LockMutex(g_music_mutex);
    if (g_music_thread) {
        playing = 1;
    }
    SDL_UnlockMutex(g_music_mutex);
    return playing;
#else
    return 0;
#endif
}

void gw_sdl2_stop_music(void) {
#ifndef NO_SDL2
    SDL_LockMutex(g_music_mutex);
    if (g_music_thread) {
        g_music_stop_requested = 1;
        SDL_UnlockMutex(g_music_mutex);
        
        int status;
        SDL_WaitThread(g_music_thread, &status);
        
        SDL_LockMutex(g_music_mutex);
        g_music_thread = NULL;
        free(g_music_mml);
        g_music_mml = NULL;
    }
    SDL_UnlockMutex(g_music_mutex);
#endif
}

// Retro 8x8 IBM PC BIOS font table (ASCII 32 to 126)
static const uint8_t GW_FONT[96][8] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 32: [space]
    {0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x18, 0x00}, // 33: !
    {0x24, 0x24, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00}, // 34: "
    {0x24, 0x24, 0x7E, 0x24, 0x7E, 0x24, 0x24, 0x00}, // 35: #
    {0x08, 0x3E, 0x08, 0x3E, 0x08, 0x3E, 0x08, 0x00}, // 36: $
    {0x00, 0x62, 0x66, 0x0c, 0x18, 0x30, 0x46, 0x46}, // 37: %
    {0x38, 0x6c, 0x38, 0x76, 0xdc, 0xcc, 0x7e, 0x00}, // 38: &
    {0x0c, 0x0c, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00}, // 39: '
    {0x0c, 0x18, 0x30, 0x30, 0x30, 0x18, 0x0c, 0x00}, // 40: (
    {0x30, 0x18, 0x0c, 0x0c, 0x0c, 0x18, 0x30, 0x00}, // 41: )
    {0x00, 0x10, 0xd6, 0x38, 0xd6, 0x10, 0x00, 0x00}, // 42: *
    {0x00, 0x10, 0x10, 0x7c, 0x10, 0x10, 0x00, 0x00}, // 43: +
    {0x00, 0x00, 0x00, 0x00, 0x0c, 0x0c, 0x04, 0x08}, // 44: ,
    {0x00, 0x00, 0x00, 0x7c, 0x00, 0x00, 0x00, 0x00}, // 45: -
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00}, // 46: .
    {0x00, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00}, // 47: /
    {0x3c, 0x66, 0x6e, 0x76, 0x66, 0x66, 0x3c, 0x00}, // 48: 0
    {0x18, 0x1c, 0x18, 0x18, 0x18, 0x18, 0x7e, 0x00}, // 49: 1
    {0x3c, 0x66, 0x06, 0x0c, 0x30, 0x60, 0x7e, 0x00}, // 50: 2
    {0x3c, 0x66, 0x06, 0x1c, 0x06, 0x66, 0x3c, 0x00}, // 51: 3
    {0x06, 0x0e, 0x1e, 0x66, 0x7e, 0x06, 0x06, 0x00}, // 52: 4
    {0x7e, 0x60, 0x7c, 0x06, 0x06, 0x66, 0x3c, 0x00}, // 53: 5
    {0x3c, 0x66, 0x60, 0x7c, 0x66, 0x66, 0x3c, 0x00}, // 54: 6
    {0x7e, 0x66, 0x06, 0x0c, 0x18, 0x18, 0x18, 0x00}, // 55: 7
    {0x3c, 0x66, 0x66, 0x3c, 0x66, 0x66, 0x3c, 0x00}, // 56: 8
    {0x3c, 0x66, 0x66, 0x3e, 0x06, 0x66, 0x3c, 0x00}, // 57: 9
    {0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x00, 0x00}, // 58: :
    {0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x08, 0x10}, // 59: ;
    {0x0c, 0x18, 0x30, 0x60, 0x30, 0x18, 0x0c, 0x00}, // 60: <
    {0x00, 0x00, 0x7e, 0x00, 0x7e, 0x00, 0x00, 0x00}, // 61: =
    {0x30, 0x18, 0x0c, 0x06, 0x0c, 0x18, 0x30, 0x00}, // 62: >
    {0x3c, 0x66, 0x06, 0x0c, 0x18, 0x00, 0x18, 0x00}, // 63: ?
    {0x3c, 0x66, 0x6f, 0x7b, 0x73, 0x60, 0x3c, 0x00}, // 64: @
    {0x18, 0x3c, 0x66, 0x66, 0x7e, 0x66, 0x66, 0x00}, // 65: A
    {0x7c, 0x66, 0x66, 0x7c, 0x66, 0x66, 0x7c, 0x00}, // 66: B
    {0x3c, 0x66, 0x60, 0x60, 0x60, 0x66, 0x3c, 0x00}, // 67: C
    {0x78, 0x6c, 0x66, 0x66, 0x66, 0x6c, 0x78, 0x00}, // 68: D
    {0x7e, 0x60, 0x60, 0x7c, 0x60, 0x60, 0x7e, 0x00}, // 69: E
    {0x7e, 0x60, 0x60, 0x7c, 0x60, 0x60, 0x60, 0x00}, // 70: F
    {0x3c, 0x66, 0x60, 0x6e, 0x66, 0x66, 0x3e, 0x00}, // 71: G
    {0x66, 0x66, 0x66, 0x7e, 0x66, 0x66, 0x66, 0x00}, // 72: H
    {0x7e, 0x18, 0x18, 0x18, 0x18, 0x18, 0x7e, 0x00}, // 73: I
    {0x06, 0x06, 0x06, 0x06, 0x06, 0x66, 0x3c, 0x00}, // 74: J
    {0x66, 0x6c, 0x78, 0x70, 0x78, 0x6c, 0x66, 0x00}, // 75: K
    {0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7e, 0x00}, // 76: L
    {0x63, 0x77, 0x7f, 0x6b, 0x63, 0x63, 0x63, 0x00}, // 77: M
    {0x66, 0x76, 0x7e, 0x76, 0x6e, 0x66, 0x66, 0x00}, // 78: N
    {0x3c, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x00}, // 79: O
    {0x7c, 0x66, 0x66, 0x7c, 0x60, 0x60, 0x60, 0x00}, // 80: P
    {0x3c, 0x66, 0x66, 0x66, 0x6a, 0x6c, 0x36, 0x00}, // 81: Q
    {0x7c, 0x66, 0x66, 0x7c, 0x78, 0x6c, 0x66, 0x00}, // 82: R
    {0x3c, 0x66, 0x60, 0x3c, 0x06, 0x66, 0x3c, 0x00}, // 83: S
    {0x7e, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00}, // 84: T
    {0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x00}, // 85: U
    {0x66, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x18, 0x00}, // 86: V
    {0x63, 0x63, 0x63, 0x6b, 0x7f, 0x77, 0x63, 0x00}, // 87: W
    {0x66, 0x66, 0x3c, 0x18, 0x3c, 0x66, 0x66, 0x00}, // 88: X
    {0x66, 0x66, 0x66, 0x3c, 0x18, 0x18, 0x18, 0x00}, // 89: Y
    {0x7e, 0x06, 0x0c, 0x18, 0x30, 0x60, 0x7e, 0x00}, // 90: Z
    {0x3c, 0x30, 0x30, 0x30, 0x30, 0x30, 0x3c, 0x00}, // 91: [
    {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x00}, // 92: backslash
    {0x3c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x3c, 0x00}, // 93: ]
    {0x08, 0x1c, 0x36, 0x63, 0x00, 0x00, 0x00, 0x00}, // 94: ^
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00}, // 95: _
    {0x18, 0x18, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00}, // 96: `
    {0x00, 0x00, 0x3c, 0x06, 0x3e, 0x66, 0x3e, 0x00}, // 97: a
    {0x60, 0x60, 0x7c, 0x66, 0x66, 0x66, 0x7c, 0x00}, // 98: b
    {0x00, 0x00, 0x3c, 0x60, 0x60, 0x66, 0x3c, 0x00}, // 99: c
    {0x06, 0x06, 0x3e, 0x66, 0x66, 0x66, 0x3e, 0x00}, // 100: d
    {0x00, 0x00, 0x3c, 0x66, 0x7e, 0x60, 0x3c, 0x00}, // 101: e
    {0x1c, 0x36, 0x30, 0x7c, 0x30, 0x30, 0x30, 0x00}, // 102: f
    {0x00, 0x00, 0x3e, 0x66, 0x66, 0x3e, 0x06, 0x3c}, // 103: g
    {0x60, 0x60, 0x7c, 0x66, 0x66, 0x66, 0x66, 0x00}, // 104: h
    {0x18, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00}, // 105: i
    {0x0c, 0x00, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x38}, // 106: j
    {0x60, 0x60, 0x66, 0x6c, 0x78, 0x6c, 0x66, 0x00}, // 107: k
    {0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00}, // 108: l
    {0x00, 0x00, 0x6e, 0x7f, 0x6b, 0x63, 0x63, 0x00}, // 109: m
    {0x00, 0x00, 0x7c, 0x66, 0x66, 0x66, 0x66, 0x00}, // 110: n
    {0x00, 0x00, 0x3c, 0x66, 0x66, 0x66, 0x3c, 0x00}, // 111: o
    {0x00, 0x00, 0x7c, 0x66, 0x66, 0x7c, 0x60, 0x60}, // 112: p
    {0x00, 0x00, 0x3e, 0x66, 0x66, 0x3e, 0x06, 0x06}, // 113: q
    {0x00, 0x00, 0x7c, 0x66, 0x60, 0x60, 0x60, 0x00}, // 114: r
    {0x00, 0x00, 0x3e, 0x60, 0x3c, 0x06, 0x3c, 0x00}, // 115: s
    {0x30, 0x30, 0x7c, 0x30, 0x30, 0x30, 0x1c, 0x00}, // 116: t
    {0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x3e, 0x00}, // 117: u
    {0x00, 0x00, 0x66, 0x66, 0x66, 0x3c, 0x18, 0x00}, // 118: v
    {0x00, 0x00, 0x63, 0x6b, 0x7f, 0x3e, 0x36, 0x00}, // 119: w
    {0x00, 0x00, 0x66, 0x3c, 0x18, 0x3c, 0x66, 0x00}, // 120: x
    {0x00, 0x00, 0x66, 0x66, 0x66, 0x3e, 0x06, 0x3c}, // 121: y
    {0x00, 0x00, 0x7e, 0x0c, 0x18, 0x30, 0x7e, 0x00}, // 122: z
    {0x0c, 0x18, 0x18, 0x30, 0x18, 0x18, 0x0c, 0x00}, // 123: {
    {0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00}, // 124: |
    {0x30, 0x18, 0x18, 0x0c, 0x18, 0x18, 0x30, 0x00}, // 125: }
    {0x76, 0x5d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}  // 126: ~
};

void gw_sdl2_update_palette(int screen_mode, int machine_type, int bg_color, int palette_idx) {
    g_current_mode = screen_mode;
    g_cga_bg_color = bg_color;
    g_cga_palette = palette_idx;
    
    if (g_is_atari_graphics) {
        static const struct { uint8_t r, g, b; } atari_hues[16] = {
            {255, 255, 255}, // 0: Grayscale
            {255, 192, 0},   // 1: Gold
            {255, 128, 0},   // 2: Orange
            {255, 64, 0},    // 3: Red-Orange
            {255, 0, 0},     // 4: Red
            {255, 0, 128},   // 5: Pink
            {192, 0, 255},   // 6: Purple
            {128, 0, 255},   // 7: Blue-Purple
            {0, 0, 255},     // 8: Blue
            {0, 128, 255},   // 9: Light Blue
            {0, 255, 255},   // 10: Turquoise
            {0, 255, 128},   // 11: Green-Blue
            {0, 255, 0},     // 12: Green
            {128, 255, 0},   // 13: Yellow-Green
            {192, 192, 0},   // 14: Olive
            {255, 150, 50}    // 15: Light Orange
        };
        for (int h = 0; h < 16; h++) {
            for (int l = 0; l < 16; l++) {
                int idx = h * 16 + l;
                double intensity = l / 15.0;
                uint8_t r, g, b;
                if (h == 0) {
                    r = g = b = (uint8_t)(l * 17);
                } else {
                    r = (uint8_t)(atari_hues[h].r * intensity);
                    g = (uint8_t)(atari_hues[h].g * intensity);
                    b = (uint8_t)(atari_hues[h].b * intensity);
                }
                GW_PALETTE[idx] = (r << 24) | (g << 16) | (b << 8) | 0xFF;
            }
        }
        return;
    }
    
    // Copy default IBM palette first
    for (int i = 0; i < 16; i++) {
        GW_PALETTE[i] = gw_get_base_palette()[i];
    }
    
    if (machine_type == 1) { // HGC (Hercules Graphics Card)
        // Monochrome Phosphor Green: 0 is black, all others are green
        GW_PALETTE[0] = 0x000000FF; // Black
        for (int i = 1; i < 16; i++) {
            GW_PALETTE[i] = 0x00CC00FF; // Green Phosphor
        }
    } else if (machine_type == 8) { // MACHINE_MDA
        uint32_t mono_color = 0xFFB000FF; // Amber default
        if (strcmp(g_mda_color, "green") == 0 || strcmp(g_mda_color, "GREEN") == 0) {
            mono_color = 0x00CC00FF; // Green
        } else if (strcmp(g_mda_color, "grey") == 0 || strcmp(g_mda_color, "gray") == 0 ||
                   strcmp(g_mda_color, "GREY") == 0 || strcmp(g_mda_color, "GRAY") == 0) {
            mono_color = 0x888888FF; // Grey
        } else if (strcmp(g_mda_color, "white") == 0 || strcmp(g_mda_color, "WHITE") == 0) {
            mono_color = 0xFFFFFFFF; // White
        }
        GW_PALETTE[0] = 0x000000FF; // Black background
        for (int i = 1; i < 16; i++) {
            GW_PALETTE[i] = mono_color;
        }
    } else if (screen_mode == 1) {
        if (machine_type == 2 || machine_type == 3 || machine_type == 4 || machine_type == 5) {
            // Tandy, PCjr, Plantronics, AT&T: 16 colors in SCREEN 1
        } else {
            // Standard CGA SCREEN 1: 4 colors
            GW_PALETTE[0] = gw_get_base_palette()[bg_color % 16];
            if (palette_idx == 0) {
                GW_PALETTE[1] = gw_get_base_palette()[2]; // Green
                GW_PALETTE[2] = gw_get_base_palette()[4]; // Red
                GW_PALETTE[3] = gw_get_base_palette()[6]; // Brown
            } else {
                GW_PALETTE[1] = gw_get_base_palette()[3]; // Cyan
                GW_PALETTE[2] = gw_get_base_palette()[5]; // Magenta
                GW_PALETTE[3] = gw_get_base_palette()[7]; // Light Gray
            }
        }
    } else if (screen_mode == 2) {
        if (machine_type == 2 || machine_type == 3 || machine_type == 4 || machine_type == 5 || machine_type == 6) {
            // Tandy, PCjr, Plantronics, AT&T, Amstrad: SCREEN 2 has standard colors
        } else {
            // Standard CGA SCREEN 2: 2-color mode
            GW_PALETTE[0] = gw_get_base_palette()[bg_color % 16];
            GW_PALETTE[1] = gw_get_base_palette()[15]; // White
            for (int i = 2; i < 16; i++) {
                GW_PALETTE[i] = gw_get_base_palette()[15];
            }
        }
    }
}

void gw_sdl2_set_machine(int machine_type) {
    g_machine_type = machine_type;
    gw_sdl2_update_palette(g_current_mode, g_machine_type, g_cga_bg_color, g_cga_palette);
}

void gw_sdl2_set_mode(int mode, int cols) {
    g_is_atari_graphics = 0;
    g_current_mode = mode;
    if (!g_sdl_active) {
        g_grid_cols = cols;
        g_cursor_x = 0;
        g_cursor_y = 0;
        memset(g_screen_chars, ' ', sizeof(g_screen_chars));
        return;
    }
    gw_sdl2_update_palette(mode, g_machine_type, g_cga_bg_color, g_cga_palette);
    int target_width = 640;
    int target_height = 400;
    
    switch (mode) {
        case 0:
            if (g_machine_type == 1) { // HGC
                target_width = 720;
                target_height = 348;
            } else if (g_machine_type == 2 || g_machine_type == 3) { // Tandy/PCjr
                target_width = 640;
                target_height = 200;
            } else {
                target_width = 640;
                target_height = 400;
            }
            break;
        case 1:
        case 7:
        case 13:
            target_width = 320;
            target_height = 200;
            break;
        case 2:
        case 8:
            target_width = 640;
            target_height = 200;
            break;
        case 3:
            if (g_machine_type == 1) { // HGC
                target_width = 720;
                target_height = 348;
            } else if (g_machine_type == 2 || g_machine_type == 3) { // Tandy/PCjr
                target_width = 160;
                target_height = 200;
            } else {
                target_width = 640;
                target_height = 400;
            }
            break;
        case 4:
        case 5:
            target_width = 320;
            target_height = 200;
            break;
        case 6:
            target_width = 640;
            target_height = 200;
            break;
        case 9:
            if (g_machine_type == 2) { // Tandy
                target_width = 640;
                target_height = 200;
            } else {
                target_width = 640;
                target_height = 350;
            }
            break;
        case 10:
            target_width = 640;
            target_height = 350;
            break;
        case 11:
        case 12:
            target_width = 640;
            target_height = 480;
            break;
        case 14:
            if (g_machine_type == 4) { // Plantronics
                target_width = 320;
                target_height = 200;
            } else if (g_machine_type == 5) { // AT&T
                target_width = 640;
                target_height = 400;
            } else if (g_machine_type == 6) { // Amstrad
                target_width = 640;
                target_height = 200;
            } else {
                target_width = 640;
                target_height = 200;
            }
            break;
        case 15:
            if (g_machine_type == 4) { // Plantronics
                target_width = 640;
                target_height = 200;
            } else if (g_machine_type == 7) { // PC98
                target_width = 640;
                target_height = 400;
            } else {
                target_width = 640;
                target_height = 350;
            }
            break;
        default:
            target_width = 640;
            target_height = 400;
            break;
    }
    
    g_tex_width = target_width;
    g_tex_height = target_height;
    
    g_grid_cols = cols;
    if (target_width == 160) {
        g_grid_cols = 20;
    } else if (target_width == 320) {
        g_grid_cols = 40;
    } else if (target_width == 720) {
        g_grid_cols = (mode == 3) ? 90 : 80;
    } else if (target_width == 640 && mode != 0) {
        g_grid_cols = 80;
    }
    
    if (target_height == 480) {
        g_grid_rows = 30;
    } else {
        g_grid_rows = 25;
    }
    
    if (g_grid_cols > MAX_GRID_COLS) g_grid_cols = MAX_GRID_COLS;
    if (g_grid_rows > MAX_GRID_ROWS) g_grid_rows = MAX_GRID_ROWS;
    
    boot_log(BOOT_DEBUG, "SDL2: Screen mode set to %d (%d cols), target resolution %dx%d (grid: %dx%d)", 
             mode, cols, target_width, target_height, g_grid_cols, g_grid_rows);
    
    g_cursor_x = 0;
    g_cursor_y = 0;
    memset(g_screen_chars, ' ', sizeof(g_screen_chars));
    
    if (g_pixels) {
        free(g_pixels);
    }
    g_pixels = (uint32_t *)calloc(target_width * target_height, sizeof(uint32_t));
    
#ifndef NO_SDL2
    if (g_renderer) {
        if (g_texture) {
            boot_log(BOOT_DEBUG, "SDL2: Recreating texture for resolution change");
            SDL_DestroyTexture(g_texture);
        }
        g_texture = SDL_CreateTexture(g_renderer, SDL_PIXELFORMAT_RGBA8888, 
                                      SDL_TEXTUREACCESS_STREAMING, target_width, target_height);
        SDL_RenderSetLogicalSize(g_renderer, target_width, target_height);
    }
#endif
    gw_sdl2_clear(g_text_bg);
    gw_sdl2_present();
}

void gw_sdl2_set_atari_graphics(int is_atari, int mode) {
    g_is_atari_graphics = is_atari;
    g_atari_graphics_mode = mode;
    
    if (!is_atari) {
        g_text_bg = 0x000000FF; // Black
        g_text_fg = 0xFFFFFFFF; // White
        return;
    }
    
    // Force palette generation to populate GW_PALETTE
    gw_sdl2_update_palette(mode, g_machine_type, g_cga_bg_color, g_cga_palette);
    
    // Atari text mode colors
    g_text_bg = GW_PALETTE[116]; // Blue (Hue 7, Luma 4)
    g_text_fg = GW_PALETTE[238]; // White (Hue 0, Luma 14)
    
    int base_mode = mode % 16;
    int modifier = (mode % 64) - base_mode;
    int is_256_color = (mode >= 64);
    int has_text_window = (modifier == 0 || modifier == 32);
    
    int target_width = 320;
    int target_height = 192;
    
    switch (base_mode) {
        case 0:
        case 3:
        case 8:
        case 15:
            target_width = 320;
            break;
        case 1:
        case 2:
        case 6:
        case 7:
        case 12:
        case 13:
        case 14:
            target_width = 160;
            break;
        case 4:
        case 5:
        case 9:
        case 10:
        case 11:
            target_width = 80;
            break;
        default:
            target_width = 320;
            break;
    }
    
    if (base_mode == 3) {
        target_height = 24;
    } else if (base_mode == 4 || base_mode == 6) {
        target_height = 48;
    } else if (base_mode == 5 || base_mode == 7) {
        target_height = 96;
    } else {
        target_height = 192;
    }
    
    g_tex_width = target_width;
    g_tex_height = target_height;
    
    if (base_mode == 0) {
        g_grid_cols = is_256_color ? 80 : 40;
        g_grid_rows = 24;
        g_scroll_start = 0;
        g_scroll_lines = 24;
    } else if (base_mode == 1) {
        g_grid_cols = is_256_color ? 40 : 20;
        g_grid_rows = 24;
        g_scroll_start = 0;
        g_scroll_lines = 24;
    } else if (base_mode == 2) {
        g_grid_cols = is_256_color ? 40 : 20;
        g_grid_rows = 12;
        g_scroll_start = 0;
        g_scroll_lines = 12;
    } else {
        if (has_text_window) {
            g_grid_cols = is_256_color ? 80 : 40;
            g_grid_rows = 24;
            g_scroll_start = 20;
            g_scroll_lines = 4;
        } else {
            g_grid_cols = is_256_color ? 80 : 40;
            g_grid_rows = 24;
            g_scroll_start = 0;
            g_scroll_lines = 24;
        }
    }
    
    g_cursor_x = 0;
    g_cursor_y = g_scroll_start;
    memset(g_screen_chars, ' ', sizeof(g_screen_chars));
    
    if (g_pixels) {
        free(g_pixels);
    }
    g_pixels = (uint32_t *)calloc(target_width * target_height, sizeof(uint32_t));
    
#ifndef NO_SDL2
    if (g_renderer) {
        if (g_texture) {
            SDL_DestroyTexture(g_texture);
        }
        g_texture = SDL_CreateTexture(g_renderer, SDL_PIXELFORMAT_RGBA8888, 
                                      SDL_TEXTUREACCESS_STREAMING, target_width, target_height);
        SDL_RenderSetLogicalSize(g_renderer, target_width, target_height);
    }
#endif
    
    gw_sdl2_clear(g_text_bg);
    gw_sdl2_present();
}

void gw_sdl2_set_text_color(uint32_t fg, uint32_t bg) {
    g_text_fg = fg;
    g_text_bg = bg;
}

uint32_t gw_sdl2_get_text_fg(void) {
    return g_text_fg;
}

uint32_t gw_sdl2_get_text_bg(void) {
    return g_text_bg;
}

void gw_sdl2_set_console(int start, int lines, int fn_keys, int mono) {
    if (start >= 0 && start < g_grid_rows) g_scroll_start = start;
    if (lines > 0 && g_scroll_start + lines <= g_grid_rows) g_scroll_lines = lines;
    if (fn_keys >= 0) g_show_fn_keys = fn_keys;
    if (mono >= 0) g_mono_mode = mono;
}

static void scroll_screen(void) {
    int start = g_scroll_start;
    int num = g_scroll_lines;
    if (start < 0) start = 0;
    if (num <= 0) num = g_grid_rows - start;
    if (start + num > g_grid_rows) num = g_grid_rows - start;
    if (num <= 1) return; // Nothing to scroll

    int char_h = 8;
    if (g_tex_width == 640 && g_tex_height == 400) { // Screen 0 Text
        char_h = 16;
    } else if (g_tex_height == 480) { // SCREEN 11, 12
        char_h = 16;
    } else if (g_tex_height == 350) { // SCREEN 9, 10
        char_h = 14;
    } else if (g_tex_width == 720) { // HGC 720x348
        char_h = 14;
    } else {
        char_h = 8;
    }
    
    // Shift text buffer rows
    memmove(g_screen_chars[start], g_screen_chars[start + 1], (num - 1) * MAX_GRID_COLS);
    memset(g_screen_chars[start + num - 1], ' ', MAX_GRID_COLS);
    
    if (g_pixels) {
        int row_pixels = g_tex_width * char_h;
        int start_pixel_row = start * char_h;
        int num_pixel_rows = num * char_h;
        uint32_t *dest = g_pixels + start_pixel_row * g_tex_width;
        uint32_t *src = dest + row_pixels;
        memmove(dest, src, (num_pixel_rows - char_h) * g_tex_width * sizeof(uint32_t));
        
        // Clear bottom row of the scroll window
        uint32_t *clear_start = dest + (num_pixel_rows - char_h) * g_tex_width;
        for (int i = 0; i < row_pixels; i++) {
            clear_start[i] = g_text_bg;
        }
    }
}

static void draw_char_cell(char c, int grid_x, int grid_y, int save_to_grid) {
    if (save_to_grid && grid_x >= 0 && grid_x < g_grid_cols && grid_y >= 0 && grid_y < g_grid_rows) {
        g_screen_chars[grid_y][grid_x] = c;
    }
    if (!g_pixels) return;
    
    uint8_t glyph[8];
    if (c >= 32 && c <= 126) {
        memcpy(glyph, GW_FONT[c - 32], 8);
    } else {
        memset(glyph, 0xFF, 8); // solid block for others
    }
    
    if (g_is_atari_graphics) {
        int base_mode = g_atari_graphics_mode % 16;
        int modifier = (g_atari_graphics_mode % 64) - base_mode;
        int is_256_color = (g_atari_graphics_mode >= 64);
        int has_text_window = (modifier == 0 || modifier == 32);
        int in_text_window = (has_text_window && grid_y >= g_grid_rows - 4);
        
        if (in_text_window) {
            int text_cols = is_256_color ? 80 : 40;
            int char_w = g_tex_width / text_cols;
            if (char_w < 1) char_w = 1;
            int scale_x = char_w >= 8 ? char_w / 8 : 1;
            int start_pixel_x = grid_x * char_w;
            int start_pixel_y = g_tex_height - 32 + (grid_y - (g_grid_rows - 4)) * 8;
            
            for (int dy = 0; dy < 8; dy++) {
                for (int dx = 0; dx < char_w; dx++) {
                    gw_sdl2_set_pixel(start_pixel_x + dx, start_pixel_y + dy, g_text_bg);
                }
            }
            for (int r = 0; r < 8; r++) {
                uint8_t row_bits = glyph[r];
                for (int c_idx = 0; c_idx < char_w && c_idx < 8; c_idx++) {
                    int src_col = (char_w == 4) ? c_idx * 2 : c_idx;
                    if (src_col >= 8) src_col = 7;
                    int bit = (row_bits >> (7 - src_col)) & 1;
                    uint32_t color = bit ? g_text_fg : g_text_bg;
                    if (color == g_text_bg) continue;
                    for (int sx = 0; sx < scale_x; sx++) {
                        gw_sdl2_set_pixel(start_pixel_x + c_idx * scale_x + sx, start_pixel_y + r, color);
                    }
                }
            }
            return;
        } else {
            int char_w = g_tex_width / g_grid_cols;
            int char_h = g_tex_height / g_grid_rows;
            if (char_w < 1) char_w = 1;
            if (char_h < 1) char_h = 1;
            int scale_x = char_w >= 8 ? char_w / 8 : 1;
            int scale_y = char_h >= 8 ? char_h / 8 : 1;
            int start_pixel_x = grid_x * char_w;
            int start_pixel_y = grid_y * char_h;
            
            for (int dy = 0; dy < char_h; dy++) {
                for (int dx = 0; dx < char_w; dx++) {
                    gw_sdl2_set_pixel(start_pixel_x + dx, start_pixel_y + dy, g_text_bg);
                }
            }
            for (int r = 0; r < 8 && r < char_h; r++) {
                uint8_t row_bits = glyph[r];
                for (int c_idx = 0; c_idx < char_w && c_idx < 8; c_idx++) {
                    int bit = (row_bits >> (7 - c_idx)) & 1;
                    uint32_t color = bit ? g_text_fg : g_text_bg;
                    if (color == g_text_bg) continue;
                    for (int sy = 0; sy < scale_y; sy++) {
                        for (int sx = 0; sx < scale_x; sx++) {
                            gw_sdl2_set_pixel(start_pixel_x + c_idx * scale_x + sx, start_pixel_y + r * scale_y + sy, color);
                        }
                    }
                }
            }
            return;
        }
    }
    
    int char_w = 8;
    int char_h = 8;
    int scale_x = 1;
    int scale_y = 1;
    int pad_y = 0;
    
    if (g_tex_width == 720) { // Hercules (720x348)
        char_w = 8;
        char_h = 14;
        scale_x = 1;
        scale_y = 1;
        pad_y = 3;
    } else if (g_tex_width == 640 && g_tex_height == 400) { // Screen 0 Text
        if (g_grid_cols == 40) {
            char_w = 16;
            char_h = 16;
            scale_x = 2;
            scale_y = 2;
        } else {
            char_w = 8;
            char_h = 16;
            scale_x = 1;
            scale_y = 2;
        }
    } else if (g_tex_height == 480) { // SCREEN 11, 12
        char_w = 8;
        char_h = 16;
        scale_x = 1;
        scale_y = 2;
    } else if (g_tex_height == 350) { // SCREEN 9, 10
        char_w = 8;
        char_h = 14;
        scale_x = 1;
        scale_y = 1;
        pad_y = 3;
    } else {
        char_w = 8;
        char_h = 8;
        scale_x = 1;
        scale_y = 1;
    }
    
    int start_pixel_x = grid_x * char_w;
    int start_pixel_y = grid_y * char_h;
    
    // Clear block with background color first
    for (int dy = 0; dy < char_h; dy++) {
        for (int dx = 0; dx < char_w; dx++) {
            gw_sdl2_set_pixel(start_pixel_x + dx, start_pixel_y + dy, g_text_bg);
        }
    }
    
    // Draw font bits
    for (int r = 0; r < 8; r++) {
        uint8_t row_bits = glyph[r];
        for (int c_idx = 0; c_idx < 8; c_idx++) {
            int bit = (row_bits >> (7 - c_idx)) & 1;
            uint32_t color = bit ? g_text_fg : g_text_bg;
            if (color == g_text_bg) continue;
            
            // Draw scaled pixel
            for (int sy = 0; sy < scale_y; sy++) {
                for (int sx = 0; sx < scale_x; sx++) {
                    int px = start_pixel_x + c_idx * scale_x + sx;
                    int py = start_pixel_y + pad_y + r * scale_y + sy;
                    gw_sdl2_set_pixel(px, py, color);
                }
            }
        }
    }
}

void gw_sdl2_write_char(char c) {
    if (c >= 0 && c < 32) {
        if (c == '\a') {
            gw_sdl2_beep();
        }
        if (c != '\n' && c != '\r' && c != '\t' && c != '\b') {
            return;
        }
    }
    int start = g_scroll_start;
    int num = g_scroll_lines;
    if (start < 0) start = 0;
    if (num <= 0) num = g_grid_rows - start;
    if (start + num > g_grid_rows) num = g_grid_rows - start;
    int scroll_end = start + num;
    int max_y = (g_cursor_y >= start && g_cursor_y < scroll_end) ? scroll_end : g_grid_rows;

    if (c == '\n') {
        g_cursor_x = 0;
        g_cursor_y++;
        if (g_cursor_y >= max_y) {
            if (max_y == scroll_end) {
                scroll_screen();
                g_cursor_y = scroll_end - 1;
            } else {
                int old_start = g_scroll_start;
                int old_lines = g_scroll_lines;
                g_scroll_start = 0;
                g_scroll_lines = g_grid_rows;
                scroll_screen();
                g_scroll_start = old_start;
                g_scroll_lines = old_lines;
                g_cursor_y = g_grid_rows - 1;
            }
        }
        gw_sdl2_present();
        return;
    }
    if (c == '\r') {
        g_cursor_x = 0;
        return;
    }
    if (c == '\t') {
        g_cursor_x = (g_cursor_x + 8) & ~7;
        if (g_cursor_x >= g_grid_cols) {
            g_cursor_x = 0;
            g_cursor_y++;
            if (g_cursor_y >= max_y) {
                if (max_y == scroll_end) {
                    scroll_screen();
                    g_cursor_y = scroll_end - 1;
                } else {
                    int old_start = g_scroll_start;
                    int old_lines = g_scroll_lines;
                    g_scroll_start = 0;
                    g_scroll_lines = g_grid_rows;
                    scroll_screen();
                    g_scroll_start = old_start;
                    g_scroll_lines = old_lines;
                    g_cursor_y = g_grid_rows - 1;
                }
            }
        }
        gw_sdl2_present();
        return;
    }
    if (c == '\b') {
        if (g_cursor_x > 0) {
            g_cursor_x--;
            draw_char_cell(' ', g_cursor_x, g_cursor_y, 1);
            gw_sdl2_present();
        }
        return;
    }
    
    draw_char_cell(c, g_cursor_x, g_cursor_y, 1);
    g_cursor_x++;
    if (g_cursor_x >= g_grid_cols) {
        g_cursor_x = 0;
        g_cursor_y++;
        if (g_cursor_y >= max_y) {
            if (max_y == scroll_end) {
                scroll_screen();
                g_cursor_y = scroll_end - 1;
            } else {
                int old_start = g_scroll_start;
                int old_lines = g_scroll_lines;
                g_scroll_start = 0;
                g_scroll_lines = g_grid_rows;
                scroll_screen();
                g_scroll_start = old_start;
                g_scroll_lines = old_lines;
                g_cursor_y = g_grid_rows - 1;
            }
        }
    }
    static int normal_char_count = 0;
    normal_char_count++;
    if (normal_char_count >= 64) {
        normal_char_count = 0;
        gw_sdl2_present();
    }
}

void gw_sdl2_clear_screen(uint32_t color) {
    gw_sdl2_clear(color);
    g_cursor_x = 0;
    g_cursor_y = 0;
    memset(g_screen_chars, ' ', sizeof(g_screen_chars));
    gw_sdl2_present_force();
}

void gw_sdl2_set_cursor(int x, int y) {
    if (x >= 0 && x < g_grid_cols) g_cursor_x = x;
    if (y >= 0 && y < g_grid_rows) g_cursor_y = y;
}

char gw_sdl2_get_char(int x, int y) {
    if (x >= 0 && x < g_grid_cols && y >= 0 && y < g_grid_rows) {
        return g_screen_chars[y][x];
    }
    return ' ';
}

int gw_sdl2_get_width(void) {
    return g_tex_width;
}

int gw_sdl2_get_height(void) {
    return g_tex_height;
}

int gw_sdl2_get_mode(void) {
    return g_current_mode;
}

uint32_t gw_sdl2_ticks(void) {
#ifndef NO_SDL2
    return SDL_GetTicks();
#else
    return 0;
#endif
}

void gw_sdl2_delay(int ms) {
#ifndef NO_SDL2
    uint32_t start = SDL_GetTicks();
    while (SDL_GetTicks() - start < (uint32_t)ms) {
        gw_sdl2_poll_events();
        SDL_Delay(5);
    }
#endif
}

void gw_sdl2_write_char_cursor(int visible) {
    if (visible) {
        draw_char_cell('_', g_cursor_x, g_cursor_y, 0);
    } else {
        char current_char = ' ';
        if (g_cursor_x >= 0 && g_cursor_x < g_grid_cols && g_cursor_y >= 0 && g_cursor_y < g_grid_rows) {
            current_char = g_screen_chars[g_cursor_y][g_cursor_x];
            if (current_char == '\0') current_char = ' ';
        }
        draw_char_cell(current_char, g_cursor_x, g_cursor_y, 0);
    }
    gw_sdl2_present_force();
}
#endif

