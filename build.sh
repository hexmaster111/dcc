#!/bin/bash
cc -ggdb -Wall src/main.c src/game.c src/global.c src/render.c -lncurses -o bin/main -lm
