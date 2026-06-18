/*
 * ---
 * BASIC++ Interpreter - vfs.c
 * ---
 *
 * Virtual Filesystem implementation.
 *
 * Mount table: 8 slots mapping virtual drive prefixes
 * (e.g. "A:", "USB:") to real filesystem paths.
 *
 * VPATH: Search path for file reads, using platform-
 * aware delimiters (';' on Windows, ':' on Linux).
 *
 * vfs_resolve() is the single entry point for all
 * path resolution. It checks mounts first, then VPATH
 * for read operations, then passes through as-is.
 *
 * ---
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "vfs.h"
#include "errors.h"

#ifdef _WIN32
#include <io.h>
#define VFS_ACCESS(f) (_access((f), 0) == 0)
#else
#include <unistd.h>
#define VFS_ACCESS(f) (access((f), F_OK) == 0)
#endif

/* --- Static State --- */

static VFSMount vfs_mounts[VFS_MAX_MOUNTS];
static char vfs_vpath[VFS_MAX_VPATH];

/* --- Helpers --- */

/*
 * str_icmp_n - Case-insensitive prefix comparison.
 * Returns 1 if a[0..n-1] matches b[0..n-1], 0 otherwise.
 */
static int str_icmp_n(const char *a, const char *b, int n)
{
 for (int i = 0; i < n; i++) {
  char ca = a[i];
  char cb = b[i];
  if (ca >= 'a' && ca <= 'z')
   ca = (char)(ca - 32);
  if (cb >= 'a' && cb <= 'z')
   cb = (char)(cb - 32);
  if (ca != cb) return 0;
 }
 return 1;
}

/*
 * has_trailing_colon - Check if string ends with ':'.
 */
static int has_trailing_colon(const char *s, int len)
{
 return (len > 0 && s[len - 1] == ':');
}

/*
 * path_append - Concatenate dir + separator + file into out.
 * Returns 0 on success, -1 if buffer too small.
 */
static int path_append(const char *dir, int dlen,
 const char *file, int flen,
 char *out, int out_max)
{
 int total;
 bool need_sep = false;

 /* Check if dir needs a trailing separator */
 if (dlen > 0) {
  char last = dir[dlen - 1];
  if (last != '/' && last != '\\')
   need_sep = 1;
 }

 total = dlen + need_sep + flen;
 if (total >= out_max) return -1;

 memcpy(out, dir, (size_t)dlen);
 if (need_sep) {
#ifdef _WIN32
  out[dlen] = '\\';
#else
  out[dlen] = '/';
#endif
 }
 memcpy(out + dlen + need_sep, file, (size_t)flen);
 out[total] = '\0';
 return 0;
}

/* --- Public API --- */

void vfs_init(void)
{
 for (int i = 0; i < VFS_MAX_MOUNTS; i++) {
  vfs_mounts[i].prefix[0] = '\0';
  vfs_mounts[i].target[0] = '\0';
  vfs_mounts[i].active = 0;
 }
 vfs_vpath[0] = '\0';
}

int vfs_mount(const char *prefix, const char *target,
 int line_num)
{
 int plen, tlen, i;

 if (prefix == NULL || target == NULL) {
  error_raise(ERR_WHAT, line_num);
  return -1;
 }

 plen = (int)strlen(prefix);
 tlen = (int)strlen(target);

 /* Prefix must end with ':' (GW-BASIC/QBasic convention) */
 if (!has_trailing_colon(prefix, plen)) {
  printf("Mount prefix must end with ':' "
         "(e.g. \"A:\")\n");
  error_raise(ERR_WHAT, line_num);
  return -1;
 }

 /* Validate lengths */
 if (plen >= VFS_MAX_PREFIX) {
  printf("Mount prefix too long (max %d)\n",
         VFS_MAX_PREFIX - 1);
  error_raise(ERR_HOW, line_num);
  return -1;
 }
 if (tlen >= VFS_MAX_TARGET) {
  printf("Mount target path too long (max %d)\n",
         VFS_MAX_TARGET - 1);
  error_raise(ERR_HOW, line_num);
  return -1;
 }

 /* Check if prefix is already mounted */

 for (i = 0; i < VFS_MAX_MOUNTS; i++) {
  if (vfs_mounts[i].active &&
      str_icmp_n(vfs_mounts[i].prefix,
                 prefix, plen) &&
      vfs_mounts[i].prefix[plen] == '\0') {
   /* Update existing mount */
   memcpy(vfs_mounts[i].target, target,
          (size_t)(tlen + 1));
   return 0;
  }
 }

 /* Find a free slot */
 for (int i = 0; i < VFS_MAX_MOUNTS; i++) {
  if (!vfs_mounts[i].active) {
   memcpy(vfs_mounts[i].prefix, prefix,
          (size_t)(plen + 1));
   memcpy(vfs_mounts[i].target, target,
          (size_t)(tlen + 1));
   vfs_mounts[i].active = 1;
   return 0;
  }
 }

 printf("No free mount slots (max %d)\n",
        VFS_MAX_MOUNTS);
 error_raise(ERR_SORRY, line_num);
 return -1;
}

int vfs_umount(const char *prefix, int line_num)
{
 int plen, i;

 if (prefix == NULL) {
  error_raise(ERR_WHAT, line_num);
  return -1;
 }

 plen = (int)strlen(prefix);

 for (int i = 0; i < VFS_MAX_MOUNTS; i++) {
  if (vfs_mounts[i].active &&
      str_icmp_n(vfs_mounts[i].prefix,
                 prefix, plen) &&
      vfs_mounts[i].prefix[plen] == '\0') {
   vfs_mounts[i].active = 0;
   vfs_mounts[i].prefix[0] = '\0';
   vfs_mounts[i].target[0] = '\0';
   return 0;
  }
 }

 printf("Mount not found: %s\n", prefix);
 error_raise(ERR_HOW, line_num);
 return -1;
}

void vfs_list_mounts(void)
{
 bool found = false;

 printf("\nVirtual Filesystem Mounts:\n");
 printf("%-12s  %s\n", "PREFIX", "TARGET");
 printf("%-12s  %s\n", "------", "------");

 for (int i = 0; i < VFS_MAX_MOUNTS; i++) {
  if (vfs_mounts[i].active) {
   printf("%-12s  %s\n",
          vfs_mounts[i].prefix,
          vfs_mounts[i].target);
   found++;
  }
 }

 if (found == 0) {
  printf("(none)\n");
 }
 printf("\n");
}

/*
 * vfs_resolve - Resolve a virtual path to a real path.
 *
 * Step 1: Check mount table. If the path starts with a
 *         mounted prefix (case-insensitive), replace the
 *         prefix with the target path.
 *
 * Step 2: For read operations only, if the file doesn't
 *         exist at the resolved path, try each directory
 *         in the VPATH search path.
 *
 * Step 3: If nothing matched, copy the path as-is.
 */
int vfs_resolve(const char *path, char *out,
 int out_max, int for_write)
{
 int plen;
 if (path == NULL || out == NULL || out_max < 2) {
  if (out && out_max > 0) out[0] = '\0';
  return -1;
 }

 plen = (int)strlen(path);

 /* --- Step 1: Mount prefix resolution --- */
 for (int i = 0; i < VFS_MAX_MOUNTS; i++) {
  int mlen;
  if (!vfs_mounts[i].active) continue;
  mlen = (int)strlen(vfs_mounts[i].prefix);
  if (plen >= mlen &&
      str_icmp_n(path,
                 vfs_mounts[i].prefix, mlen)) {
   /* Match! Replace prefix with target. */
   const char *rest = path + mlen;
   int rlen = plen - mlen;
   int tlen = (int)strlen(vfs_mounts[i].target);

   if (rlen == 0) {
    /* Just the prefix, resolve to target dir */
    if (tlen >= out_max) return -1;
    memcpy(out, vfs_mounts[i].target,
           (size_t)(tlen + 1));
    return 0;
   }
   /* target + separator + rest */
   return path_append(
    vfs_mounts[i].target, tlen,
    rest, rlen, out, out_max);
  }
 }

 /* --- Step 2: VPATH search (reads only) --- */
 if (!for_write && vfs_vpath[0] != '\0') {
  /*
   * Only search VPATH for relative paths.
   * Skip if the path is absolute or has a drive letter.
   */
  bool is_abs = false;
#ifdef _WIN32
  /* Absolute: starts with \ or / or X:\ */
  if (path[0] == '\\' || path[0] == '/') is_abs = 1;
  if (plen >= 3 && path[1] == ':' &&
      (path[2] == '\\' || path[2] == '/'))
   is_abs = 1;
#else
  if (path[0] == '/') is_abs = 1;
#endif

  if (!is_abs) {
   /* First try the file as-is in CWD */
   if (VFS_ACCESS(path)) {
    if (plen >= out_max) return -1;
    memcpy(out, path, (size_t)(plen + 1));
    return 0;
   }

   /* Search each VPATH directory */
   {
    const char *vp = vfs_vpath;
    while (*vp != '\0') {
     char dir[VFS_MAX_TARGET];
     int dlen = 0;
     char trial[VFS_MAX_TARGET + 260];

     /* Extract next directory */
     while (*vp != '\0' &&
            *vp != VFS_PATH_DELIM) {
      if (dlen < VFS_MAX_TARGET - 1)
       dir[dlen++] = *vp;
      vp++;
     }
     dir[dlen] = '\0';
     if (*vp == VFS_PATH_DELIM) vp++;

     /* Skip empty entries */
     if (dlen == 0) continue;

     /* Build trial path: dir/filename */
     if (path_append(dir, dlen, path, plen,
                     trial,
                     (int)sizeof(trial))
         == 0) {
      if (VFS_ACCESS(trial)) {
       int tlen2 = (int)strlen(trial);
       if (tlen2 >= out_max) return -1;
       memcpy(out, trial,
              (size_t)(tlen2 + 1));
       return 0;
      }
     }
    }
   }
  }
 }

 /* --- Step 3: Passthrough --- */
 if (plen >= out_max) return -1;
 memcpy(out, path, (size_t)(plen + 1));
 return 0;
}

void vfs_set_vpath(const char *path)
{
 if (path == NULL || path[0] == '\0') {
  vfs_vpath[0] = '\0';
  return;
 }
 {
  int len = (int)strlen(path);
  if (len >= VFS_MAX_VPATH)
   len = VFS_MAX_VPATH - 1;
  memcpy(vfs_vpath, path, (size_t)len);
  vfs_vpath[len] = '\0';
 }
}

const char *vfs_get_vpath(void)
{
 return vfs_vpath;
}
