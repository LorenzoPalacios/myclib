#ifndef UTILMYCLIB_H

#include <stdio.h>
#include <stdlib.h> /* For abort(). */

typedef unsigned char byte;

/* Aborts the program if `expr` evaluates to false. */
#define util_assert(expr) \
  inline_if(expr, NULL, (util_assert_msg(expr), abort()))

#if (defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L)
#define util_assert_msg(expr)                                                 \
  fprintf(stderr,                                                             \
          "\n%s:%zu in function %s - Expression\n\n%s\n\nevaluated to false!\n", \
          __FILE__, (size_t)__LINE__, __func__, STRINGIFY(expr)),             \
      fflush(stderr)
#else
#define util_assert_msg(expr)                                               \
  fprintf(stderr, "\n%s:%zu - Expression evaluated to false!\n", __FILE__, \
          (size_t)__LINE__),                                                \
      fflush(stderr)
#endif

#define inline_if(cond, true_branch_expr, false_branch_expr) \
  (((cond) ? (void)(true_branch_expr) : (void)(false_branch_expr)))

#ifndef STRINGIFY
#define STRINGIFY(x) #x
#endif

#endif
