#ifndef MYCLIB_H
#define MYCLIB_H

#ifndef __cplusplus

#if (defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L)

#if (__STDC_VERSION__ < 202311L)
#include <stdbool.h> /* For C99 to C17. */
#endif
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

#endif
