#pragma once

#define max(x, y)              \
    ({                         \
      __typeof__ __x = (x);    \
      __typeof__ __y = (y);    \
      __x > __y ? __x : __y;   \
    })

#define min(x, y)              \
    ({                         \
      __typeof__ __x = (x);    \
      __typeof__ __y = (y);    \
      __x < __y ? __x : __y;   \
    })
