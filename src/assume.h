#ifndef ASSUME_H
#define ASSUME_H

#include <stdio.h>
#include <stdlib.h>

#define ASSUME(cond)                                               \
    if (!(cond))                                                   \
    {                                                              \
        fprintf(stderr, "assumtion wrong: %s, file %s, line %d\n", \
                #cond, __FILE__, __LINE__);                        \
        fflush(stderr);                                            \
        abort();                                                   \
    }

#endif // ASSUME_H