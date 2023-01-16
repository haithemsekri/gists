#if 0
$CXX   fs.cc gnss.cc main.cc dbus.cc  ntp.cc  rtc.cc  pollfd.cc app.cc  com_actia_Power_Diagnostic.cc  -o main.cc.bin  -lsystemd -lacu6-pro-lv-common ../toolchain/sysroots/aarch64-acu6-linux/usr/lib/libactia-common-cxx.so
g++ -fstack-protector-strong -D_FORTIFY_SOURCE=2 -Wformat -Wformat-security -Werror=format-security -Og -ggdb -funwind-tables -feliminate-unused-debug-types -Wl,--hash-style=gnu -Wl,-Og -Wl,--as-needed -Wl,-z,relro,-z,now -Wall -Wextra -pedantic 

#endif

#include <sys/stat.h>
#include <inttypes.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <sys/timerfd.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <poll.h>
#include <sys/signalfd.h>
#include <math.h>
#include <limits.h>


#define L_DEBUG printf
#define L_ERROR printf
#define L_FATAL(...) printf(__VA_ARGS__); exit(-1)

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define AT __FILE__ ":" TOSTRING(__LINE__) ":"

#define inline_t __inline__ __attribute__((always_inline))
#define static_inline_t static inline_t
#define nonull_t(...) __attribute__((nonnull(__VA_ARGS__)))
#define restrict_t(...) __restrict __VA_ARGS__
#define SIZE_OF(p) (sizeof(p)/sizeof(p[0]))

#undef assert
#define assert(expr, ...) \
  ((void) sizeof ((expr) ? 1 : 0), __extension__ ({ \
  if (expr)	{ \
    ; /* empty */ \
  } \
  else { \
    (void) fprintf(stderr, AT  "Assertion(" #expr ") failed\n" __VA_ARGS__); \
    abort();	\
  } \
  }))

#define assert_return(r, expr, ...) \
  do { if (!(expr)) { \
    (void) fprintf(stderr, AT  "Assertion(" #expr ") failed\n" __VA_ARGS__); \
    return r;	\
  }} while(0)

size_t rdtsc(){
    unsigned int lo,hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((size_t)hi << 32) | lo;
}

#define MAGIC_BYTE 153
#define ZERO_BYTE  0xFF

#define BUF_LEN_TYPE unsigned 
#define MAX_BUF_LEN UINT16_MAX

#define MEMBLOCK_RES0_BITS 8
#define MEMBLOCK_NUSE_BITS 8
#define MEMBLOCK_SIZE_BITS 16
#define MEMBLOCK_VPTR_BITS 64
#define MEMBLOCK_DATA_BITS 8


typedef struct __attribute__((packed)) {
  uint8_t res0;
  uint8_t nuse;
  uint16_t size;
  uintptr_t vptr;
  uint8_t data;
} memblock_t;

#define MEM_BLOCKS_MAX_LEN 64

memblock_t imemblock_list[MEM_BLOCKS_MAX_LEN];

inline_t void * _malloc(size_t l)
{
  assert_return(NULL, l);
  assert_return(NULL, l <= MAX_BUF_LEN);

  BUF_LEN_TYPE nl = 13 + l;
  uint8_t * p = (unsigned char *) malloc(nl);
  assert_return(NULL, p);
  p[0] = MAGIC_BYTE; // .res0, first byte for sanity check
  p[1] = 0; // .nuse
  *(uint16_t *)  &p[2] = l; // .size
  *(uintptr_t *) &p[4] = (uintptr_t)&p[12]; // .vptr
  p[12] = 0; // first byte of data worth a reset
  p[nl - 1] = MAGIC_BYTE; // .data, last byte for sanity check
  return (void *) &p[12];
}

inline_t int _xfree(void * ptr, bool dry)
{
  assert_return(-1, ptr);
  
  uint8_t * p = (uint8_t *) ((uintptr_t) ptr - 12);
  assert_return(-1, *(uintptr_t *) &p[4] == (uintptr_t) ptr);
  assert_return(-1, *(uint16_t  *) &p[2] <= MAX_BUF_LEN);
  assert_return(-1, *(uint16_t  *) &p[2] > 0);
  assert_return(-1, p[1] == 0);
  assert_return(-1, p[0] == MAGIC_BYTE);
  assert_return(-1, p[13 - 1 + *(uint16_t  *) &p[2]] == MAGIC_BYTE);
  free(p);
  return 0;
}

int main (void)
{
  size_t cyc1 = rdtsc();

  for (size_t i = 0; i < UINT16_MAX * 1024; i++) {
    _xfree(_malloc(1), false);
    _xfree(_malloc(128), false);
    _xfree(_malloc(4096), false);
/*
    free(malloc(1));
    free(malloc(128));
    free(malloc(4096));
    */
  }
  L_DEBUG("cyc2 - cyc1 %ld\n", rdtsc() - cyc1);
  return 0;
}