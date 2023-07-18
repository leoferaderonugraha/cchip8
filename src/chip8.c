#include "chip8.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <time.h>

#define MAX_ROM_SIZE 3584
#define BASE_ADDR 0x200

void load_rom(Chip8* chip8, Registers* regs, const char* path)
{
    memset(chip8->memory, 0, MEM_SIZE);
    memset(chip8->gfx, 0, SCREEN_SIZE);
    memset(chip8->stack, 0, 16 * sizeof(uint16_t));

    memset(regs->v, 0, 16);

    regs->pc = BASE_ADDR;
    regs->sp = 0;
    regs->dt = 0;
    regs->i = 0;
    regs->st = 0;

    FILE *fp = fopen(path, "rb");

    if (fp == NULL)
    {
        fprintf(stderr, "Failed to open %s\n", path);
        exit(1);
    }

    fseek(fp, 0, SEEK_END);
    chip8->rom_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (chip8->rom_size > MAX_ROM_SIZE) {
        exit(1);
    }

    fread(chip8->memory + BASE_ADDR, chip8->rom_size, 1, fp);
    fclose(fp);
}

void emulate_cycle(Chip8* chip8, Registers* regs)
{
    uint16_t opcode = get_opcode(chip8, regs);
    uint8_t vx = (opcode & 0x0F00) >> 8;
    uint8_t vy = (opcode & 0x00F0) >> 4;

    uint8_t kk = opcode & 0x00FF;
    uint16_t nnn = opcode & 0x0FFF;

    switch (opcode & 0xF000) {
        case 0x0000:
        {
            switch (opcode & 0x00FF) {
                case 0x00E0:
                {
                    printf("CLS\n");

                    memset(chip8->gfx, 0, SCREEN_SIZE);
                    chip8->draw_flag = true;

                    regs->pc += 2;
                    break;
                }
                case 0x00EE:
                {
                    printf("RET\n");

                    if (regs->sp == 0) {
                        printf("Stack underflow\n");
                        return;
                    } else if (regs->sp > 16) {
                        printf("Stack overflow\n");
                        return;
                    }

                    regs->pc = chip8->stack[--regs->sp] + 2;

                    break;
                }
                default:
                {
                    printf("UNIMPLEMENTED: 0x%X\n", opcode);
                    exit(1);
                }
            }
            break;
        }
        case 0x1000:
        {
            printf("JP %4X\n", nnn);

            regs->pc = nnn;
            break;
        }
        case 0x2000:
        {
            printf("CALL %4X\n", nnn);

            chip8->stack[regs->sp++] = regs->pc;
            regs->pc = nnn;

            break;
        }
        case 0x3000:
        {
            printf("SE V%X, %X\n", vx, kk);

            regs->pc += 2;

            if (regs->v[vx] == kk) {
                regs->pc += 2;
            }

            break;
        }
        case 0x4000:
        {
            printf("SNE V%X, %X\n", vx, kk);

            regs->pc += 2;

            if (regs->v[vx] != kk) {
                regs->pc += 2;
            }

            break;
        }
        case 0x5000:
        {
            printf("SE V%X, V%X\n", vx, vy);

            regs->pc += 2;

            if (regs->v[vx] == regs->v[vy]) {
                regs->pc += 2;
            }

            break;
        }
        case 0x6000:
        {
            printf("LD V%X, %X\n", vx, kk);

            regs->v[vx] = kk;

            regs->pc += 2;

            break;
        }
        case 0x7000:
        {
            printf("ADD V%X, %X\n", vx, kk);

            regs->v[vx] += kk;

            regs->pc += 2;
            break;
        }
        case 0x8000:
        {
            switch (opcode & 0x000F) {
                case 0x0000:
                {
                    printf("LD V%X, V%X\n", vx, vy);

                    regs->v[vx] = regs->v[vy];
                    regs->pc += 2;

                    break;
                }
                case 0x0001:
                {
                    printf("OR V%X, V%X\n", vx, vy);

                    regs->v[vx] |= regs->v[vy];
                    regs->pc += 2;

                    break;
                }
                case 0x0002:
                {
                    printf("AND V%X, V%X\n", vx, vy);

                    regs->v[vx] &= regs->v[vy];
                    regs->pc += 2;

                    break;
                }
                case 0x0003:
                {
                    printf("XOR V%X, V%X\n", vx, vy);

                    regs->v[vx] ^= regs->v[vy];
                    regs->pc += 2;

                    break;
                }
                case 0x0004:
                {
                    printf("ADD V%X, V%X\n", vx, vy);

                    regs->v[0xF] = (regs->v[vx] + regs->v[vy]) > 255;

                    regs->v[vx] += regs->v[vy];
                    regs->pc += 2;

                    break;
                }
                case 0x0005:
                {
                    printf("SUB V%X, V%X\n", vx, vy);

                    regs->v[0xF] = regs->v[vx] > regs->v[vy];

                    regs->v[vx] -= regs->v[vy];
                    regs->pc += 2;

                    break;
                }
                case 0x0006:
                {
                    printf("SHR V%X {, V%X}\n", vx, vy);

                    regs->v[0xF] = regs->v[vx] & 0x0001;
                    regs->v[vx] >>= 1;
                    regs->pc += 2;

                    break;
                }
                case 0x0007:
                {
                    printf("SUBN V%X, V%X\n", vx, vy);

                    regs->v[0xF] = regs->v[vy] > regs->v[vx];
                    regs->v[vx] = regs->v[vy] - regs->v[vx];
                    regs->pc += 2;

                    break;
                }
                case 0x000E:
                {
                    printf("SHL V%X {, V%X}\n", vx, vy);

                    regs->v[0xF] = (regs->v[vx] & 0x80) >> 12;
                    regs->v[vx] <<= 1;
                    regs->pc += 2;

                    break;
                }
                default:
                {
                    // unreachable
                    printf("UNIMPLEMENTED: 0x%X\n", opcode);
                    exit(1);
                }
            }
            break;
        }
        case 0x9000:
        {
            printf("SNE V%X, V%X\n", vx, vy);

            regs->pc += 2;

            if (regs->v[vx] != regs->v[vy]) {
                regs->pc += 2;
            }

            break;
        }
        case 0xA000:
        {
            printf("LD I, %X\n", nnn);

            regs->i = nnn;
            regs->pc += 2;

            break;
        }
        case 0xC000:
        {
            printf("RND V%X, %2X", vx, kk);

            regs->v[vx] = get_rand() & kk;
            regs->pc += 2;
            break;
        }
        case 0xD000:
        {
            printf("DRW V%X, V%X, %X\n", vx, vy, opcode & 0x000F);

            uint16_t x = regs->v[vx];
            uint16_t y = regs->v[vy];
            uint16_t height = opcode & 0x000F;

            regs->v[0xF] = 0;
            for (int yline = 0; yline < height; yline++) {
                uint16_t pixel = chip8->memory[regs->i + yline];
                for(int xline = 0; xline < 8; xline++) {
                    uint32_t pos = x + xline + ((y + yline) * 64);
                    if((pixel & (0x80 >> xline)) != 0) {

                        // collide
                        if(chip8->gfx[pos] == 1) {
                            regs->v[0xF] = 1;
                        }

                        chip8->gfx[pos] ^= 1;
                    }
                }
            }

            chip8->draw_flag = true;
            regs->pc += 2;
            break;
        }
        case 0xF000:
        {
            switch (opcode & 0x00FF) {
                case 0x001E:
                {
                    printf("ADD I, V%X\n", vx);

                    regs->i += regs->v[vx];
                    regs->pc += 2;

                    break;
                }
                case 0x0033:
                {
                    printf("LD B, V%X\n", vx);

                    uint8_t n = regs->v[vx];
                    chip8->memory[regs->i]  = n / 100;
                    n /= 100;
                    chip8->memory[regs->i + 1] = n / 10;
                    n /= 10;
                    chip8->memory[regs->i + 2] = n;

                    regs->pc += 2;

                    break;
                }
                case 0x0055:
                {
                    printf("LD [I], V%X\n", vx);

                    for (size_t i = 0; i < vx; i++) {
                        chip8->memory[regs->v[i]] = regs->v[i];
                    }

                    regs->pc += 2;
                    break;
                }
                case 0x0065:
                {
                    printf("LD V%X, [I]\n", vx);

                    for (uint8_t i = 0; i < vx; i++) {
                        chip8->memory[regs->i + i] = regs->v[i];
                    }
                    regs->pc += 2;

                    break;
                }
                default:
                {
                    printf("UNIMPLEMENTED: 0x%X\n", opcode);
                    exit(1);
                }
            }
            break;
        }
        default:
        {
            printf("UNIMPLEMENTED: 0x%X\n", opcode);
            exit(1);
        }
    }
}

uint16_t get_opcode(Chip8* chip8, Registers* regs)
{
    uint16_t opcode = (chip8->memory[regs->pc] << 8) | chip8->memory[regs->pc + 1];
    return opcode;
}

uint8_t get_rand()
{
    srand(time(0));
    return rand() % 255;
}
