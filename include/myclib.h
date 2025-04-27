#ifndef MYCLIB_H
#define MYCLIB_H

#include <limits.h> /* LONG_MAX, LONG_MIN, ULONG_MAX, LLONG_MAX, LLONG_MIN, ULLONG_MAX */
#include <stddef.h> /* NULL, size_t */
#include <stdio.h>  /* fputs(), stderr */
#include <stdlib.h> /* abort() */

/* - COMPATIBILITY - */

#define STDC95_VERSION (199409L)
#define STDC99_VERSION (199901L)
#define STDC11_VERSION (201112L)
#define STDC17_VERSION (201710L)
#define STDC23_VERSION (202311L)

#ifdef __STDC_VERSION__
#define IS_STDC95 (__STDC_VERSION__ >= STDC95_VERSION)
#define IS_STDC99 (__STDC_VERSION__ >= STDC99_VERSION)
#define IS_STDC11 (__STDC_VERSION__ >= STDC11_VERSION)
#define IS_STDC17 (__STDC_VERSION__ >= STDC17_VERSION)
#define IS_STDC23 (__STDC_VERSION__ >= STDC23_VERSION)
#else
#define IS_STDC95 (0)
#define IS_STDC99 (0)
#define IS_STDC11 (0)
#define IS_STDC17 (0)
#define IS_STDC23 (0)
#endif

#ifdef __cplusplus
#define IS_CPP (1)
#else
#define IS_CPP (0)
#endif

#if (IS_CPP)
#define restrict /* `restrict` keyword is not present in C++ or versions of C \
                    prior to C99. */
#elif (!IS_STDC99)

#define inline   /* `inline` keyword is not standardized prior to C99. */
#define restrict /* `restrict` keyword is not standardized prior to C99. */
#endif

/* - DEFINITIONS - */

/* - BOOL - */
#if (!IS_CPP && !IS_STDC23)
#if (IS_STDC99)

#include <stdbool.h> /* For C99 to C17. */
#else
/* Boolean type for standards prior to C99. */
typedef unsigned char bool;
#ifndef true
#define true (1)
#endif
#ifndef false
#define false (0)
#endif

#endif
#endif

typedef unsigned char byte;

/* - ASSERTION - */

#if (IS_STDC99)
#define util_assert_msg(expr)                                              \
  ((fputs("\n"__FILE__                                                     \
          ":" STRINGIFY(__LINE__) " in function ",                         \
          stderr),                                                         \
    fputs(__func__, stderr),                                               \
    fputs(" - Expression\n\n" STRINGIFY(expr) "\n\nevaluated to false!\n", \
          stderr),                                                         \
    fflush(stderr)))
#else
#define util_assert_msg(expr)                                        \
  fputs(                                                             \
      "\n"__FILE__                                                   \
      ":" STRINGIFY(__LINE__) " - Expression evaluated to false!\n", \
      stderr),                                                       \
      fflush(stderr)
#endif

/* Aborts the program if `expr` evaluates to false. */
#define util_assert(expr) \
  inline_if(expr, NULL, (util_assert_msg(expr), abort()))

/* - MISCELLANEOUS - */

#define ARR_LEN(arr) (sizeof(arr) / sizeof *(arr))

#define inline_if(cond, true_branch_expr, false_branch_expr) \
  ((cond) ? (void)(true_branch_expr) : (void)(false_branch_expr))

#define INTEGRAL_CAST(n, cast_type) \
  ((cast_type)((wb_uint)(n) & (wb_uint)(~(cast_type)0)))

#define STRINGIFY(x) #x

/* - WIDEST BASIC INTEGRAL TYPES - */

#if (IS_STDC99)
#include <inttypes.h>
#include <stdint.h>

/* Widest basic integral type. */
typedef intmax_t wb_int;
/* Widest basic unsigned integral type. */
typedef uintmax_t wb_uint;

#define WB_INT_FMT "%lld"
#define WB_UINT_FMT "%llu"

#define WB_INT_MAX (INTMAX_MAX)
#define WB_UINT_MAX (UINTMAX_MAX)
#define WB_INT_MIN (INTMAX_MIN)
#else
/* Widest basic integral type. */
typedef long wb_int;
/* Widest basic unsigned integral type. */
typedef unsigned long wb_uint;

#define WB_INT_FMT "%ld"
#define WB_UINT_FMT "%lu"

#define WB_INT_MAX (LONG_MAX)
#define WB_UINT_MAX (ULONG_MAX)
#define WB_INT_MIN (LONG_MIN)
#endif

#endif
