#pragma once

#include <ctype.h>
#include <stddef.h>

// NOTE: DEPRECATED! Damned MSVC doesn't have support for generic types so instead I will use stdlib.h for this functions
//#define max(x, y)              \
//    ({                         \
//      __typeof__ __x = (x);    \
//      __typeof__ __y = (y);    \
//      __x > __y ? __x : __y;   \
//    })
//
//#define min(x, y)              \
//    ({                         \
//      __typeof__ __x = (x);    \
//      __typeof__ __y = (y);    \
//      __x < __y ? __x : __y;   \
//    })

static inline int strcmp_nocase(const char* str1, const char* str2)
{
    int result = 0;
    while (*str1 || *str2)
    {
        result = tolower((int)(*str1)) - tolower((int)(*str2));
        if (result != 0)
            break;
        str1++, str2++;
    }

    return result;
}

static inline int strncmp_nocase(const char* str1, const char* str2, size_t n)
{
    int result = 0;
    size_t i = 0;
    while (i < n && (*str1 || *str2))
    {
        result = tolower((int)(*str1)) - tolower((int)(*str2));
        if (result != 0)
            break;
        str1++, str2++, i++;
    }

    return result;
}
