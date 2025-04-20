#ifndef UTILMYCLIB_H

#include <limits.h>
#include <stdio.h>
#include <stdlib.h> /* For abort(). */

typedef unsigned char byte;

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

#define inline_if(cond, true_branch_expr, false_branch_expr) \
  (((cond) ? (void)(true_branch_expr) : (void)(false_branch_expr)))

#if (defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L)
#define cnt_digits(n) \
  (cnt_digits_((n) < 0 ? ULLONG_MAX - (unsigned long long)(n) + 1 : (n)))

static inline size_t cnt_digits_(unsigned long long n) {
  size_t digits = 1;
  while (n /= 10) digits++;
  return digits;
}
#else
#define cnt_digits(n) \
  (cnt_digits_((n) < 0 ? ULLONG_MAX - (unsigned long)(n) + 1 : (n)))

static inline size_t cnt_digits_(unsigned long n) {
  size_t digits = 1;
  while (n /= 10) digits++;
  return digits;
}
#endif

#ifndef STRINGIFY
#define STRINGIFY(x) #x
#endif

#endif
