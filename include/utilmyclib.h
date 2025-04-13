#ifndef UTILMYCLIB_H

#include <stdio.h>
#include <stdlib.h> /* For abort(). */

/* Aborts the program if `expr` evaluates to false. */
#define util_assert(expr) \
  ((void)((expr) ? 0 : util_assert_msg(expr), fflush(stderr), abort()))

#if (__STDC_VERSION__ >= 199901L)
#define util_assert_msg(expr)                                                 \
  fprintf(stderr,                                                             \
          "\n%s:%zu in function %s -\nExpression (%s) evaluated to false!\n", \
          __FILE__, (size_t)__LINE__, __func__, STRINGIFY(expr)),             \
      fflush(stderr)
#else
#define util_assert_msg(expr)                                          \
  fprintf(stderr, "\n%s:%zu -\nExpression (%s) evaluated to false!\n", \
          __FILE__, (size_t)__LINE__, STRINGIFY(expr)),                \
      fflush(stderr)
#endif

#define inline_if(cond, true_branch_expr, false_branch_expr) \
  ((void)((cond) ? (true_branch_expr) : (false_branch_expr)))

#ifndef STRINGIFY
#define STRINGIFY(x) #x
#endif

#endif
