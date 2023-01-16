#if 0
$CXX   fs.cc gnss.cc main.cc dbus.cc  ntp.cc  rtc.cc  pollfd.cc app.cc  com_actia_Power_Diagnostic.cc  -o main.cc.bin  -lsystemd -lacu6-pro-lv-common ../toolchain/sysroots/aarch64-acu6-linux/usr/lib/libactia-common-cxx.so
g++-11  -fstack-protector-strong -D_FORTIFY_SOURCE=2 -Wformat -Wformat-security -Werror=format-security -O2 -ggdb -funwind-tables -feliminate-unused-debug-types -Wl,--hash-style=gnu -Wl,-O2 -Wl,--as-needed -Wl,-z,relro,-z,now -Wall -Wextra -pedantic memalloc.cc -o memalloc.bin

g++ -fstack-protector-strong  -D_FORTIFY_SOURCE=2  -Wshift-count-overflow -Wsizeof-array-argument -Wall -Wextra -Werror -pedantic -pedantic-errors -Warray-bounds=2 -feliminate-unused-debug-types -Wformat -Wformat-security -Werror=format-security -O2 -ggdb -funwind-tables -std=gnu++17 -Wl,-O2 -fstack-protector-strong  -D_FORTIFY_SOURCE=2 -Wformat -Wformat-security -Werror=format-security -ffile-prefix-map=/home/hs/work/astm/timed=. -Wshift-count-overflow -Wsizeof-array-argument -Wall -Wextra -Werror -pedantic -pedantic-errors -Warray-bounds=2 -feliminate-unused-debug-types -fstack-protector-strong -D_FORTIFY_SOURCE=2 -Wformat -Wformat-security -Werror=format-security -O2 -ggdb -funwind-tables -std=gnu++17  memcheck1.cc -o memcheck1.bin -Wno-unused-variabl

-Wstringop-overflow -Wfortify-source
-Wstringop-overflow
-Wstringop-overflow=1

 g++-12 -DWARRAY_BOUND -fstack-protector-strong  -D_FORTIFY_SOURCE=2  -Wshift-count-overflow -Wsizeof-array-argument -Wall -Wextra -Werror -pedantic -pedantic-errors -Warray-bounds=2 -feliminate-unused-debug-types -Wformat -Wformat-security -Werror=format-security -O2 -ggdb -funwind-tables -std=gnu++17 -Wl,-O2 memcheck.cc



export CXX="g++ -fstack-protector-strong -D_FORTIFY_SOURCE=2 -Wshift-count-overflow -Wsizeof-array-argument -Wall -Wextra -Werror -pedantic -pedantic-errors -Warray-bounds=2 -feliminate-unused-debug-types -Wformat -Wformat-security -Werror=format-security -O2 -ffile-prefix-map=$PWD=."
export CXX="$CXX -fdiagnostics-show-option -Wfloat-equal	-Wconversion -Wlogical-op	-Wundef	-Wredundant-decls	-Wshadow -Wstrict-overflow=2 -Wwrite-strings -Wpointer-arith	-Wcast-qual	-Wformat=2 -Wformat-truncation	-Wmissing-include-dirs -Wswitch-enum	-Wsign-conversion	-Wdisabled-optimization	-Winline -Winvalid-pch	-Wmissing-declarations -Wdouble-promotion -Wshadow -Wtrampolines	-Wvector-operation-performance -Wnull-dereference -Wduplicated-cond	-Wshift-overflow=2 -Wnull-dereference -Wduplicated-cond	"
export CXX="$CXX   -Wctor-dtor-privacy -Woverloaded-virtual -Wstrict-null-sentinel -Wuseless-cast -Wzero-as-null-pointer-constant -Wextra-semi   "

gcc decl.c -o decl.bin -Wall -Wextra -fstack-protector-strong -D_FORTIFY_SOURCE=2 -Wshift-count-overflow -Wsizeof-array-argument -Wall -Wextra -Werror -Warray-bounds=2 -feliminate-unused-debug-types -Wformat -Wformat-security -Werror=format-security -O2 -ffile-prefix-map=$PWD=. -std=gnu11 -fdiagnostics-show-option -Wfloat-equal        -Wconversion -Wlogical-op       -Wundef    -Wredundant-decls       -Wshadow -Wstrict-overflow=2 -Wwrite-strings -Wpointer-arith    -Wcast-qual     -Wformat=2 -Wformat-truncation  -Wmissing-include-dirs -Wswitch-enum    -Wsign-conversion       -Wdisabled-optimization    -Winline -Winvalid-pch  -Wmissing-declarations -Wdouble-promotion -Wshadow -Wtrampolines        -Wvector-operation-performance -Wnull-dereference -Wduplicated-cond     -Wshift-overflow=2 -Wnull-dereference -Wduplicated-cond

#endif

#include "common.h"


#define VPTR_CHK_MAX_LEN 16
#define VPTR_TYPE_LOW_FLAGS 0x80
#define VPTR_TYPE_STA_ALLOC VPTR_TYPE_LOW_FLAGS
#define VPTR_TYPE_DYN_ALLOC (VPTR_TYPE_LOW_FLAGS | 0x01)
#define VPTR_TYPE_DECL_BASIC VPTR_TYPE_LOW_FLAGS
#define VPTR_TYPE_DECL_BUFFR (VPTR_TYPE_LOW_FLAGS | 0x02)
#define VPTR_TYPE_DECL_FREED VPTR_TYPE_LOW_FLAGS
#define VPTR_TYPE_DECL_USED  (VPTR_TYPE_LOW_FLAGS | 0x04)
#define VPTR_TYPE_DECL_PRIVATE VPTR_TYPE_LOW_FLAGS
#define VPTR_TYPE_DECL_PUBLIC (VPTR_TYPE_LOW_FLAGS | 0x08)
#define VPTR_TYPE_DECL_LOCAL VPTR_TYPE_LOW_FLAGS
#define VPTR_TYPE_DECL_GLOBAL (VPTR_TYPE_LOW_FLAGS | 0x10)

#define VPTR_HDR_LEN 8
#define VPTR_HDR_SAN0 0
#define VPTR_HDR_LEN0 2
#define VPTR_HDR_LEN1 4
#define VPTR_HDR_FLAG 6
#define VPTR_HDR_PID8 7


/*--------------------------------------------------------------------------------------------*/
#define CONCAT_(x,y) x##y
#define CONCAT(x,y) CONCAT_(x,y)
#define _GenerateVarName(a1, a2, a3, p, s) CONCAT(s_##p##_##a1##_##a3##_##a2##_##s, __LINE__)
#define GenerateVarName(a1, a2, a3, p, s) _GenerateVarName(var, a2, a3, p, s)

#define StaticBufName(a1, a2, a3) GenerateVarName(a1, a2, a3, static_buf, decl)
#define VolatileBufName(a1, a2, a3) GenerateVarName(a1, a2, a3, volatile_buf, decl)

/*--------------------------------------------------------------------------------------------*/
#define StaticBufConstructor(a1, a2, a3) \
  __attribute__((constructor)) \
  GenerateVarName(a1, a2, a3, static_buf, constructor_func)

#define StaticBufDestructor(a1, a2, a3) \
  __attribute__((destructor)) \
  GenerateVarName(a1, a2, a3, static_buf, destructor_func)

/*--------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------*/
#define shared_var(t, n, l) \
  uint8_t StaticBufName(t, n, l)[VPTR_HDR_LEN +  (l * sizeof(t))] ; \
  static t * n; \
  static_inline_t void StaticBufConstructor(t, n, l)(void) { \
    static_assert(l); \
    n = (t *) &StaticBufName(t, n, l)[VPTR_HDR_LEN]; \
    StaticBufName(t, n, l)[VPTR_HDR_SAN0] = 0xFF; \
    *((uint16_t *) &StaticBufName(t, n, l)[VPTR_HDR_LEN0]) = sizeof(t); \
    *((uint16_t *) &StaticBufName(t, n, l)[VPTR_HDR_LEN1]) = l; \
    StaticBufName(t, n, l)[VPTR_HDR_FLAG] = VPTR_TYPE_STA_ALLOC | VPTR_TYPE_DECL_BUFFR | VPTR_TYPE_DECL_USED | VPTR_TYPE_DECL_PRIVATE | VPTR_TYPE_DECL_GLOBAL; \
    StaticBufName(t, n, l)[VPTR_HDR_PID8] = PtrId8((uintptr_t) &StaticBufName(t, n, l)[VPTR_HDR_LEN]); \
    StaticBufName(t, n, l)[VPTR_HDR_LEN + (l * sizeof(t)) - 1] = 0xff; \
    L_DEBUG("StaticBufConstructor: %s:0x%lx:0x%x:%d:%d\n", #n, \
      (uintptr_t) &StaticBufName(t, n, l)[VPTR_HDR_LEN], PtrId8((uintptr_t) &StaticBufName(t, n, l)[VPTR_HDR_LEN]), \
      ((uint16_t *) &StaticBufName(t, n, l)[VPTR_HDR_LEN0])[0], ((uint16_t *) &StaticBufName(t, n, l)[VPTR_HDR_LEN1])[0]); \
  } \
  static_inline_t void StaticBufDestructor(t, n, l)(void) { \
    L_DEBUG("StaticBufDestructor: %s:0x%lx:0x%x:%d:%d\n", #n, \
      (uintptr_t) &StaticBufName(t, n, l)[VPTR_HDR_LEN], PtrId8((uintptr_t) &StaticBufName(t, n, l)[VPTR_HDR_LEN]), \
      ((uint16_t *) &StaticBufName(t, n, l)[VPTR_HDR_LEN0])[0], ((uint16_t *) &StaticBufName(t, n, l)[VPTR_HDR_LEN1])[0]); \
  } \
  static_inline_t t* n##Ptr (size_t pos) { \
    return (t*) &StaticBufName(t, n, l)[pos * sizeof(t)]; \
  }

#define unique_var(t, n, l) \
  static_assert(l); \
  static_assert(l * sizeof(t) < UINT16_MAX); \
  uint8_t VolatileBufName(t, n, l)[VPTR_HDR_LEN +  (l * sizeof(t))] ; \
  t * n = (t *) &VolatileBufName(t, n, l)[VPTR_HDR_LEN]; \
  VolatileBufName(t, n, l)[VPTR_HDR_SAN0] = 0xFF; \
  *((uint16_t *) &VolatileBufName(t, n, l)[VPTR_HDR_LEN0]) = sizeof(t); \
  *((uint16_t *) &VolatileBufName(t, n, l)[VPTR_HDR_LEN1]) = l; \
  VolatileBufName(t, n, l)[VPTR_HDR_FLAG] = VPTR_TYPE_STA_ALLOC | VPTR_TYPE_DECL_BUFFR | VPTR_TYPE_DECL_USED; \
  VolatileBufName(t, n, l)[VPTR_HDR_PID8] = PtrId8((uintptr_t) & VolatileBufName(t, n, l)[VPTR_HDR_LEN]); \
  VolatileBufName(t, n, l)[VPTR_HDR_LEN + (l * sizeof(t)) - 1] = 0xFF; \
  Mem_OnCleanupCheck(&VolatileBufName(t, n, l)[VPTR_HDR_LEN]); \
  inline_t t* n##Ptr (size_t pos) { \
    return (t*) &VolatileBufName(t, n, l)[pos * sizeof(t)]; \
  }

/*--------------------------------------------------------------------------------------------*/

#define Mem_AssertOnCleanupCheck(...) \
  __attribute__((cleanup(_CheckBufferCleanup))) \
  const void * (GenerateVarName(void, ptr, list, local_var, decl))[] = { \
  (void *) (COUNT_ARGUMENTS(__VA_ARGS__) + 4), __FILE__, (void *) __LINE__, "ARGS", __VA_ARGS__ , NULL}; \
  _AssertBuffer((GenerateVarName(void, ptr, list, local_var, decl)))

#define Mem_OnCleanupCheck(...) \
  __attribute__((cleanup(_CheckBufferCleanup))) \
  const void * (GenerateVarName(void, ptr, list, local_var, decl))[] = { \
  (void *) (COUNT_ARGUMENTS(__VA_ARGS__) + 4), __FILE__, (void *) __LINE__, "ARGS", __VA_ARGS__ , NULL}

/*--------------------------------------------------------------------------------------------*/

#define PTR_VALID(arg, r) \
  if (!arg) { \
    return r; \
  } \
  const uint8_t * u8ptr = &((const uint8_t *) arg) [-VPTR_HDR_LEN]; \
  size_t sz = (*(const uint16_t *)&u8ptr[VPTR_HDR_LEN0]) *  (*(const uint16_t *)&u8ptr[VPTR_HDR_LEN1]); \
  return (u8ptr[VPTR_HDR_PID8] == PtrId8((uintptr_t) arg)) && \
    ((VPTR_TYPE_LOW_FLAGS & u8ptr[VPTR_HDR_FLAG]) == VPTR_TYPE_LOW_FLAGS) && \
    (u8ptr[0] == 0xFF) && sz && (sz < UINT16_MAX) && (0xFF == u8ptr[VPTR_HDR_LEN +  sz - 1])

/*--------------------------------------------------------------------------------------------*/
static_inline_t bool _vPtrValid(const void * arg)
{
  PTR_VALID(arg, false) ? true : false;
}

static_inline_t const uint8_t * _vPtrGetBase(const void * arg)
{
  PTR_VALID(arg, NULL) ? u8ptr : NULL;
}

static_inline_t  uint8_t _vPtrGetFlags(const void * arg)
{
  PTR_VALID(arg, 0) ? u8ptr[VPTR_HDR_FLAG] : 0;
}

static_inline_t  ssize_t _vPtrGetLen(const void * arg)
{
  PTR_VALID(arg, -1) ? (*(const uint16_t *)&u8ptr[VPTR_HDR_LEN1]) : -1;
}

static_inline_t  ssize_t _vPtrGetTypeLen(const void * arg)
{
  PTR_VALID(arg, -1) ? (*(const uint16_t *)&u8ptr[VPTR_HDR_LEN0]) : -1;
}

static_inline_t  uint8_t _vPtrGetTypePid8(const void * arg)
{
  PTR_VALID(arg, 0) ? u8ptr[VPTR_HDR_PID8] : 0;
}

void _AssertBuffer(const void * arg)
{
  const uintptr_t * vptr = (const uintptr_t *) arg;
  assert(vptr);

  if (vptr[0] >= VPTR_CHK_MAX_LEN) {
    L_ERROR("%s:%d: AssertBuffer: InvalidPtrLen(%ld)\n", (const char *) vptr[1], (int) vptr[2], vptr[0]);
    return;
  }
  if (vptr[0] < 4) {
    L_ERROR("%s:%d: AssertBuffer: InvalidPtrLen(%ld)\n", (const char *) vptr[1], (int) vptr[2], vptr[0]);
    return;
  }
  // L_DEBUG("%s:%d: AssertBuffer: len(0x%lx)\n", (const char *) vptr[1], (int) vptr[2], (uintptr_t) vptr[0]);

  for (size_t i = 4; (i < vptr[0]) && vptr[i]; i++) {
    const uint8_t * u8ptr = &((const uint8_t *) vptr[i])[-VPTR_HDR_LEN];
    const uint16_t * u16Ptr = (const uint16_t *) &u8ptr[VPTR_HDR_LEN0];
    L_DEBUG("%s:%d: AssertBuffer: vptr[%lu](0x%lx:0x%x) Size(%d:%d) Flags(0x%0x) %s\n",
      (const char *) vptr[1], (int) vptr[2], i, (uintptr_t) vptr[i], PtrId8(vptr[i]),
       u16Ptr[0], u16Ptr[1], u8ptr[VPTR_HDR_FLAG], _vPtrValid((const void *) vptr[i]) ? "OK" : "NOK");
  }
}

void _CheckBufferCleanup(const void * arg)
{
  const uintptr_t * vptr = (const uintptr_t *) arg;
  assert(vptr);

  if (vptr[0] >= VPTR_CHK_MAX_LEN) {
    L_ERROR("%s:%d: CheckBufferCleanup: InvalidPtrLen(%ld)\n", (const char *) vptr[1], (int) vptr[2], vptr[0]);
    return;
  }
  if (vptr[0] < 4) {
    L_ERROR("%s:%d: CheckBufferCleanup: InvalidPtrLen(%ld)\n", (const char *) vptr[1], (int) vptr[2], vptr[0]);
    return;
  }
  // L_DEBUG("%s:%d: CheckBufferCleanup: len(0x%lx)\n", (const char *) vptr[1], (int) vptr[2], (uintptr_t) vptr[0]);

  for (size_t i = 4; (i < vptr[0]) && vptr[i]; i++) {
    const uint8_t * u8ptr =  &((const uint8_t *) vptr[i])[-VPTR_HDR_LEN];
    const uint16_t * u16Ptr = (const uint16_t *) &u8ptr[VPTR_HDR_LEN0];
    L_DEBUG("%s:%d: CheckBufferCleanup: vptr[%lu](0x%lx:0x%x) Size(%d:%d) Flags(0x%0x) %s\n",
      (const char *) vptr[1], (int) vptr[2], i, (uintptr_t) vptr[i], PtrId8(vptr[i]),
       u16Ptr[0], u16Ptr[1], u8ptr[VPTR_HDR_FLAG], _vPtrValid((const void *) vptr[i]) ? "OK" : "NOK");
  }
}

/*--------------------------------------------------------------------------------------------*/

shared_var(uintptr_t, char_6_buf, 6);
shared_var(char, s_char_2_buf, 2);
shared_var(char, s_char_8_buf, 8);
shared_var(char, s_char_1_buf, 1);
shared_var(char*, pchar_77_buf, 77);
shared_var(void*, pvoid_77_buf, 77);

int main(int argc, char * argv[])
{
  (void) argc;

  Mem_AssertOnCleanupCheck(&argc, argv);

  assert (false == _vPtrValid(argv));
  assert (NULL == _vPtrGetBase(argv));
  assert (0 == _vPtrGetFlags(argv));
  assert (-1 == _vPtrGetLen(argv));
  assert (-1 == _vPtrGetTypeLen(argv));
  assert (0 == _vPtrGetTypePid8(argv));

  unique_var(char, buf_5, 5);
  assert (true == _vPtrValid(buf_5));
  assert (&((const uint8_t * ) buf_5)[-VPTR_HDR_LEN] == _vPtrGetBase(buf_5));
  assert ((VPTR_TYPE_STA_ALLOC | VPTR_TYPE_DECL_BUFFR | VPTR_TYPE_DECL_USED) == _vPtrGetFlags(buf_5));
  assert (5 == _vPtrGetLen(buf_5));
  assert (sizeof(char) == _vPtrGetTypeLen(buf_5));
  assert (PtrId8((uintptr_t) buf_5) == _vPtrGetTypePid8(buf_5));


  unique_var(char, buf_4096, 4096);
  assert (4096 == _vPtrGetLen(buf_4096));
  assert (sizeof(char) == _vPtrGetTypeLen(buf_4096));

  *s_char_8_bufPtr(5) = 0;
  memcpy(buf_4096, &s_char_8_buf[11], 8);

  Mem_AssertOnCleanupCheck(char_6_buf);
  assert (true == _vPtrValid(char_6_buf));
  assert (&((const uint8_t * ) char_6_buf)[-VPTR_HDR_LEN] == _vPtrGetBase(char_6_buf));
  assert ((VPTR_TYPE_STA_ALLOC | VPTR_TYPE_DECL_BUFFR | VPTR_TYPE_DECL_USED | VPTR_TYPE_DECL_PRIVATE | VPTR_TYPE_DECL_GLOBAL) == _vPtrGetFlags(char_6_buf));
  assert (6 == _vPtrGetLen(char_6_buf));
  assert (sizeof(uintptr_t) == _vPtrGetTypeLen(char_6_buf));
  assert (PtrId8((uintptr_t) char_6_buf) == _vPtrGetTypePid8(char_6_buf));

  Mem_AssertOnCleanupCheck(s_char_2_buf);
  Mem_AssertOnCleanupCheck(s_char_8_buf);
  Mem_AssertOnCleanupCheck(s_char_1_buf);

  typedef struct __attribute__((packed)) {
    uint8_t res0;
    uint8_t nuse;
    uint16_t size;
    uintptr_t vptr;
    uint8_t data;
  } memblock_t;


  unique_var(memblock_t, memblock_1024, 1024);

  assert (true == _vPtrValid(memblock_1024));
  assert (&((const uint8_t * ) memblock_1024)[-VPTR_HDR_LEN] == _vPtrGetBase(memblock_1024));
  assert ((VPTR_TYPE_STA_ALLOC | VPTR_TYPE_DECL_BUFFR | VPTR_TYPE_DECL_USED | VPTR_TYPE_DECL_PRIVATE | VPTR_TYPE_DECL_GLOBAL) == _vPtrGetFlags(char_6_buf));
  assert (1024 == _vPtrGetLen(memblock_1024));
  assert (sizeof(memblock_t) == _vPtrGetTypeLen(memblock_1024));
  assert (PtrId8((uintptr_t) memblock_1024) == _vPtrGetTypePid8(memblock_1024));

  return 0;
}
