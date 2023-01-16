#ifndef __COMMON__
#define __COMMON__ __COUNTER__

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

#define MAX_BUF_SIZE 65536

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
#define assert(expr) \
  ((void) sizeof ((expr) ? 1 : 0), __extension__ ({ \
  if (expr)	{ \
    ; /* empty */ \
  } \
  else { \
    (void) fprintf(stderr, AT "Assertion(" #expr ") failed\n" ); \
    abort();	\
  } \
  }))

#define assert_return(r, expr) \
  do { if (!(expr)) { \
    (void) fprintf(stderr, AT "Assertion(" #expr ") failed\n" ); \
    return r;	\
  }} while(0)

static_inline_t size_t rdtsc(void)
{
  size_t lo = 0;
  size_t hi = 0;
  __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
  return (hi << 32) | lo;
}


#define ELEVENTH_ARGUMENT(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, ...) _11
#define COUNT_ARGUMENTS(...) ELEVENTH_ARGUMENT(none, ##__VA_ARGS__, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define PP_NARG(...) \
         PP_NARG_(__VA_ARGS__,PP_RSEQ_N())
#define PP_NARG_(...) \
         PP_ARG_N(__VA_ARGS__)
#define PP_ARG_N( \
          _1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
         _11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
         _21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
         _31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
         _41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
         _51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
         _61,_62,_63,N,...) N
#define PP_RSEQ_N() \
         63,62,61,60,                   \
         59,58,57,56,55,54,53,52,51,50, \
         49,48,47,46,45,44,43,42,41,40, \
         39,38,37,36,35,34,33,32,31,30, \
         29,28,27,26,25,24,23,22,21,20, \
         19,18,17,16,15,14,13,12,11,10, \
         9,8,7,6,5,4,3,2,1,0

static_inline_t uint8_t PtrId8(uintptr_t uPtr)
{
  uint8_t * p = (uint8_t *) &uPtr;
  uint8_t uid8 = ((p[0] ^ p[1]) ^ (p[2] ^ p[3]) ^ (p[4] ^ p[5]) ^ (p[6] ^ p[7]));
  return uid8;
}


void _AssertBuffer(const void * arg);
void _CheckBufferCleanup(const void * arg);


#define CheckIfInsideFunction(fname) \
  const char * fname =  *__builtin_FUNCTION() ? __builtin_FUNCTION():NULL;

#endif