#!/bin/bash
cc -ggdb -Wall mapgen_test/gen_main.c -lncurses -o bin/gen_main -lm
#if build fails, halt script
if [ $? -ne 0 ]; then
    echo "Build failed"
    exit 1
fi