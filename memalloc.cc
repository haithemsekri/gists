#if 0
$CXX   fs.cc gnss.cc main.cc dbus.cc  ntp.cc  rtc.cc  pollfd.cc app.cc  com_actia_Power_Diagnostic.cc  -o main.cc.bin  -lsystemd -lacu6-pro-lv-common ../toolchain/sysroots/aarch64-acu6-linux/usr/lib/libactia-common-cxx.so
g++-11  -fstack-protector-strong -D_FORTIFY_SOURCE=2 -Wformat -Wformat-security -Werror=format-security -O2 -ggdb -funwind-tables -feliminate-unused-debug-types -Wl,--hash-style=gnu -Wl,-O2 -Wl,--as-needed -Wl,-z,relro,-z,now -Wall -Wextra -pedantic memalloc.cc -o memalloc.bin

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

#define ENTRY_LEN 13
#define IHEAP_ENTRIES 4
#define IHEAP_LEN (ENTRY_LEN * IHEAP_ENTRIES)
static uint8_t * s_iheap_start = NULL;
static uint8_t * s_iheap_end;
static uint8_t * s_iheap_max;
static uint8_t * s_last_p = NULL;

__attribute__((constructor))
static void _initialize(void) {
  if (!s_iheap_start) {
    s_iheap_start = (uint8_t *) malloc(IHEAP_LEN);
    assert(s_iheap_start);
    s_iheap_end = s_iheap_start + IHEAP_LEN;
    memset(s_iheap_start, ZERO_BYTE, IHEAP_LEN);
    s_iheap_max = s_iheap_end;
    s_last_p = s_iheap_start;
    L_DEBUG("s_iheap_start alloc %lx, %lx\n", (uintptr_t) s_iheap_start, (uintptr_t) s_iheap_end);
  }
}

__attribute__((destructor))
static void _destroy(void) {
  if (s_iheap_start) {
    free(s_iheap_start);
    s_iheap_start = NULL;
    L_DEBUG("s_iheap_start dealloc\n");
  }
}

void * _malloc(size_t l) __attribute_malloc__ __attribute_alloc_size__ ((1)) __wur;
void * _malloc(size_t l) 
{
  assert_return(NULL, l);
  assert_return(NULL, l <= MAX_BUF_LEN);

  if (l <= sizeof(uintptr_t)) {
    assert_return(NULL, s_iheap_start < s_iheap_end);
    
    //s_last_p = (NULL == s_last_p) ? s_iheap_start : s_last_p;
    uint8_t * p = s_last_p;
    while ((p < s_iheap_end) && !p[1]) {
      p += ENTRY_LEN;
    }
    if ((s_last_p > s_iheap_start) && (p >= s_iheap_end)) {
      p = s_iheap_start;
      while ((p < s_last_p) && !p[1]) {
        p += ENTRY_LEN;
      }
      assert_return(NULL, p < s_last_p);
    } else {
      assert_return(NULL, p < s_iheap_end);
    }
    assert_return(NULL, ZERO_BYTE == p[1]);
    //L_DEBUG("p - s_iheap_start [%ld] : %lx\n", l, ((uintptr_t)p - (uintptr_t)s_iheap_start));
    s_last_p = &p[ENTRY_LEN];
    s_last_p = (s_last_p + 1 >= s_iheap_end) ? s_iheap_start : s_last_p;

    p[0] = MAGIC_BYTE; // .res0, first byte for sanity check
    p[1] = 0; // .nuse
    *(uint16_t *)  &p[2] = l; // .size
    *(uintptr_t *) &p[4] = UINTPTR_MAX; // .vptr become the data
    p[ENTRY_LEN - 1] = MAGIC_BYTE; // .data, last byte for sanity check
    // L_DEBUG("_imalloc: Ptr(%ld) == %lx,%lx\n", l, (uintptr_t) &p[0], (uintptr_t) &p[4]);
    return (void *) &p[4];
  } else {
    BUF_LEN_TYPE nl = ENTRY_LEN + l;
    uint8_t * p = (unsigned char *) malloc(nl);
    assert_return(NULL, p);
    p[0] = MAGIC_BYTE; // .res0, first byte for sanity check
    p[1] = 0; // .nuse
    *(uint16_t *)  &p[2] = l; // .size
    *(uintptr_t *) &p[4] = (uintptr_t)&p[12]; // .vptr
    p[12] = UINT8_MAX; // first byte of data worth a reset
    p[nl - 1] = MAGIC_BYTE; // .data, last byte for sanity check
    // L_DEBUG("_bmalloc: Ptr(%ld) == %lx\n", l, (uintptr_t) &p[12]);
    if (&p[nl - 1] > s_iheap_max) {
      s_iheap_max = &p[nl - 1];
    }
    return (void *) &p[12];
  }
}

int _free(void * ptr);
int _free(void * ptr)
{
  assert_return(-1, NULL != ptr);
  
  if ((ptr < s_iheap_end) && (ptr > s_iheap_start)) {
    uint8_t * p = (uint8_t *) ((uintptr_t) ptr - 4);
    assert_return(-1, 0 == p[1] /* NotAllocated*/);
    assert_return(-1, *(uint16_t  *) &p[2] <= sizeof(uintptr_t));
    assert_return(-1, *(uint16_t  *) &p[2] > 0);
    assert_return(-1, MAGIC_BYTE == p[0]);
    assert_return(-1, p[ENTRY_LEN - 1] == MAGIC_BYTE);
    // L_DEBUG("_ifree  : Ptr(%d) == %lx,%lx\n", *(uint16_t  *) &p[2], (uintptr_t) p, (uintptr_t) ptr);
    for (size_t i = *(uint16_t  *) &p[2]; i < sizeof(uintptr_t); i++) {
      assert_return(-1, p[i + 4] == ZERO_BYTE);
    }
    memset(p, ZERO_BYTE, ENTRY_LEN);
  } else if ((ptr >= s_iheap_end) && (ptr < s_iheap_max)) {
    uint8_t * p = (uint8_t *) ((uintptr_t) ptr - 12);
    assert_return(-1, 0 == p[1] /* NotAllocated*/);
    assert_return(-1, *(uintptr_t *) &p[4] == (uintptr_t) ptr);
    assert_return(-1, *(uint16_t  *) &p[2] <= MAX_BUF_LEN);
    assert_return(-1, *(uint16_t  *) &p[2] > 0);
    assert_return(-1, p[0] == MAGIC_BYTE);
    assert_return(-1, p[ENTRY_LEN - 1 + *(uint16_t  *) &p[2]] == MAGIC_BYTE);
    // L_DEBUG("_bfree  : Ptr(%d) == %lx\n", *(uint16_t  *) &p[2], (uintptr_t) &p[12]);
    free(p);
  } else {
    L_DEBUG("OutOfrangePtr %lx != [%lx .. %lx .. %lx]\n", (uintptr_t) ptr, (uintptr_t) s_iheap_start, (uintptr_t) s_iheap_end, (uintptr_t) s_iheap_max);
    return -1;
  }
  return 0;
}

const void * xptr_valid(const void * ptr, size_t l)
{
  assert_return(NULL, NULL != ptr);
  
  if ((ptr < s_iheap_end) && (ptr > s_iheap_start)) {
    uint8_t * p = (uint8_t *) ((uintptr_t) ptr - 4);
    assert_return(NULL, 0 == p[1] /* NotAllocated*/);
    assert_return(NULL, *(uint16_t  *) &p[2] <= sizeof(uintptr_t));
    assert_return(NULL, *(uint16_t  *) &p[2] > 0);
    assert_return(NULL, l <= *(uint16_t  *) &p[2]);
    assert_return(NULL, MAGIC_BYTE == p[0]);
    assert_return(NULL, p[ENTRY_LEN - 1] == MAGIC_BYTE);
  } else if ((ptr >= s_iheap_end) && (ptr < s_iheap_max)) {
    uint8_t * p = (uint8_t *) ((uintptr_t) ptr - 12);
    assert_return(NULL, 0 == p[1] /* NotAllocated*/);
    assert_return(NULL, *(uintptr_t *) &p[4] == (uintptr_t) ptr);
    assert_return(NULL, *(uint16_t  *) &p[2] <= MAX_BUF_LEN);
    assert_return(NULL, *(uint16_t  *) &p[2] > 0);
    assert_return(NULL, l <= *(uint16_t  *) &p[2]);
    assert_return(NULL, p[0] == MAGIC_BYTE);
    assert_return(NULL, p[ENTRY_LEN - 1 + *(uint16_t  *) &p[2]] == MAGIC_BYTE);
  } else {
    L_DEBUG("OutOfrangePtr %lx != [%lx .. %lx .. %lx]\n", (uintptr_t) ptr, (uintptr_t) s_iheap_start, (uintptr_t) s_iheap_end, (uintptr_t) s_iheap_max);
    return NULL;
  }
  return ptr;
}


const void * xptr_assert(const void * ptr, uint8_t val, size_t pos, size_t len, const char * file, int line)
{
  (void) val;

  // L_DEBUG("%s:%d: xptr_assert(%lx[%ld + %ld])\n", file, line, (uintptr_t) ptr, pos, len);
  
  char * ret_ptr = (char *) xptr_valid(ptr, pos + len);
  if (!ret_ptr) {
    L_FATAL("%s:%d: OutOfrangePtrAccess %lx[%ld + %ld]\n", file, line, (uintptr_t) ptr, pos, len);
    return NULL;
  }
  return &ret_ptr[pos];
}

#if 0
#define ptr_valid(ptr, pos) (typeof((ptr))) xptr_valid(ptr, ((typeof((ptr))) p)[pos], pos * sizeof(*(ptr)), sizeof(*(ptr)))
#define ptr_assert(ptr, pos) (typeof((ptr))) xptr_assert(ptr, ((typeof((ptr))) p)[pos], pos * sizeof(*(ptr)), sizeof(*(ptr)), __FILE__, __LINE__)
#endif

#define wptr(_p, _pos) \
    ((typeof((_p))) xptr_assert(_p, ((uint8_t *)_p)[(_pos + 1) * sizeof(*_p) - 1], _pos * sizeof(*_p), sizeof(*_p), __FILE__, __LINE__))

#define rptr(_p, _pos) \
    ((const typeof((*_p)) *) xptr_assert(_p, ((uint8_t *)_p)[(_pos + 1) * sizeof(*_p) - 1], _pos * sizeof(*_p), sizeof(*_p), __FILE__, __LINE__))

int main (void)
{
  size_t cyc1 = rdtsc();

  
  for (size_t i = 0; i < UINT16_MAX * 1024; i++) {
    
    _free(_malloc(1));
    _free(_malloc(128));
    _free(_malloc(4096));

    /*_free(_malloc(1));
    _free(_malloc(128));
    _free(_malloc(4096));*/
  }

  L_DEBUG("cyc2 - cyc1 %ld\n", rdtsc() - cyc1);
  return 0;

  // _free((void *) &cyc1);

  //_free(_malloc(0));
/*
  _free(_malloc(1));
  _free(_malloc(5));
  _free(_malloc(6));
  _free(_malloc(7));
  _free(_malloc(7));
  _free(_malloc(8));
  _free(_malloc(9));
 _free(_malloc(128));
    void * p1 = _malloc(1);
    void * p2 = _malloc(2);
    void * p3 = _malloc(3);
    void * p4 = _malloc(4);
    //_free(p4);
    void * p5 = _malloc(5);
    _free(p1);
    _free(p2);
    _free(p3);
    _free(p4);
    _free(p5);
    */
/*
  {
    uint8_t * p = (uint8_t *) _malloc(2);
    *wptr(p, 0) = 0;
    *wptr(p, 1) = 0;
    *wptr((size_t *) p, 0) = 0;
    _free(p);
  }*/
  /*
  {
   
    memblock_t * p = (memblock_t *) _malloc(2 * sizeof(memblock_t));
    *wptr(p, 0) = *rptr(p, 1);
    *wptr((uint8_t *)p, 0) = *rptr((uint8_t *)p, 13);
    *wptr((uint8_t *)p, 0) = *rptr((uint8_t *)p, 25);

    // *wptr((uint8_t *)p, 0) = *rptr((uint8_t *)p, 26);
    // *wptr(p, 1) = *rptr(p, 2);
    // *wptr(p, 3) = *rptr(p, 4);

    _free(p);
  }*/
  {
    uint8_t * p = (uint8_t *) _malloc(128);
    p[2] = 0;

    *wptr(p, 0) = 0;
    *wptr(p, 127) = 0;
    *wptr((size_t *) p, 0) = 0;
    *wptr((size_t *) p, 15) = 0;
    // *wptr(p, 128) = 0;
    // *wptr((size_t *) p, 16) = 0;
    // *wptr((size_t *) p, 20) = 0;
    _free(p); 
    // _free(p);
  }

/*
  for (size_t i = 0; i < 128; i++) {
    void * p1 = _malloc(i);
    void * p2 = _malloc(i);
    void * p3 = _malloc(i);
    _free(p1);
    _free(p2);
    _free(p3);
  }

  for (size_t i = 0; i < UINT16_MAX * 1024; i++) {
    _free(_malloc(1));
    _free(_malloc(128));
    _free(_malloc(4096));
  }
*/
  L_DEBUG("cyc2 - cyc1 %ld\n", rdtsc() - cyc1);
  return 0;
}
