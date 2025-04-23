#ifndef UTILMYCLIB_H
#define UTILMYCLIB_H

#include <limits.h>
#include <stddef.h> /* For NULL, size_t. */
#include <stdio.h>  /* For fputs(), stderr. */
#include <stdlib.h> /* For abort(). */

#define STDC95_VERSION 199409L
#define STDC99_VERSION 199901L
#define STDC11_VERSION 201112L
#define STDC17_VERSION 201710L
#define STDC23_VERSION 202311L

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

typedef unsigned char byte;

#define ARR_LEN(arr) (sizeof(arr) / sizeof *(arr))

#define inline_if(cond, true_branch_expr, false_branch_expr) \
  (((cond) ? (void)(true_branch_expr) : (void)(false_branch_expr)))

#define INT_TO_CHAR(c) ((char)(unsigned char)(c))

/* Aborts the program if `expr` evaluates to false. */
#define util_assert(expr) \
  inline_if(expr, NULL, (util_assert_msg(expr), abort()))

#if (IS_STDC99)
#define util_assert_msg(expr)                                              \
  ((fputs("\n"__FILE__                                                     \
          ":" STRINGIFY(__LINE__) " in function ",                         \
          stderr),                                                         \
    fputs(__func__, stderr),                                               \
    fputs(" - Expression\n\n" STRINGIFY(expr) "\n\nevaluated to false!\n", \
          stderr),                                                         \
    fflush(stderr)))

/* Widest basic integral type. */
typedef long long wb_int;
/* Widest basic unsigned integral type. */
typedef unsigned long long wb_uint;

#define WB_INT_FMT "%lld"
#define WB_UINT_FMT "%llu"

#define WB_INT_MAX (LLONG_MAX)
#define WB_UINT_MAX (ULLONG_MAX)
#define WB_INT_MIN (LLONG_MIN)
#else
#define util_assert_msg(expr)                                        \
  fputs(                                                             \
      "\n"__FILE__                                                   \
      ":" STRINGIFY(__LINE__) " - Expression evaluated to false!\n", \
      stderr),                                                       \
      fflush(stderr)

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

#ifndef STRINGIFY
#define STRINGIFY(x) #x
#endif

#endif
