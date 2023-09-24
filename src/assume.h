#ifndef ASSUME_H
#define ASSUME_H

#include <stdio.h>
#include <stdlib.h>
#include "global.h"

#define ASSUME(cond)                                               \
    if (!(cond))                                                   \
    {                                                              \
        fprintf(stderr, "!!GAME CRASH!!\n");                       \
        fprintf(stderr, "assumtion wrong: %s, file %s, line %d\n", \
                #cond, __FILE__, __LINE__);                        \
        fflush(stderr);                                            \
        glog_printf("assumtion wrong: %s, file %s, line %d\n",     \
                    #cond, __FILE__, __LINE__);                    \
        glog_destroy();                                            \
        abort();                                                   \
    }

#endif // ASSUME_H