#ifndef COMPAT_H
#define COMPAT_H

#ifdef __cplusplus
#define restrict /* `restrict` keyword is not present in C++ or versions of C prior to C99. */
#elif !(defined __STDC_VERSION__) || (__STDC_VERSION__ >= 199901L)

#define inline   /* `inline` keyword is not standardized prior to C99. */
#define restrict /* `restrict` keyword is not standardized prior to C99. */

#endif
#endif
