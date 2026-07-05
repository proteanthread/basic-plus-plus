#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "module.h"
#include "security.h"
#include "vdev.h"
#include "builtins.h"
#include "runtime.h"
#include "errors.h"
#include "io/vfs.h"
#include "funcreg.h"
#include "lexer.h"

#if defined(_WIN32) || defined(__WATCOMC__)
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

static void ensure_dir_exists(const char *dir)
{
#if defined(_WIN32) || defined(__WATCOMC__)
    _mkdir(dir);
#else
    mkdir(dir, 0755);
#endif
}

static int persist_init(void *rt);
static void persist_cleanup(void);

static ModuleInfo persist_module_info = {
    "PERSIST",
    "1.0",
    "Persistent storage variables and virtual device",
    MOD_EXTENSION,
    CAP_IO | CAP_FILE,
    persist_init,
    persist_cleanup
};

// PSTORE(key$, val) - store persistent variable (key$ is string, val is string or number)
static BValue builtin_pstore(BValue *args, int argc, void *rt)
{
    (void)rt;
    if (argc < 2) {
        error_raise(ERR_WHAT, 0);
        return bval_int(-1);
    }
    BValue key_val = args[0];
    BValue data_val = args[1];
    if (key_val.type != VAL_STRING) {
        error_raise(ERR_WHAT, 0);
        return bval_int(-1);
    }
    
    char path[512];
    char key_buf[128];
    int klen = key_val.v.sval.length;
    if (klen >= (int)sizeof(key_buf)) klen = sizeof(key_buf) - 1;
    memcpy(key_buf, key_val.v.sval.data, (size_t)klen);
    key_buf[klen] = '\0';
    
    // Sanitize key name to prevent directory traversal
    for (int i = 0; i < klen; i++) {
        if (!isalnum((unsigned char)key_buf[i]) && key_buf[i] != '_') {
            key_buf[i] = '_';
        }
    }
    
    ensure_dir_exists("vfs");
    ensure_dir_exists("vfs/persist");
    
    sprintf(path, "vfs/persist/%s.dat", key_buf);
    
    char resolved[512];
    if (vfs_resolve(path, resolved, sizeof(resolved), 1) != 0) {
        error_raise(ERR_HOW, 0);
        return bval_int(-1);
    }
    printf("[DEBUG PSTORE] path='%s' resolved='%s'\n", path, resolved);
    
    FILE *fp = fopen(resolved, "wb");
    if (fp == NULL) {
        error_raise(ERR_HOW, 0);
        return bval_int(-1);
    }
    
    if (data_val.type == VAL_STRING) {
        fprintf(fp, "S:%.*s", data_val.v.sval.length, data_val.v.sval.data);
    } else if (data_val.type == VAL_FLOAT) {
        fprintf(fp, "F:%f", data_val.v.fval);
    } else {
        fprintf(fp, "I:%ld", data_val.v.ival);
    }
    fclose(fp);
    
    return bval_int(0);
}

// PRETRIEVE(key$) / PRETRIEVE$(key$) - retrieve persistent variable
static BValue builtin_pretrieve(BValue *args, int argc, void *rt)
{
    RuntimeState *state = (RuntimeState *)rt;
    if (argc < 1 || args[0].type != VAL_STRING) {
        error_raise(ERR_WHAT, 0);
        return bval_string(NULL, 0);
    }
    BValue key_val = args[0];
    char path[512];
    char key_buf[128];
    int klen = key_val.v.sval.length;
    if (klen >= (int)sizeof(key_buf)) klen = sizeof(key_buf) - 1;
    memcpy(key_buf, key_val.v.sval.data, (size_t)klen);
    key_buf[klen] = '\0';
    
    for (int i = 0; i < klen; i++) {
        if (!isalnum((unsigned char)key_buf[i]) && key_buf[i] != '_') {
            key_buf[i] = '_';
        }
    }
    
    sprintf(path, "vfs/persist/%s.dat", key_buf);
    
    char resolved[512];
    if (vfs_resolve(path, resolved, sizeof(resolved), 0) != 0) {
        return bval_string(NULL, 0);
    }
    printf("[DEBUG PRETRIEVE] path='%s' resolved='%s'\n", path, resolved);
    
    FILE *fp = fopen(resolved, "rb");
    if (fp == NULL) {
        printf("[DEBUG PRETRIEVE] fopen failed for '%s'\n", resolved);
        return bval_string(NULL, 0);
    }
    
    char type_char = (char)fgetc(fp);
    fgetc(fp); // skip ':'
    
    char val_buf[1024];
    int len = 0;
    int ch;
    while ((ch = fgetc(fp)) != EOF && len < (int)sizeof(val_buf) - 1) {
        val_buf[len++] = (char)ch;
    }
    val_buf[len] = '\0';
    fclose(fp);
    
    if (type_char == 'S') {
        char *pooled = strpool_alloc(&state->strpool, len);
        if (pooled != NULL) {
            memcpy(pooled, val_buf, (size_t)len);
        }
        return bval_string(pooled, len);
    } else if (type_char == 'F') {
        double fval = strtod(val_buf, NULL);
        return bval_float(fval);
    } else {
        long ival = strtol(val_buf, NULL, 10);
        return bval_int(ival);
    }
}

// === Virtual Device "PERSIST:" implementation ===
typedef struct {
    char current_key[128];
    int is_open;
    int write_mode; // 1 = output, 0 = input
} PersistDevState;

static PersistDevState dev_state;

static int dev_persist_open(VDev *d, const char *path, const char *mode)
{
    (void)d;
    // Extract key name from path (e.g. "PERSIST:my_key" -> "my_key")
    const char *key = path;
    if (strncmp(key, "PERSIST:", 8) == 0) {
        key += 8;
    }
    int len = (int)strlen(key);
    if (len >= (int)sizeof(dev_state.current_key)) {
        len = sizeof(dev_state.current_key) - 1;
    }
    memcpy(dev_state.current_key, key, (size_t)len);
    dev_state.current_key[len] = '\0';
    
    // Sanitize key name
    for (int i = 0; i < len; i++) {
        if (!isalnum((unsigned char)dev_state.current_key[i]) && dev_state.current_key[i] != '_') {
            dev_state.current_key[i] = '_';
        }
    }
    
    dev_state.is_open = 1;
    dev_state.write_mode = (mode[0] == 'w' || mode[0] == 'a' || mode[0] == 'o');
    return 0;
}

static int dev_persist_close(VDev *d)
{
    (void)d;
    dev_state.is_open = 0;
    dev_state.current_key[0] = '\0';
    return 0;
}

static int dev_persist_puts(VDev *d, const char *s)
{
    (void)d;
    if (!dev_state.is_open || !dev_state.write_mode || dev_state.current_key[0] == '\0') {
        return -1;
    }
    
    ensure_dir_exists("vfs");
    ensure_dir_exists("vfs/persist");
    
    char path[512];
    sprintf(path, "vfs/persist/%s.dat", dev_state.current_key);
    
    char resolved[512];
    if (vfs_resolve(path, resolved, sizeof(resolved), 1) != 0) {
        return -1;
    }
    
    FILE *fp = fopen(resolved, "wb");
    if (fp == NULL) {
        return -1;
    }
    
    fprintf(fp, "S:%s", s);
    fclose(fp);
    return 0;
}

static int dev_persist_gets(VDev *d, char *buf, int max)
{
    (void)d;
    if (!dev_state.is_open || dev_state.write_mode || dev_state.current_key[0] == '\0') {
        return -1;
    }
    
    char path[512];
    sprintf(path, "vfs/persist/%s.dat", dev_state.current_key);
    
    char resolved[512];
    if (vfs_resolve(path, resolved, sizeof(resolved), 0) != 0) {
        return -1;
    }
    
    FILE *fp = fopen(resolved, "rb");
    if (fp == NULL) {
        return -1;
    }
    
    fgetc(fp); // skip type char
    fgetc(fp); // skip ':'
    
    int len = 0;
    int ch;
    while ((ch = fgetc(fp)) != EOF && len < max - 1) {
        buf[len++] = (char)ch;
    }
    buf[len] = '\0';
    fclose(fp);
    return len;
}

static VDev persist_vdev;

static int persist_init(void *rt)
{
    (void)rt;
    // Initialize virtual device
    memset(&persist_vdev, 0, sizeof(persist_vdev));
    persist_vdev.name = "PERSIST:";
    persist_vdev.dev_puts = dev_persist_puts;
    persist_vdev.dev_gets = dev_persist_gets;
    persist_vdev.dev_open = dev_persist_open;
    persist_vdev.dev_close = dev_persist_close;
    persist_vdev.dev_class = VDCLASS_FILE;
    persist_vdev.dev_caps = VDCAP_READ | VDCAP_WRITE | VDCAP_STREAM;
    persist_vdev.dev_version = "1.0";
    persist_vdev.dev_description = "Persistent key-value virtual device";
    persist_vdev.dev_req_caps = CAP_FILE;

    static FunctionEntry pstore_entry = { 
        "PSTORE", 
        KW_PSTORE, 
        FCAT_IO, 
        FRET_INT, 
        2, 
        2, 
        FSAFE_IO, 
        0, 
        builtin_pstore, 
        "Store persistent variable: PSTORE(\"name\", value)" 
    };
    static FunctionEntry pretrieve_entry = { 
        "PRETRIEVE", 
        KW_PRETRIEVE, 
        FCAT_IO, 
        FRET_ANY, 
        1, 
        1, 
        FSAFE_IO, 
        0, 
        builtin_pretrieve, 
        "Retrieve persistent variable" 
    };
    static FunctionEntry pretrieve_str_entry = { 
        "PRETRIEVE$", 
        KW_PRETRIEVE_STR, 
        FCAT_IO, 
        FRET_STRING, 
        1, 
        1, 
        FSAFE_IO, 
        0, 
        builtin_pretrieve, 
        "Retrieve persistent variable as string" 
    };
    
    funcreg_register(&pstore_entry);
    funcreg_register(&pretrieve_entry);
    funcreg_register(&pretrieve_str_entry);
    
    vdev_register(&persist_vdev);
    return 0;
}

static void persist_cleanup(void)
{
    dev_persist_close(NULL);
}

void mod_persist_register(void)
{
    module_register(&persist_module_info);
}
