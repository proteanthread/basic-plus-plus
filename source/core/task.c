#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "task.h"
#include "errors.h"
#include "fileio.h"
#include "../console.h"
#include "../platform.h"

#ifdef BPP_LITE_BUILD
void task_mutex_lock(void) {}
void task_mutex_unlock(void) {}
int task_spawn(const char *filename, int line_num) { (void)filename; (void)line_num; return -1; }
int task_get_status(int pid) { (void)pid; return 0; }
void task_list(void) {}
void task_scheduler_tick(void) {}
int task_has_background_active(void) { return 0; }
void task_mgr_shutdown(void) {}
void task_mgr_init(RuntimeState *main_rt) { (void)main_rt; }
BasicTask *task_get_current(void) { return NULL; }
MemorySystem *task_get_main_mem(void) { return NULL; }
#else

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define TokenType WinTokenType
#include <windows.h>
#undef TokenType
static CRITICAL_SECTION g_task_cs;
static int g_cs_initialized = 0;
#elif defined(__linux__) || defined(__APPLE__) || defined(__unix__)
#include <unistd.h>
static pthread_mutex_t g_task_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

extern void exec_run_step_cooperative(RuntimeState *rt);

void task_mutex_lock(void)
{
#ifdef _WIN32
    if (g_cs_initialized) {
        EnterCriticalSection(&g_task_cs);
    }
#elif defined(__linux__) || defined(__APPLE__) || defined(__unix__)
    pthread_mutex_lock(&g_task_mutex);
#endif
}

void task_mutex_unlock(void)
{
#ifdef _WIN32
    if (g_cs_initialized) {
        LeaveCriticalSection(&g_task_cs);
    }
#elif defined(__linux__) || defined(__APPLE__) || defined(__unix__)
    pthread_mutex_unlock(&g_task_mutex);
#endif
}

#if defined(_WIN32)
static DWORD WINAPI task_thread_worker(LPVOID param)
{
    BasicTask *task = (BasicTask *)param;
    RuntimeState *task_rt = task->rt;
    while (task_rt->running && !task_rt->stopped && task_rt->current_index < task_rt->program->count && task_rt->vm_state == 1) {
        if (task->state == TASK_RUNNING_BG) {
            exec_run_step_cooperative(task_rt);
        }
        Sleep(1);
    }
    if (task_rt->stopped || task_rt->current_index >= task_rt->program->count) {
        task->state = TASK_DONE;
        printf("Task %d completed execution.\n", task->pid);
    } else if (task_rt->vm_state == 3) { // VM_ERROR
        task->state = TASK_ERROR;
        printf("Task %d halted on error.\n", task->pid);
    }
    return 0;
}
#elif defined(__linux__) || defined(__APPLE__) || defined(__unix__)
static void *task_thread_worker(void *param)
{
    BasicTask *task = (BasicTask *)param;
    RuntimeState *task_rt = task->rt;
    while (task_rt->running && !task_rt->stopped && task_rt->current_index < task_rt->program->count && task_rt->vm_state == 1) {
        if (task->state == TASK_RUNNING_BG) {
            exec_run_step_cooperative(task_rt);
        }
        platform_sleep_ms(1);
    }
    if (task_rt->stopped || task_rt->current_index >= task_rt->program->count) {
        task->state = TASK_DONE;
        printf("Task %d completed execution.\n", task->pid);
    } else if (task_rt->vm_state == 3) { // VM_ERROR
        task->state = TASK_ERROR;
        printf("Task %d halted on error.\n", task->pid);
    }
    return NULL;
}
#endif

static BasicTask g_tasks[MAX_TASKS];
static int g_current_task_pid = 0;
static int g_next_pid = 1;


void task_mgr_init(RuntimeState *main_rt)
{
    int i;
#ifdef _WIN32
    InitializeCriticalSection(&g_task_cs);
    g_cs_initialized = 1;
#endif

    for (i = 0; i < MAX_TASKS; i++) {
        g_tasks[i].pid = 0;
        g_tasks[i].rt = NULL;
        g_tasks[i].mem = NULL;
        g_tasks[i].state = TASK_DONE;
        g_tasks[i].active_bank_id = 0;
        g_tasks[i].is_used = 0;
        g_tasks[i].is_threaded = 0;
#if defined(_WIN32)
        g_tasks[i].thread_handle = NULL;
#elif defined(__linux__) || defined(__APPLE__) || defined(__unix__)
        g_tasks[i].has_thread = 0;
#endif
    }

    // Register foreground main task
    g_tasks[0].pid = 0;
    g_tasks[0].rt = main_rt;
    g_tasks[0].mem = main_rt->memory;
    g_tasks[0].state = TASK_RUNNING_FG;
    g_tasks[0].active_bank_id = 0;
    g_tasks[0].is_used = 1;
    strcpy(g_tasks[0].filename, "main");
    g_current_task_pid = 0;
}

void task_mgr_shutdown(void)
{
    int i;
    // First, signal all threads to stop
    for (i = 1; i < MAX_TASKS; i++) {
        if (g_tasks[i].is_used && g_tasks[i].rt != NULL) {
            g_tasks[i].rt->running = 0;
            g_tasks[i].rt->stopped = 1;
        }
    }

    // Now wait and clean up resources
    for (i = 1; i < MAX_TASKS; i++) {
        if (g_tasks[i].is_used) {
#if defined(_WIN32)
            if (g_tasks[i].thread_handle != NULL) {
                WaitForSingleObject(g_tasks[i].thread_handle, 200);
                CloseHandle(g_tasks[i].thread_handle);
                g_tasks[i].thread_handle = NULL;
            }
#elif defined(__linux__) || defined(__APPLE__) || defined(__unix__)
            if (g_tasks[i].has_thread) {
                pthread_join(g_tasks[i].thread_handle, NULL);
                g_tasks[i].has_thread = 0;
            }
#endif
            if (g_tasks[i].rt != NULL) {
                extern void runtime_cleanup(RuntimeState *rt);
                runtime_cleanup(g_tasks[i].rt);
                free(g_tasks[i].rt);
            }
            if (g_tasks[i].mem != NULL) {
                mem_shutdown(g_tasks[i].mem);
                free(g_tasks[i].mem);
            }
            g_tasks[i].is_used = 0;
        }
    }

#ifdef _WIN32
    if (g_cs_initialized) {
        DeleteCriticalSection(&g_task_cs);
        g_cs_initialized = 0;
    }
#endif
}

int task_spawn(const char *filename, int line_num)
{
    int slot = -1;
    int i;
    for (i = 1; i < MAX_TASKS; i++) {
        if (!g_tasks[i].is_used) {
            slot = i;
            break;
        }
    }

    if (slot == -1) {
        error_raise(ERR_SORRY, line_num);
        return -1;
    }

    // Allocate memory system for process isolation
    MemorySystem *task_mem = (MemorySystem *)malloc(sizeof(MemorySystem));
    if (task_mem == NULL) {
        error_raise(ERR_SORRY, line_num);
        return -1;
    }

    if (mem_init(task_mem) != 0) {
        free(task_mem);
        error_raise(ERR_SORRY, line_num);
        return -1;
    }

    // Load file into task memory
    char path[260];
    strcpy(path, filename);
    // Ensure extension
    int flen = (int)strlen(path);
    if (flen < 4 || (strcmp(path + flen - 4, ".BAS") != 0 && strcmp(path + flen - 4, ".bas") != 0)) {
        strcat(path, ".BAS");
    }

    if (fileio_load(&task_mem->program, path) != 0) {
        mem_shutdown(task_mem);
        free(task_mem);
        printf("Task Spawn failed: file not found '%s'\n", path);
        error_raise(ERR_WHAT, line_num);
        return -1;
    }

    // Allocate runtime state
    RuntimeState *task_rt = (RuntimeState *)calloc(1, sizeof(RuntimeState));
    if (task_rt == NULL) {
        mem_shutdown(task_mem);
        free(task_mem);
        error_raise(ERR_SORRY, line_num);
        return -1;
    }

    // Initialize runtime
    runtime_init(task_rt, &task_mem->program, task_mem);
    
    // Copy options from main runtime
    RuntimeState *main_rt = g_tasks[0].rt;
    if (main_rt != NULL) {
        task_rt->screen_mode = main_rt->screen_mode;
        task_rt->screen_width = main_rt->screen_width;
        task_rt->screen_lines = main_rt->screen_lines;
    }

    task_rt->running = 1;
    task_rt->stopped = 0;
    task_rt->vm_state = 1; // VM_RUNNING

    // Fill task descriptor
    g_tasks[slot].pid = g_next_pid++;
    strcpy(g_tasks[slot].filename, filename);
    g_tasks[slot].rt = task_rt;
    g_tasks[slot].mem = task_mem;
    g_tasks[slot].state = TASK_RUNNING_BG;
    // Process isolation: auto-assign default isolated RAMBANK matching its PID
    g_tasks[slot].active_bank_id = g_tasks[slot].pid % 254 + 1;
    g_tasks[slot].is_used = 1;

    g_tasks[slot].is_threaded = 0;
#if defined(_WIN32)
    g_tasks[slot].thread_handle = CreateThread(
        NULL,
        0,
        task_thread_worker,
        &g_tasks[slot],
        0,
        &g_tasks[slot].thread_id
    );
    if (g_tasks[slot].thread_handle != NULL) {
        g_tasks[slot].is_threaded = 1;
    } else {
        printf("Warning: Thread creation failed, falling back to cooperative execution.\n");
    }
#elif defined(__linux__) || defined(__APPLE__) || defined(__unix__)
    g_tasks[slot].has_thread = 0;
    if (pthread_create(&g_tasks[slot].thread_handle, NULL, task_thread_worker, &g_tasks[slot]) == 0) {
        g_tasks[slot].is_threaded = 1;
        g_tasks[slot].has_thread = 1;
    } else {
        printf("Warning: Thread creation failed, falling back to cooperative execution.\n");
    }
#endif

    printf("Task %d spawned background process: %s (Memory Bank: %d, Threaded: %s)\n",
           g_tasks[slot].pid, path, g_tasks[slot].active_bank_id,
           g_tasks[slot].is_threaded ? "YES" : "NO");

    return g_tasks[slot].pid;
}

void task_list(void)
{
    printf("\nPID  FILE                 STATE (COLOR-CODED STATUS)\n");
    printf("---  -------------------  --------------------------------\n");
    int i;
    for (i = 0; i < MAX_TASKS; i++) {
        if (g_tasks[i].is_used) {
            const char *color = "\033[37m"; // White default
            const char *state_str = "RUNNING_BG";

            switch (g_tasks[i].state) {
                case TASK_RUNNING_FG:
                    color = "\033[32m"; // Green
                    state_str = "RUNNING_FG (Active)";
                    break;
                case TASK_WAITING:
                    color = "\033[33m"; // Yellow
                    state_str = "WAITING ON INPUT";
                    break;
                case TASK_RUNNING_BG:
                    color = "\033[37m"; // White
                    state_str = "RUNNING_BG";
                    break;
                case TASK_ERROR:
                    color = "\033[31m"; // Red
                    state_str = "HALTED ON RUNTIME ERROR";
                    break;
                case TASK_DONE:
                    color = "\033[34m"; // Blue
                    state_str = "DONE (Clean Exit)";
                    break;
                case TASK_DONE_ERR:
                    color = "\033[35m"; // Purple
                    state_str = "DONE (Non-Clean Exit)";
                    break;
            }
            printf("%-3d  %-19s  %s%s\033[0m\n",
                   g_tasks[i].pid, g_tasks[i].filename, color, state_str);
        }
    }
    printf("\n");
}

void task_switch(int target_pid, int line_num)
{
    if (target_pid == g_current_task_pid) {
        return; // Already current
    }

    int src_slot = -1;
    int dest_slot = -1;
    int i;
    for (i = 0; i < MAX_TASKS; i++) {
        if (g_tasks[i].is_used) {
            if (g_tasks[i].pid == g_current_task_pid) {
                src_slot = i;
            }
            if (g_tasks[i].pid == target_pid) {
                dest_slot = i;
            }
        }
    }

    if (dest_slot == -1) {
        printf("Task Switch failed: PID %d not found\n", target_pid);
        error_raise(ERR_HOW, line_num);
        return;
    }

    // Toggle states
    if (src_slot != -1) {
        if (g_tasks[src_slot].state == TASK_RUNNING_FG) {
            g_tasks[src_slot].state = TASK_RUNNING_BG;
        }
    }

    if (g_tasks[dest_slot].state == TASK_RUNNING_BG) {
        g_tasks[dest_slot].state = TASK_RUNNING_FG;
    }
    g_current_task_pid = target_pid;

    printf("Switched foreground context to Task %d (%s)\n", target_pid, g_tasks[dest_slot].filename);
}

BasicTask *task_get_current(void)
{
    int i;
    for (i = 0; i < MAX_TASKS; i++) {
        if (g_tasks[i].is_used && g_tasks[i].pid == g_current_task_pid) {
            return &g_tasks[i];
        }
    }
    return &g_tasks[0];
}

int task_has_background_active(void)
{
    int i;
    for (i = 1; i < MAX_TASKS; i++) {
        if (g_tasks[i].is_used &&
            (g_tasks[i].state == TASK_RUNNING_BG || g_tasks[i].state == TASK_RUNNING_FG || g_tasks[i].state == TASK_WAITING)) {
            return 1;
        }
    }
    return 0;
}

void task_mark_waiting(int waiting)
{
    int i;
    for (i = 0; i < MAX_TASKS; i++) {
        if (g_tasks[i].is_used && g_tasks[i].pid == g_current_task_pid) {
            if (waiting) {
                g_tasks[i].state = TASK_WAITING;
            } else {
                if (g_tasks[i].pid == 0) {
                    g_tasks[i].state = TASK_RUNNING_FG;
                } else {
                    g_tasks[i].state = (g_current_task_pid == g_tasks[i].pid) ? TASK_RUNNING_FG : TASK_RUNNING_BG;
                }
            }
            break;
        }
    }
}

// Tick the scheduler to run exactly one statement on the next ready background task
extern void exec_run_step_cooperative(RuntimeState *rt);

void task_scheduler_tick(void)
{
    static int last_ticked_slot = 0;

    // Scan for next background task to schedule
    int next_slot = -1;
    int i;
    for (i = 1; i <= MAX_TASKS; i++) {
        int idx = (last_ticked_slot + i) % MAX_TASKS;
        if (idx == 0) continue; // Skip foreground main task (handled by its own run thread)
        
        if (g_tasks[idx].is_used && g_tasks[idx].state == TASK_RUNNING_BG && g_tasks[idx].rt != NULL && !g_tasks[idx].is_threaded) {
            next_slot = idx;
            break;
        }
    }

    if (next_slot != -1) {
        last_ticked_slot = next_slot;
        RuntimeState *task_rt = g_tasks[next_slot].rt;

        // Run a single cooperative step for this task
        exec_run_step_cooperative(task_rt);

        // Check if task finished
        if (task_rt->stopped || task_rt->current_index >= task_rt->program->count) {
            g_tasks[next_slot].state = TASK_DONE;
            printf("Task %d completed execution.\n", g_tasks[next_slot].pid);
        } else if (task_rt->vm_state == 3) { // VM_ERROR
            g_tasks[next_slot].state = TASK_ERROR;
            printf("Task %d halted on error.\n", g_tasks[next_slot].pid);
        }
    }
}

MemorySystem *task_get_main_mem(void)
{
    return g_tasks[0].mem;
}

void task_kill(int pid, int line_num)
{
    (void)line_num;
    task_mutex_lock();
    int i;
    for (i = 1; i < MAX_TASKS; i++) {
        if (g_tasks[i].is_used && g_tasks[i].pid == pid) {
            if (g_tasks[i].state == TASK_RUNNING_BG || g_tasks[i].state == TASK_RUNNING_FG || g_tasks[i].state == TASK_WAITING) {
                g_tasks[i].state = TASK_DONE_ERR;
                if (g_tasks[i].rt != NULL) {
                    g_tasks[i].rt->running = 0;
                    g_tasks[i].rt->stopped = 1;
                }
                printf("Task %d killed.\n", pid);
            }
            break;
        }
    }
    task_mutex_unlock();
}

void task_join(int pid, RuntimeState *rt, int line_num)
{
    (void)rt;
    (void)line_num;
    int running = 1;
    while (running) {
        task_mutex_lock();
        int found = 0;
        for (int i = 1; i < MAX_TASKS; i++) {
            if (g_tasks[i].is_used && g_tasks[i].pid == pid) {
                if (g_tasks[i].state == TASK_RUNNING_BG || g_tasks[i].state == TASK_RUNNING_FG || g_tasks[i].state == TASK_WAITING) {
                    found = 1;
                }
                break;
            }
        }
        task_mutex_unlock();
        
        if (!found) {
            break;
        }
        
        BasicTask *curr = task_get_current();
        if (curr == NULL || curr->pid == 0) {
            task_scheduler_tick();
            platform_sleep_ms(1);
        } else {
            platform_sleep_ms(10);
        }
    }
}

int task_get_status(int pid)
{
    task_mutex_lock();
    int status = 0;
    for (int i = 0; i < MAX_TASKS; i++) {
        if (g_tasks[i].is_used && g_tasks[i].pid == pid) {
            status = (int)g_tasks[i].state + 1;
            break;
        }
    }
    task_mutex_unlock();
    return status;
}
#endif

