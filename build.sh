#!/bin/bash

gcc src/*.c -o chip8 -lSDL2 -Wall -Wextra -Wpedantic -Werror
gcc src/disasm/main.c -o disasm
