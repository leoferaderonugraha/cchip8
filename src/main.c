#include "chip8.h"

#include <stdio.h>
#include <SDL2/SDL.h>


int main(int argc, char** argv)
{
    if (argc != 2) {
        printf("Usage: %s <rom>\n", argv[0]);
        return 1;
    }

    Chip8 chip8;
    Registers regs;

    load_rom(&chip8, &regs, argv[1]);

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Chip8", 100, 100, 640, 320, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    if (renderer == NULL) {
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);
    if (texture == NULL) {
        printf("SDL_CreateTexture Error: %s\n", SDL_GetError());
        return 1;
    }


    uint32_t pixels[SCREEN_SIZE] = {0};

    bool paused = false;

    while (true) {

        if (!paused) {
            emulate_cycle(&chip8, &regs);
        }

        SDL_Event ev;

        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) {
                break;
            }

            if (ev.type == SDL_KEYDOWN) {
                if (ev.key.keysym.sym == SDLK_q) {
                    goto cleanup;
                } else if (ev.key.keysym.sym == SDLK_s) {
                    emulate_cycle(&chip8, &regs);
                } else if (ev.key.keysym.sym == SDLK_d) {
                    regs.pc -= 2;
                    printf("PREV: %4X\n", get_opcode(&chip8, &regs));
                    regs.pc += 2;
                    printf("CURRENT: %4X\n", get_opcode(&chip8, &regs));
                    regs.pc += 2;
                    printf("NEXT: %4X\n", get_opcode(&chip8, &regs));
                    regs.pc -= 2;
                } else if (ev.key.keysym.sym == SDLK_f) {
                    printf("Stack: \n");
                    printf("\n----------\n");
                    for (int i = 0; i < 16; i++) {
                        printf("%X ", chip8.stack[i]);
                    }
                    printf("\n----------\n");
                    printf("PC: %X\n", regs.pc);
                    printf("SP: %X\n", regs.sp);
                    printf("VAL: %X\n", chip8.stack[regs.sp]);
                } else if (ev.key.keysym.sym == SDLK_p) {
                    paused = !paused;
                }
            }
        }

        if (chip8.draw_flag) {
            for (int i = 0; i < SCREEN_SIZE; i++) {
                uint16_t pixel = chip8.gfx[i];
                pixels[i] = (0x00FFFFFF * pixel) | 0xFF000000;
            }

            SDL_UpdateTexture(texture, NULL, pixels, SCREEN_WIDTH * sizeof(uint32_t));
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, NULL, NULL);
            SDL_RenderPresent(renderer);

            chip8.draw_flag = false;

            // 60 fps?
            SDL_Delay(1000 / 60);
        }
    }

    SDL_Delay(1000);

cleanup:
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
