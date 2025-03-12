#ifndef MYCLIB_H
#define MYCLIB_H

#if (defined __STDC_VERSION__ && __STDC_VERSION__ > 199409L)

#if (__STDC_VERSION__ < 202311L)
#include <stdbool.h> /* For C99 to C17. */
#endif

#else

#define inline /* `inline` keyword is not standardized prior to C99. */
/* Boolean type for C95 and below. */
typedef unsigned char bool;
#ifndef true
#define true (1)
#endif
#ifndef false
#define false (0)
#endif

#endif
#endif
