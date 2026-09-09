// FILENAME: stmt_mutex.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_dispatch.c)
// NEEDS: libengine (eval.h, lexer.h, stmt_mutex.h, vm.h)
// Provides runtime implementation for MUTEX concurrency statement and functions.
//
// ---- Includes ----

#include "statements/system/stmt_mutex.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "eval/eval.h"
#include "types/errors.h"

#define MAX_MUTEXES 64

typedef struct {
    bool active;
    bool locked;
    char name[64];
} VirtualMutex;

static VirtualMutex g_mutexes[MAX_MUTEXES];
static bool g_mutexes_init = false;

static void mutex_subsystem_init(void) {
    if (!g_mutexes_init) {
        runtime_memset(g_mutexes, 0, sizeof(g_mutexes));
        g_mutexes_init = true;
    }
}

int mutex_create_or_get(const char *name) {
    mutex_subsystem_init();
    if (!name || !*name) name = "DEFAULT";

    for (int i = 0; i < MAX_MUTEXES; i++) {
        if (g_mutexes[i].active && runtime_strcasecmp(g_mutexes[i].name, name) == 0) {
            return i + 1;
        }
    }

    for (int i = 0; i < MAX_MUTEXES; i++) {
        if (!g_mutexes[i].active) {
            g_mutexes[i].active = true;
            g_mutexes[i].locked = false;
            runtime_strncpy(g_mutexes[i].name, name, 63);
            return i + 1;
        }
    }
    return 0; // Pool full
}

bool mutex_lock(int id) {
    mutex_subsystem_init();
    if (id < 1 || id > MAX_MUTEXES) return false;
    VirtualMutex *m = &g_mutexes[id - 1];
    if (!m->active) {
        m->active = true;
        m->locked = false;
        runtime_snprintf(m->name, sizeof(m->name), "MUTEX_%d", id);
    }
    m->locked = true;
    return true;
}

bool mutex_unlock(int id) {
    mutex_subsystem_init();
    if (id < 1 || id > MAX_MUTEXES) return false;
    VirtualMutex *m = &g_mutexes[id - 1];
    if (!m->active) {
        m->active = true;
        m->locked = false;
        runtime_snprintf(m->name, sizeof(m->name), "MUTEX_%d", id);
        return true;
    }
    m->locked = false;
    return true;
}

bool mutex_is_locked(int id) {
    mutex_subsystem_init();
    if (id < 1 || id > MAX_MUTEXES) return false;
    return g_mutexes[id - 1].active && g_mutexes[id - 1].locked;
}

static const LangDesc g_stmt_mutex_desc = {
    .name = "MUTEX",
    .category = "Concurrency",
    .syntax = "MUTEX action$, id | MUTEX \"LOCK\", id | MUTEX \"UNLOCK\", id",
    .description = "Manages virtual mutex acquisition and release for concurrency control.",
    .error_summary = "Error 5: Illegal Function Call",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

static const LangDesc g_fn_mutex_desc = {
    .name = "MUTEX",
    .category = "Concurrency",
    .syntax = "m = MUTEX(name$) | MUTEX_LOCK(m) | MUTEX_UNLOCK(m)",
    .description = "Creates, locks, or unlocks a virtual concurrency mutex.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_FUNCTION
};

void stmt_mutex_register(void) {
    lang_desc_register(&g_stmt_mutex_desc);
    lang_desc_register(&g_fn_mutex_desc);
}

BppError stmt_mutex_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    BValue arg1 = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    int m_id = 0;
    char action[32] = "LOCK";

    if (arg1.type == VAL_STRING && arg1.as.string) {
        const char *s = str_data(arg1.as.string);
        if (s) runtime_strncpy(action, s, sizeof(action) - 1);
        str_release(vm_get_str(vm), arg1.as.string);

        BppToken tok = lex_peek(lex);
        if (tok.type == TOK_COMMA) {
            lex_next(lex);
            BValue arg2 = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            m_id = (int)((arg2.type == VAL_NUMBER || arg2.type == VAL_INTEGER) ? arg2.as.number : 0);
        }
    } else {
        m_id = (int)arg1.as.number;
    }

    if (runtime_strcasecmp(action, "UNLOCK") == 0) {
        mutex_unlock(m_id);
    } else if (runtime_strcasecmp(action, "INIT") == 0) {
        if (m_id >= 1 && m_id <= MAX_MUTEXES) {
            VirtualMutex *m = &g_mutexes[m_id - 1];
            m->active = true;
            m->locked = false;
            runtime_snprintf(m->name, sizeof(m->name), "MUTEX_%d", m_id);
        }
    } else {
        mutex_lock(m_id);
    }

    return err;
}

BValue func_mutex_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    (void)uname;
    (void)err;
    BValue res = { .type = VAL_NUMBER, .as.number = 0.0 };
    if (arg_count < 1 || !args) return res;

    if (args[0].type == VAL_STRING && args[0].as.string) {
        const char *name = str_data(args[0].as.string);
        res.as.number = (double)mutex_create_or_get(name);
        return res;
    }

    int id = (int)((args[0].type == VAL_NUMBER || args[0].type == VAL_INTEGER) ? args[0].as.number : 0);
    if (arg_count >= 2 && args[1].type == VAL_STRING && args[1].as.string) {
        const char *action = str_data(args[1].as.string);
        if (action && runtime_strcasecmp(action, "UNLOCK") == 0) {
            res.as.number = mutex_unlock(id) ? 1.0 : 0.0;
        } else if (action && runtime_strcasecmp(action, "INIT") == 0) {
            if (id >= 1 && id <= MAX_MUTEXES) {
                VirtualMutex *m = &g_mutexes[id - 1];
                m->active = true;
                m->locked = false;
                runtime_snprintf(m->name, sizeof(m->name), "MUTEX_%d", id);
                res.as.number = 1.0;
            } else {
                res.as.number = 0.0;
            }
        } else {
            res.as.number = mutex_lock(id) ? 1.0 : 0.0;
        }
    } else {
        res.as.number = mutex_lock(id) ? 1.0 : 0.0;
    }
    return res;
}

BValue func_mutex_lock_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    (void)uname;
    (void)err;
    BValue res = { .type = VAL_NUMBER, .as.number = 0.0 };
    if (arg_count < 1 || !args) return res;
    int id = (int)((args[0].type == VAL_NUMBER || args[0].type == VAL_INTEGER) ? args[0].as.number : 0);
    res.as.number = mutex_lock(id) ? 1.0 : 0.0;
    return res;
}

BValue func_mutex_unlock_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    (void)uname;
    (void)err;
    BValue res = { .type = VAL_NUMBER, .as.number = 0.0 };
    if (arg_count < 1 || !args) return res;
    int id = (int)((args[0].type == VAL_NUMBER || args[0].type == VAL_INTEGER) ? args[0].as.number : 0);
    res.as.number = mutex_unlock(id) ? 1.0 : 0.0;
    return res;
}
