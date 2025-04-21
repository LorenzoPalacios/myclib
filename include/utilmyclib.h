#ifndef UTILMYCLIB_H
#define UTILMYCLIB_H

#include <limits.h> /* For ULLONG_MAX, ULONG_MAX. */
#include <stddef.h> /* For NULL, size_t. */
#include <stdio.h>  /* For fputs(), stderr. */
#include <stdlib.h> /* For abort(). */

typedef unsigned char byte;

#define ARR_LEN(arr) (sizeof(arr) / sizeof *(arr))

#if (defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L)
#define cnt_digits(n)                                              \
  (cnt_digits_(((n) < 0 ? ULLONG_MAX - (unsigned long long)(n) + 1 \
                        : (unsigned long long)(n))))

static inline size_t cnt_digits_(unsigned long long n) {
  size_t digits = 1;
  while (n /= 10) digits++;
  return digits;
}
#else
#define cnt_digits(n) \
  (cnt_digits_(       \
      ((n) < 0 ? ULONG_MAX - (unsigned long)(n) + 1 : (unsigned long)(n))))

static size_t cnt_digits_(unsigned long n) {
  size_t digits = 1;
  while (n /= 10) digits++;
  return digits;
}
#endif

#define inline_if(cond, true_branch_expr, false_branch_expr) \
  (((cond) ? (void)(true_branch_expr) : (void)(false_branch_expr)))

#define INT_TO_CHAR(c) ((char)(unsigned char)(c))

/* Aborts the program if `expr` evaluates to false. */
#define util_assert(expr) \
  inline_if(expr, NULL, (util_assert_msg(expr), abort()))

#if (defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L)
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

#ifndef STRINGIFY
#define STRINGIFY(x) #x
#endif

#endif
