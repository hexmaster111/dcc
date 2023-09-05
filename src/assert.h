#ifndef ASSERT_H
#define ASSERT_H

#include <stdio.h>
#include <stdlib.h>

#define ASSERT(cond)                                                \
    if (!(cond))                                                    \
    {                                                               \
        fprintf(stderr, "Assertion failed: %s, file %s, line %d\n", \
                #cond, __FILE__, __LINE__);                         \
        fflush(stderr);                                             \
        abort();                                                    \
    }

#endif // ASSERT_H