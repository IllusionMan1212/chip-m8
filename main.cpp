#include <SDL2/SDL.h>

#include "chip8.h"
#include "version.h"

#include <fstream>
#include <iostream>

void poll_events(SDL_Window *window, bool *running) {
    SDL_Event ev;

    while (SDL_PollEvent(&ev)) {
        switch (ev.type) {
        case SDL_QUIT:
            *running = false;
            break;
        case SDL_KEYDOWN:
            switch (ev.key.keysym.sym) {
            case SDLK_ESCAPE:
                *running = false;
                break;
            }
            break;
        case SDL_WINDOWEVENT:
            switch (ev.window.event) {
            case SDL_WINDOWEVENT_RESIZED:
                break;
            }
        }
    }
}

void emulate(Chip8 &chip8, SDL_Window *window) {
    constexpr uint32_t ON_COLOR = 0xFFFFFFFF;
    constexpr uint32_t OFF_COLOR = 0x000000FF;
    const uint8_t first_byte = chip8.memory[chip8.PC];
    const uint8_t second_byte = chip8.memory[chip8.PC + 1];
    const uint16_t opcode = first_byte << 8 | second_byte;

    printf("Executing %02X%02X \n", first_byte, second_byte);

    switch (first_byte >> 4) {
    case 0x0:
        switch (second_byte) {
        case 0xE0: {
            // clear the screen with a black color
            std::fill_n(chip8.pixels, 64 * 32, OFF_COLOR);
            chip8.PC += 2;
            break;
        }
        case 0xEE:
            // return from a subroutine
            chip8.SP--;
            chip8.PC = chip8.stack[chip8.SP];
            chip8.PC += 2;
            break;
        }
        break;
    case 0x1:
        chip8.PC = ((first_byte & 0x0F) << 8) | second_byte;
        break;
    case 0x2:
        // CALL subroutine
        chip8.stack[chip8.SP] = chip8.PC;
        chip8.SP = (chip8.SP + 1);
        chip8.PC = ((first_byte & 0x0F) << 8) | second_byte;
        break;
    case 0x3:
        if (chip8.V[first_byte & 0x0F] == second_byte) {
            chip8.PC += 2;
        }
        chip8.PC += 2;
        break;
    case 0x4:
        if (chip8.V[first_byte & 0x0F] != second_byte) {
            chip8.PC += 2;
        }
        chip8.PC += 2;
        break;
    case 0x5:
        if (chip8.V[first_byte & 0x0F] == chip8.V[(second_byte & 0xF0) >> 4]) {
            chip8.PC += 2;
        }
        chip8.PC += 2;
        break;
    case 0x6:
        chip8.V[first_byte & 0x0F] = second_byte;
        chip8.PC += 2;
        break;
    case 0x7:
        chip8.V[first_byte & 0x0F] = chip8.V[first_byte & 0x0F] + second_byte;
        chip8.PC += 2;
        break;
    case 0x8:
        switch (second_byte & 0x0F) {
        case 0x0:
            chip8.V[first_byte & 0x0F] = chip8.V[(second_byte & 0xF0) >> 4];
            chip8.PC += 2;
            break;
        case 0x1:
            chip8.V[first_byte & 0x0F] = chip8.V[first_byte & 0x0F] | chip8.V[(second_byte & 0xF0) >> 4];
            chip8.PC += 2;
            break;
        case 0x2:
            chip8.V[first_byte & 0x0F] = chip8.V[first_byte & 0x0F] & chip8.V[(second_byte & 0xF0) >> 4];
            chip8.PC += 2;
            break;
        case 0x3:
            chip8.V[first_byte & 0x0F] = chip8.V[first_byte & 0x0F] ^ chip8.V[(second_byte & 0xF0) >> 4];
            chip8.PC += 2;
            break;
        case 0x4:
            if ((chip8.V[first_byte & 0x0F] + chip8.V[(second_byte & 0xF0) >> 4]) > 0xFF) {
                chip8.V[0xF] = 1;
            } else {
                chip8.V[0xF] = 0;
            }
            chip8.V[first_byte & 0x0F] = chip8.V[first_byte & 0x0F] + chip8.V[(second_byte & 0xF0) >> 4];
            chip8.PC += 2;
            break;
        case 0x5:
            // set Vx = Vx - Vy, set VF = NOT borrow
            if (chip8.V[first_byte & 0x0F] > chip8.V[(second_byte & 0xF0) >> 4]) {
                chip8.V[0xF] = 1;
            } else {
                chip8.V[0xF] = 0;
            }
            chip8.V[first_byte & 0x0F] = chip8.V[first_byte & 0x0F] - chip8.V[(second_byte & 0xF0) >> 4];
            chip8.PC += 2;
            break;
        case 0x6:
            if (chip8.V[first_byte & 0x0F] & 1 == 1) {
                chip8.V[0xF] = 1;
            } else {
                chip8.V[0xF] = 0;
            }
            chip8.V[first_byte & 0x0F] = chip8.V[first_byte & 0x0F] >> 1;
            chip8.PC += 2;
            break;
        case 0x7:
            // set Vx = Vy - Vx, set VF = NOT borrow
            if (chip8.V[(second_byte & 0xF0) >> 4] > chip8.V[first_byte & 0x0F]) {
                chip8.V[0xF] = 1;
            } else {
                chip8.V[0xF] = 0;
            }
            chip8.V[first_byte & 0x0F] = chip8.V[(second_byte & 0xF0) >> 4] - chip8.V[first_byte & 0x0F];
            chip8.PC += 2;
            break;
        case 0xE:
            chip8.V[0xF] = (chip8.V[(opcode & 0x0F00) >> 8] >> 7) & 1;
            chip8.V[first_byte & 0x0F] = chip8.V[first_byte & 0x0F] << 1;
            chip8.PC += 2;
            break;
        }
        break;
    case 0x9:
        if (chip8.V[first_byte & 0x0F] != chip8.V[(second_byte & 0xF0) >> 4]) {
            chip8.PC += 2;
        }
        chip8.PC += 2;
        break;
    case 0xA:
        chip8.I = ((first_byte & 0x0F) << 8) | second_byte;
        chip8.PC += 2;
        break;
    case 0xB:
        chip8.PC = (((first_byte & 0x0F) << 8) | second_byte) + chip8.V[0];
        break;
    case 0xC:
        chip8.V[first_byte & 0x0F] = (rand() & 0x00FF) & second_byte;
        chip8.PC += 2;
        break;
    case 0xD: {
        int height = second_byte & 0x0F;
        chip8.V[0xF] = 0;
        for (int y = 0; y < height; y++) {
            uint8_t pixel = chip8.memory[chip8.I + y];
            for (int x = 0; x < 8; x++) {
                if (pixel & (0x80 >> x)) {
                    // check if we're turning off the pixel and set VF.
                    int index = (chip8.V[first_byte & 0x0F] + x) % 64 + ((chip8.V[((second_byte & 0xF0) >> 4)] + y) % 32) * 64;
                    if (chip8.pixels[index] == ON_COLOR) {
                        chip8.V[0xF] = 1;
                        chip8.pixels[index] = OFF_COLOR;
                    }
                    chip8.pixels[index] = ON_COLOR;
                }
            }
        }
        chip8.draw = true;
        chip8.PC += 2;
        break;
    }
    case 0xE:
        // TODO:
        chip8.PC += 2;
        break;
    case 0xF:
        auto X = (opcode & 0x0F00) >> 8;
        switch (second_byte) {
        case 0x07:
            // TODO:
            break;
        case 0x0A:
            // TODO:
            break;
        case 0x15:
            // TODO:
            break;
        case 0x18:
            // TODO:
            break;
        case 0x1E:
            chip8.I = chip8.I + chip8.V[X];
            break;
        case 0x29:
            chip8.I = chip8.V[X] * 5;
            break;
        case 0x33:
            chip8.memory[chip8.I] = (chip8.V[X]) / 100;
            chip8.memory[chip8.I + 1] = (chip8.V[X] % 100) / 10;
            chip8.memory[chip8.I + 2] = chip8.V[X] % 10;
            break;
        case 0x55: {
            for (int i = 0; i <= X; i++) {
                chip8.memory[chip8.I + i] = chip8.V[i];
            }
            break;
        }
        case 0x65: {
            for (int i = 0; i <= X; i++) {
                chip8.V[i] = chip8.memory[chip8.I + i];
            }
            break;
        }
        }
        chip8.PC += 2;
        break;
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cout << "No file was provided as input, quitting program" << std::endl;
        return 0;
    }

    std::ifstream file(argv[1], std::ios::binary | std::ios::ate);

    if (!file) {
        std::cout << "Invalid file was provided, quitting program" << std::endl;
        return 0;
    }

    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    SDL_Texture *texture = nullptr;

    char title[64];
    sprintf(title, "Chip-8 v%d.%d [%s]", VERSION_MAJOR, VERSION_MINOR, argv[1]);

    Chip8 chip8;

    memcpy(chip8.memory, font, sizeof(font));
    memset(chip8.pixels, 0xFF000000, sizeof(chip8.pixels));
    memset(chip8.stack, 0, sizeof(chip8.stack));
    memset(chip8.V, 0, sizeof(chip8.V));

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "Video init failed";
        return -1;
    }
    
    srand(time(0));

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    file.read(reinterpret_cast<char *>(&chip8.memory) + 0x200, size);

    window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 320, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, 0, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_BGRA8888, SDL_TEXTUREACCESS_STATIC, 64, 32);

    bool running = true;

    while (running) {
        emulate(chip8, window);

        if (chip8.draw) {
            SDL_UpdateTexture(texture, 0, chip8.pixels, 64 * sizeof(uint32_t));
            SDL_RenderCopy(renderer, texture, 0, 0);
            SDL_RenderPresent(renderer);

            chip8.draw = false;
        }
        poll_events(window, &running);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}