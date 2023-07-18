#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>
#include <stddef.h>
#include <ctype.h>
#include <stdbool.h>

#define MEM_SIZE 4096
#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define SCREEN_SIZE SCREEN_WIDTH * SCREEN_HEIGHT


typedef struct Chip8 {
    uint8_t memory[MEM_SIZE];
    uint8_t gfx[64 * 32];
    uint16_t stack[16];

    size_t rom_size;

    bool draw_flag;
} Chip8;

typedef struct Registers {
    uint8_t v[16];
    uint16_t i;
    uint16_t pc;
    uint8_t sp;
    uint8_t dt;
    uint8_t st;
} Registers;

void load_rom(Chip8* mem, Registers* regs, const char* path);

void emulate_cycle(Chip8* mem, Registers* regs);

uint16_t get_opcode(Chip8* mem, Registers* regs);

uint8_t get_rand();

#endif  // CHIP8_H
