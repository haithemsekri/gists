
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

#define inline_t __inline__ __attribute__((always_inline))
#define static_inline_t static inline_t
#define nonull_t(...) __attribute__((nonnull(__VA_ARGS__)))
#define restrict_t(...) __restrict __VA_ARGS__
#define SIZE_OF(p) (sizeof(p)/sizeof(p[0]))

#define MAGIC_BYTE 153
#define ZERO_BYTE  0xFF

typedef struct __attribute__((packed)) {
  size_t res0 : 8;
  size_t not_used : 1;
  size_t bytes : 7;
  size_t val;
  size_t res1 : 8;
} memblock_t;

#define MEM_BLOCKS_MAX_LEN 64

memblock_t imemblock_list[MEM_BLOCKS_MAX_LEN];

memblock_t bmemblock_list[MEM_BLOCKS_MAX_LEN];

#define BUF_LEN_TYPE unsigned 
#define MAX_BUF_LEN UINT16_MAX

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define AT __FILE__ ":" TOSTRING(__LINE__) ":"
        


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

void * _malloc(size_t l)
{
  assert_return(NULL, l);
  assert_return(NULL, l <= MAX_BUF_LEN);

  unsigned i;
  
  if (l <= sizeof(size_t)) {
    for (i = 0; (i < MEM_BLOCKS_MAX_LEN) && !imemblock_list[i].not_used; i++)
    assert (i < MEM_BLOCKS_MAX_LEN);
    imemblock_list[i].not_used = 0;
    imemblock_list[i].bytes = l;
    imemblock_list[i].res0 = MAGIC_BYTE;
    imemblock_list[i].res1 = MAGIC_BYTE;
    // L_DEBUG("_malloc: imemblock_list[%d](%ld) == %lx\n", i, l, (uintptr_t) &imemblock_list[i].val);
    return &imemblock_list[i].val;
  } else {
    for (i = 0; (i < MEM_BLOCKS_MAX_LEN) && !bmemblock_list[i].not_used; i++)
    assert (i < MEM_BLOCKS_MAX_LEN);
    bmemblock_list[i].not_used = 0;
    bmemblock_list[i].bytes = 0;
    BUF_LEN_TYPE nl = sizeof(unsigned char) + sizeof(BUF_LEN_TYPE) + l + sizeof(unsigned char);
    unsigned char * p = (unsigned char *) malloc(nl);
    assert_return(NULL, p);
    bmemblock_list[i].val = (uintptr_t) &p[sizeof(unsigned char) + sizeof(BUF_LEN_TYPE)];
    bmemblock_list[i].res0 = MAGIC_BYTE;
    bmemblock_list[i].res1 = MAGIC_BYTE;
    p[0] = MAGIC_BYTE;
    p[nl - 1] = MAGIC_BYTE;
    * (BUF_LEN_TYPE *) &p[sizeof(unsigned char)] = l;
    // L_DEBUG("_malloc: bmemblock_list[%d](%ld) == %lx\n", i, l, (uintptr_t) bmemblock_list[i].val);
    return (void *) bmemblock_list[i].val;
  }
}

int _xfree(void * ptr, bool dry)
{
  assert_return(-1, ptr);
  
  if (((memblock_t *) ptr > &imemblock_list[0]) && ((memblock_t *) ptr < &imemblock_list[MEM_BLOCKS_MAX_LEN])) {
    size_t i = ((uintptr_t) ptr - (uintptr_t) imemblock_list) / sizeof(memblock_t);
    assert_return(-1, i < MEM_BLOCKS_MAX_LEN);
    assert_return(-1, !imemblock_list[i].not_used);
    assert_return(-1, (uintptr_t) ptr ==  (uintptr_t) &imemblock_list[i].val);
    // L_DEBUG("_free  : imemblock_list[%ld](%d) == %lx\n", i, imemblock_list[i].bytes, (uintptr_t) &imemblock_list[i].val);
    assert_return(-1, imemblock_list[i].bytes > 0);
    assert_return(-1, imemblock_list[i].bytes <= sizeof(size_t));
    assert_return(-1, MAGIC_BYTE == imemblock_list[i].res0);
    assert_return(-1, MAGIC_BYTE == imemblock_list[i].res1);
    assert_return(-1, imemblock_list[i].bytes <= sizeof(size_t));
    if (!dry) {
      memset(&imemblock_list[i], ZERO_BYTE, sizeof(memblock_t));
    }
  } else {
    size_t i;
    for (i = 0; (i < MEM_BLOCKS_MAX_LEN); i++) {
      if (!bmemblock_list[i].not_used && (bmemblock_list[i].val == (uintptr_t) ptr)) {
        break;
      }
    }
    assert_return(-1, i < MEM_BLOCKS_MAX_LEN);
    uintptr_t uptr = (uintptr_t) ptr - sizeof(BUF_LEN_TYPE);
    size_t l = *(BUF_LEN_TYPE *) uptr;
    assert_return(-1, l > sizeof(size_t));
    assert_return(-1, l <= MAX_BUF_LEN);
    assert_return(-1, 0 == bmemblock_list[i].bytes);
    assert_return(-1, MAGIC_BYTE == bmemblock_list[i].res0);
    assert_return(-1, MAGIC_BYTE == bmemblock_list[i].res1);
    uptr -= sizeof(unsigned char);
    assert_return(-1, MAGIC_BYTE == *(unsigned char *)uptr);
    assert_return(-1, MAGIC_BYTE == *(unsigned char *)(uptr + sizeof(unsigned char) + sizeof(BUF_LEN_TYPE) + l));
    // L_DEBUG("_free  : bmemblock_list[%ld](%ld) == %lx\n", i, l, (size_t) bmemblock_list[i].val);
    if (!dry) {
      free((void *) uptr);
      memset(&bmemblock_list[i], ZERO_BYTE, sizeof(memblock_t));
    }
  }

  return 0;
}

void * xptr_valid(void * ptr, size_t pos, size_t len)
{
  assert_return(NULL, ptr);
  
  if (((memblock_t *) ptr > &imemblock_list[0]) && ((memblock_t *) ptr < &imemblock_list[MEM_BLOCKS_MAX_LEN])) {
    size_t i = ((uintptr_t) ptr - (uintptr_t) imemblock_list) / sizeof(memblock_t);
    assert_return(NULL, i < MEM_BLOCKS_MAX_LEN);
    assert_return(NULL, !imemblock_list[i].not_used);
    assert_return(NULL, (uintptr_t) ptr == (uintptr_t) &imemblock_list[i].val);
    assert_return(NULL, imemblock_list[i].bytes > 0);
    assert_return(NULL, imemblock_list[i].bytes <= sizeof(size_t));
    assert_return(NULL, pos + len <= imemblock_list[i].bytes);
  } else {
    static size_t i = MEM_BLOCKS_MAX_LEN;
    if ((i < MEM_BLOCKS_MAX_LEN) && (bmemblock_list[i].val == (uintptr_t) ptr)) {
      //  L_DEBUG("Pointer already in cache\n");
    } else {
      // L_DEBUG("Pointer not in cache\n");
      for (i = 0; (i < MEM_BLOCKS_MAX_LEN); i++) {
        if (bmemblock_list[i].val == (uintptr_t) ptr) {
          break;
        }
      }
      assert_return(NULL, i < MEM_BLOCKS_MAX_LEN);
      assert_return(NULL, !bmemblock_list[i].not_used);
      assert_return(NULL, 0 == bmemblock_list[i].bytes);
      assert_return(NULL, MAGIC_BYTE == bmemblock_list[i].res0);
      assert_return(NULL, MAGIC_BYTE == bmemblock_list[i].res1);
    }
    uintptr_t uptr = (uintptr_t) ptr - sizeof(BUF_LEN_TYPE);
    size_t l = *(BUF_LEN_TYPE *) uptr;
    assert_return(NULL, l > sizeof(size_t));
    assert_return(NULL, l <= MAX_BUF_LEN);
    uptr -= sizeof(unsigned char);
    assert_return(NULL, MAGIC_BYTE == *(unsigned char *)uptr);
    assert_return(NULL, MAGIC_BYTE == *(unsigned char *)(uptr + sizeof(unsigned char) + sizeof(BUF_LEN_TYPE) + l));
    assert_return(NULL, pos + len <= l);
  }

  return ptr;
}

void * xptr_assert(void * ptr, size_t pos, size_t len, const char * file, int line)
{
  char * ret_ptr = (char *) xptr_valid(ptr, pos, len);
  if (!ret_ptr) {
    L_FATAL("%s:%d: Invalid Ptr %lx[%lu + %lu]\n", file, line, (uintptr_t) ptr, pos, len);
  }
  return &ret_ptr[pos];
}

#define ptr_valid(ptr, pos) (typeof((ptr))) xptr_valid(ptr, pos, sizeof(*(ptr)))
#define ptr_assert(ptr, pos) (typeof((ptr))) xptr_assert(ptr, pos, sizeof(*(ptr)), __FILE__, __LINE__)

int _free(void * ptr)
{
  assert (0 == _xfree(ptr, false));
  return 0;
}

void init(void)
{
  memset(imemblock_list, ZERO_BYTE, sizeof(imemblock_list));
  memset(bmemblock_list, ZERO_BYTE, sizeof(bmemblock_list));
  L_DEBUG("sizeof(memblock_t) = %ld\n", sizeof(memblock_t));
  L_DEBUG("sizeof(imemblock_list) = %ld\n", sizeof(imemblock_list));
}



size_t rdtsc(){
    unsigned int lo,hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((size_t)hi << 32) | lo;
}

int main (void)
{
  size_t cyc1 = rdtsc();

  init();

  for (size_t i = 0; i < UINT16_MAX * 256; i++) {
    unsigned char  * p1 = (unsigned char *) _malloc(1);
    unsigned char * p2 = (unsigned char *) _malloc(2);
    unsigned char * p3 = (unsigned char *) _malloc(3);
    unsigned char * p4 = (unsigned char *) _malloc(4);
    unsigned char * p5 = (unsigned char *) _malloc(5);
    unsigned char * p9 = (unsigned char *) _malloc(9);
    unsigned char * p32 = (unsigned char *) _malloc(32);
    unsigned char * p256 = (unsigned char *) _malloc(256);
    unsigned char * p1024 = (unsigned char *) _malloc(256);

    #define ASSIGN(p) \
      *ptr_assert(p, 0) = 1; \
      assert(*ptr_assert(p, 0) == 1); \

    ASSIGN(p1);
    ASSIGN(p2);
    ASSIGN(p3);
    ASSIGN(p4);
    ASSIGN(p5);
    ASSIGN(p9);
    ASSIGN(p32);
    ASSIGN(p256);
    ASSIGN(p1024);

    _free(p1);
    _free(p2);
    _free(p3);
    _free(p4);
    _free(p5);
    _free(p9);
    _free(p32);
    _free(p256);
    _free(p1024);
  }
  L_DEBUG("cyc2 - cyc1 %ld\n", rdtsc() - cyc1);
  return 0;
  // _free(_malloc(0));
  _free(_malloc(1));
  _free(_malloc(5));
  _free(_malloc(8));
  _free(_malloc(9));
  _free(_malloc(256));
  _free(_malloc(1024));
  _free(_malloc(8192));
  _free(_malloc(32 * 1024));
  _free(_malloc(MAX_BUF_LEN));


  // unsigned char * p1 = (unsigned char *) _malloc(1);
  // *ptr_assert((unsigned *) p1, 0) = 0;

  unsigned char * p7 = (unsigned char *) _malloc(7);
  unsigned char * p8 = (unsigned char *) _malloc(8);
  unsigned char * p9 = (unsigned char *) _malloc(9);
  unsigned char * p256 = (unsigned char *) _malloc(256);
  _free(p7);
  //*ptr_assert(p7, 0) = 11;
  _free(p8);
  *ptr_assert(p9, 0) = 11;
  assert(*ptr_assert(p9, 0) == 11);
  *ptr_assert(p9, 1) = 11;
  assert(*ptr_assert(p9, 1) == 11);
  *ptr_assert(p9, 8) = 11;
  assert(*ptr_assert(p9, 8) == 11);
  //*ptr_assert(p9, 9) = 11;

  _xfree(p9, true);
  _free(p9);
  // _xfree(p9, true);
  _free(p256);
  //_free(_malloc(MAX_BUF_LEN + 1));
  
  return 0;
}

#if 0
#define ALIGNMENT 8 // must be a power of 2

#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))

/* For Alignment = 8 bytes:
// ALIGN(1) = 8 ,ALIGN(3) = 8, ALIGN(5) = 8, ALIGN(7) = 8
// ALIGN(9) = 16, ALIGN(11) = 16, ALIGN(13) = 16, ALIGN(15) = 16
// ALIGN(17) = 24, ALIGN(19) = 24, ALIGN(21) = 24, ALIGN(23) = 24
// ALIGN(25) = 32, ALIGN(27) = 32, ALIGN(29) = 32 , ALIGN(32) = 32 */
/*
typedef struct {

} memblock_struct_type;
*/


#define USIZE_UINTS_LEN 8
#define USIZE_BYTES_LEN (USIZE_UINTS_LEN * sizeof(unsigned))
#define USIZE_BITS_LEN (USIZE_BYTES_LEN * 8)

typedef unsigned usize_t[USIZE_UINTS_LEN];

static_inline_t bool usize_bit_get(usize_t val, size_t b);
static_inline_t bool usize_bit_get(usize_t val, size_t b) {
  assert(b < USIZE_BITS_LEN);
  return (val[b / 32] >> (b % 32)) & 1;
}

static_inline_t bool usize_bit_set(usize_t val, size_t b);
static_inline_t bool usize_bit_set(usize_t val, size_t b) {
  assert(b < USIZE_BITS_LEN);
  val[b / 32] |= 1 << (b % 32);
  return 1;
}

usize_t s_u8_mem_res;


typedef struct __attribute__((packed)) {
  unsigned res0 : 3;
  unsigned notuse : 1;
  unsigned val : 8;
  unsigned res2 : 4;
} u8_st;

u8_st u8_st_list[MEM_BLOCKS_MAX_LEN];
size_t u8_st_malloc_i = 0;

typedef struct __attribute__((packed)) {
  unsigned res0 : 3;
  unsigned notuse : 1;
  unsigned val : 16;
  unsigned res2 : 4;
} u16_st;
u16_st u16_st_list[MEM_BLOCKS_MAX_LEN];
size_t u16_st_malloc_i = 0;

typedef struct __attribute__((packed)) {
  unsigned res0 : 3;
  unsigned notuse : 1;
  unsigned val : 32;
  unsigned res2 : 4;
} u32_st;
u32_st u32_st_list[MEM_BLOCKS_MAX_LEN];
size_t u32_st_malloc_i = 0;

typedef struct __attribute__((packed)) {
  unsigned res0 : 3;
  unsigned notuse : 1;
  unsigned long val : 64;
  unsigned res2 : 4;
} u64_st;
u64_st u64_st_list[MEM_BLOCKS_MAX_LEN];
size_t u64_st0      _malloc_i = 0;


/*
#define usize_bit_get(val, b)
  b < 8 : val.usize_0 & 

unsigned char s_mem_u8[sizeof(size_t) * 8];
unsigned short s_mem_u16[sizeof(size_t) * 8];
unsigned int s_mem_u32[sizeof(size_t) * 8];
unsigned long s_mem_u64[sizeof(size_t) * 8];*/

#define SIZE_OF(p) (sizeof(p)/sizeof(p[0]))
int main (void)
{

  L_DEBUG("sizeof(u8_st) = %ld\n", sizeof(u8_st) * 8);
  L_DEBUG("sizeof(u16_st) = %ld\n", sizeof(u16_st) * 8);
  L_DEBUG("sizeof(u32_st) = %ld\n", sizeof(u32_st) * 8);
  L_DEBUG("sizeof(u64_st) = %ld\n", sizeof(u64_st) * 8);

  L_DEBUG("sizeof(u8_st_list) = %ld\n", sizeof(u8_st_list) * 8);
  L_DEBUG("sizeof(u16_st_list) = %ld\n", sizeof(u16_st_list) * 8);
  L_DEBUG("sizeof(u32_st_list) = %ld\n", sizeof(u32_st_list) * 8);
  L_DEBUG("sizeof(u64_st_list) = %ld\n", sizeof(u64_st_list) * 8);

  memset(u8_st_list, 0xFF, sizeof(u8_st_list));
  memset(u16_st_list, 0xFF, sizeof(u16_st_list));
  memset(u32_st_list, 0xFF, sizeof(u32_st_list));
  memset(u64_st_list, 0xFF, sizeof(u64_st_list));
  return 0;
  
  memset(&s_u8_mem_res, 0x00, sizeof(s_u8_mem_res));
  /*
  L_DEBUG("sizeof(unsigned char) = %ld\n", sizeof(unsigned char));
  L_DEBUG("sizeof(unsigned short) = %ld\n", sizeof(unsigned short));
  L_DEBUG("sizeof(unsigned int) = %ld\n", sizeof(unsigned int));
  L_DEBUG("sizeof(unsigned long) = %ld\n", sizeof(unsigned long));
  L_DEBUG("USIZE_UINTS_LEN = %ld\n", USIZE_UINTS_LEN);
  L_DEBUG("USIZE_BYTES_LEN = %ld\n", USIZE_BYTES_LEN);
  L_DEBUG("USIZE_BITS_LEN = %ld\n", USIZE_BITS_LEN);
  L_DEBUG("sizeof(usize_t::usize_0) = %ld\n", sizeof(s_u8_mem_res[0]));*/
/*
  L_DEBUG("usize_bit_get(0) = %d\n", usize_bit_get(s_u8_mem_res, 0));
  L_DEBUG("usize_bit_get(255) = %d\n", usize_bit_get(s_u8_mem_res, 255));*/

  //s_u8_mem_res[1] = UINT32_MAX;
  usize_bit_set(s_u8_mem_res, 48);
  usize_bit_set(s_u8_mem_res, 255);
  usize_bit_set(s_u8_mem_res, 0);
  L_DEBUG("usize_bit_get(0) = %d\n", usize_bit_get(s_u8_mem_res, 0));
  L_DEBUG("usize_bit_get(31) = %d\n", usize_bit_get(s_u8_mem_res, 31));
  L_DEBUG("usize_bit_get(32) = %d\n", usize_bit_get(s_u8_mem_res, 32));
  L_DEBUG("usize_bit_get(47) = %d\n", usize_bit_get(s_u8_mem_res, 47));
  L_DEBUG("usize_bit_get(48) = %d\n", usize_bit_get(s_u8_mem_res, 48));
  L_DEBUG("usize_bit_get(49) = %d\n", usize_bit_get(s_u8_mem_res, 49));
  L_DEBUG("usize_bit_get(63) = %d\n", usize_bit_get(s_u8_mem_res, 63));
  L_DEBUG("usize_bit_get(64) = %d\n", usize_bit_get(s_u8_mem_res, 64));
  L_DEBUG("usize_bit_get(255) = %d\n", usize_bit_get(s_u8_mem_res, 255));

  return 0;
}
#endif
