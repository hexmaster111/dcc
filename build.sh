#!/bin/bash
cc -ggdb -Wall src/main.c src/game.c src/global.c -lncurses -o bin/main
