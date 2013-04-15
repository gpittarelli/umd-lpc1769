#ifndef __UMDLPC_util_util_h_
#define __UMDLPC_util_util_h_

#define ARRAY_LEN(a) (sizeof(a)/sizeof((a)[0]))

#define MAX(a,b) \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
       _a > _b ? _a : _b; })

#define MIN(a,b) \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
       _a < _b ? _a : _b; })

#endif
