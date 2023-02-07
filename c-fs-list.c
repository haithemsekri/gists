
#include <time.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>

#undef L_DEBUG
#undef L_ERROR
#undef L_FATAL
#define L_DEBUG printf
#define L_ERROR printf
#define L_FATAL(...) printf(__VA_ARGS__); exit(-1)

#define inline_t __inline__ __attribute__((always_inline))
#define static_inline_t static inline_t
#define nonull_t(...) __attribute__((nonnull(__VA_ARGS__)))
#define restrict_t(...) __restrict __VA_ARGS__


#define FS_MAXPATH 256
#define FS_ENTRY_MAX_LEN FS_MAXPATH

char in_buff[4096 * 64] = {0};

int fs_dir_list_files_recursive(const char * path, char * buff, size_t * size);

int fs_dir_list_files_recursive(const char * path, char * buff, size_t * size)
{
  assert(path);
  assert(*path == '/');
  assert(buff);
  assert(size);
  assert(*size);

  const size_t maxSize = *size;
  *size = 0;
  *buff = 0;
  L_DEBUG("%lu %lu %s\n", maxSize, (uintptr_t)buff - (uintptr_t)in_buff, path);
  int reg_fs_cnt = 0;
  DIR * dir = opendir(path);
  if (!dir) {
     L_ERROR("opendir: operation failed opendir(%s) (errno == %s)\n", path, strerror(errno));
     return -errno;
  } else {
    struct dirent * entry;
    while (((entry = readdir(dir)) != NULL) && (*size < maxSize)) {
      if (entry->d_name[0] && (entry->d_name[0] != '.')) {
        if(DT_REG == entry->d_type) {
          int bRead = snprintf (buff + *size, maxSize - *size, "%s/%s", path, entry->d_name) ;
          if (bRead > 0) {
            bRead++;
            *size += (unsigned) bRead;
            reg_fs_cnt++;
          }
        } else if(DT_DIR == entry->d_type) {
          char sPath[FS_MAXPATH];
          size_t nSize = maxSize - *size;
          int bRead = snprintf (sPath, sizeof(sPath) - 1, "%s/%s", path, entry->d_name);
          if (bRead > 0) {
            int fret = fs_dir_list_files_recursive(sPath, buff + *size, &nSize);
            if ((fret > 0) && nSize) {
              reg_fs_cnt += fret;
              *size += nSize;
            }
          }
        }
      }
    }
    closedir(dir);
  }
  return reg_fs_cnt;
}

typedef void (*fs_entry_cb_t) (void * ctx, size_t type, const char * path, size_t len);
ssize_t fs_dir_list_files_recursive_cb(void * ctx, const size_t level, const char * path, fs_entry_cb_t cbk);

ssize_t fs_dir_list_files_recursive_cb(void * ctx, const size_t level, const char * path, fs_entry_cb_t cbk)
{
  assert(path);
  assert(*path == '/');
  assert(cbk);

  size_t path_len = strlen(path);
  char sPath[path_len + FS_ENTRY_MAX_LEN + 1];
  memcpy(sPath, path, path_len);
  if (sPath[path_len - 1] != '/') {
    sPath[path_len++] = '/';
  }
  sPath[path_len] = 0;

  ssize_t reg_fs_cnt = 0;
  DIR * dir = opendir(path);
  if (!dir) {
     L_ERROR("opendir: operation failed opendir(%s) (errno == %s)\n", path, strerror(errno));
     return -errno;
  } else {
    struct dirent * entry;
    while ((entry = readdir(dir))) {
      if (entry->d_name[0] && (entry->d_name[0] != '.')) {
        reg_fs_cnt++;
        strncpy(sPath + path_len, entry->d_name, FS_ENTRY_MAX_LEN);
        cbk(ctx, entry->d_type, sPath, path_len + strlen(entry->d_name));
        if ((level > 0) && (DT_DIR == entry->d_type)) {
          ssize_t fret = fs_dir_list_files_recursive_cb(ctx, level - 1, sPath, cbk);
          if (fret > 0) {
            reg_fs_cnt += fret;
          }
        }
      }
    }
    closedir(dir);
  }
  return reg_fs_cnt;
}

static_inline_t const char * fs_type_to_str(size_t type)
{
  switch (type) {
    case DT_FIFO: return "DT_FIFO";
    case DT_CHR:  return "DT_CHR";
    case DT_DIR:  return "DT_DIR";
    case DT_BLK:  return "DT_BLK";
    case DT_REG:  return "DT_REG";
    case DT_LNK:  return "DT_LNK";
    case DT_SOCK: return "DT_SOCK";
    case DT_WHT:  return "DT_WHT";
    default:      return "DT_UNKNOWN";
  }
}


int main(int argc, char * argv[])
{
  (void) argc;

  inline_t void list_fs_cbk (void * ctx, size_t type, const char * path, size_t len) {
    (void) ctx;
    (void) len;
    L_DEBUG("%s: %s\n", fs_type_to_str(type), path);
  }
  ssize_t ret = fs_dir_list_files_recursive_cb(NULL, (size_t) atol(argv[1]), argv[2], list_fs_cbk);

  L_DEBUG("ret (%ld)\n", ret);
  return 0;
}

#if 0

int main(int argc, char * argv[])
{
  (void) argc;

  size_t in_buff_len = sizeof(in_buff) - 1;
  int ret = fs_dir_list_files_recursive(argv[1], in_buff, &in_buff_len);
  L_DEBUG("ret (%d)\n", ret);

  char * s = in_buff;
  while (*s && (s < (in_buff + sizeof(in_buff)))) {
    L_DEBUG("%s\n", s);
    s += strlen(s) + 1;
    ret--;
  }
  L_DEBUG("ret (%d)\n", ret);
  return 0;
}
gcc dir_list.c -o dir_list.bin -Wall -Wextra  -fstack-protector-strong -D_FORTIFY_SOURCE=2 -Wshift-count-overflow -Wsizeof-array-argument   -Wall -Wextra -Werror -Warray-bounds=2 -feliminate-unused-debug-types -Wformat -Wformat-security -Werror=format-security -O2 -ffile-prefix-map=$PWD=.    -fdiagnostics-show-option -Wfloat-equal -Wconversion -Wlogical-op -Wundef -Wredundant-decls -Wshadow -Wstrict-overflow=2 -Wwrite-strings   -Wpointer-arith -Wcast-qual -Wformat=2 -Wformat-truncation -Wmissing-include-dirs -Wswitch-enum -Wdisabled-optimization -Winline -Winvalid-pch   -Wmissing-declarations -Wdouble-promotion -Wshadow -Wvector-operation-performance -Wnull-dereference -Wduplicated-cond -Wshift-overflow=2   -Wnull-dereference -Wduplicated-cond -Wcast-function-type   -Wmissing-declarations -Wmissing-field-initializers -Wunreachable-code -Wpointer-arith
#endif
