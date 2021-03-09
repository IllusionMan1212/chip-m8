#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#include "chip8.h"
#include "version.h"

#include <fstream>

constexpr uint32_t ON_COLOR = 0xFFFFFFFF;
constexpr uint32_t OFF_COLOR = 0x000000FF;

void poll_events(SDL_Window *window, bool *running, Chip8 &chip8) {
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
            case SDLK_1:
                chip8.keys[0] = true;
                break;
            case SDLK_2:
                chip8.keys[1] = true;
                break;
            case SDLK_3:
                chip8.keys[2] = true;
                break;
            case SDLK_4:
                chip8.keys[3] = true;
                break;
            case SDLK_q:
                chip8.keys[4] = true;
                break;
            case SDLK_w:
                chip8.keys[5] = true;
                break;
            case SDLK_e:
                chip8.keys[6] = true;
                break;
            case SDLK_r:
                chip8.keys[7] = true;
                break;
            case SDLK_a:
                chip8.keys[8] = true;
                break;
            case SDLK_s:
                chip8.keys[9] = true;
                break;
            case SDLK_d:
                chip8.keys[0xA] = true;
                break;
            case SDLK_f:
                chip8.keys[0xB] = true;
                break;
            case SDLK_z:
                chip8.keys[0xC] = true;
                break;
            case SDLK_x:
                chip8.keys[0xD] = true;
                break;
            case SDLK_c:
                chip8.keys[0xE] = true;
                break;
            case SDLK_v:
                chip8.keys[0xF] = true;
                break;
            }
            break;
        case SDL_KEYUP:
            switch (ev.key.keysym.sym) {
            case SDLK_1:
                chip8.keys[0] = false;
                break;
            case SDLK_2:
                chip8.keys[1] = false;
                break;
            case SDLK_3:
                chip8.keys[2] = false;
                break;
            case SDLK_4:
                chip8.keys[3] = false;
                break;
            case SDLK_q:
                chip8.keys[4] = false;
                break;
            case SDLK_w:
                chip8.keys[5] = false;
                break;
            case SDLK_e:
                chip8.keys[6] = false;
                break;
            case SDLK_r:
                chip8.keys[7] = false;
                break;
            case SDLK_a:
                chip8.keys[8] = false;
                break;
            case SDLK_s:
                chip8.keys[9] = false;
                break;
            case SDLK_d:
                chip8.keys[0xA] = false;
                break;
            case SDLK_f:
                chip8.keys[0xB] = false;
                break;
            case SDLK_z:
                chip8.keys[0xC] = false;
                break;
            case SDLK_x:
                chip8.keys[0xD] = false;
                break;
            case SDLK_c:
                chip8.keys[0xE] = false;
                break;
            case SDLK_v:
                chip8.keys[0xF] = false;
                break;
            }
            break;
        }
    }
}

void emulate(Chip8 &chip8, SDL_Window *window, Mix_Music *beep) {
    const uint8_t first_byte = chip8.memory[chip8.PC];
    const uint8_t second_byte = chip8.memory[chip8.PC + 1];
    const uint16_t opcode = first_byte << 8 | second_byte;

    if (chip8.delay_timer > 0) {
        // decrement at 60hz
        chip8.delay_timer--;
    }

    if (chip8.sound_timer > 0) {
        // decrement at 60hz
        // play sound
        Mix_PlayMusic(beep, 1);
        chip8.sound_timer--;
    }

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
                    int index = ((chip8.V[first_byte & 0x0F] + x) % 64) + ((chip8.V[((second_byte & 0xF0) >> 4)] + y) % 32) * 64;
                    if (chip8.pixels[index] == ON_COLOR) {
                        chip8.V[0xF] = 1;
                        chip8.pixels[index] = OFF_COLOR;
                    } else {
                        chip8.pixels[index] = ON_COLOR;
                    }
                }
            }
        }
        chip8.draw = true;
        chip8.PC += 2;
        break;
    }
    case 0xE:
        switch (second_byte) {
        case 0x9E:
            if (chip8.keys[chip8.V[first_byte & 0x0F]]) {
                chip8.PC += 2;
            }
            break;
        case 0xA1:
            if (!chip8.keys[chip8.V[first_byte & 0x0F]]) {
                chip8.PC += 2;
            }
            break;
        }
        chip8.PC += 2;
        break;
    case 0xF:
        auto X = (opcode & 0x0F00) >> 8;
        switch (second_byte) {
        case 0x07:
            chip8.V[X] = chip8.delay_timer;
            break;
        case 0x0A:
            // TODO: wait for keypress and then store it somewhere
            break;
        case 0x15:
            chip8.delay_timer = chip8.V[X];
            break;
        case 0x18:
            chip8.sound_timer = chip8.V[X];
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
        printf("No file was provided as input, quitting program\n");
        return 0;
    }

    std::ifstream file(argv[1], std::ios::binary | std::ios::ate);

    if (!file) {
        printf("Invalid file was provided, quitting program\n");
        return 0;
    }

    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    SDL_Texture *texture = nullptr;

    char title[64];
    sprintf(title, "Chip-8 v%d.%d [%s]", VERSION_MAJOR, VERSION_MINOR, argv[1]);

    Chip8 chip8;

    memcpy(chip8.memory, font, sizeof(font));
    memset(chip8.pixels, 0, sizeof(chip8.pixels));
    memset(chip8.stack, 0, sizeof(chip8.stack));
    memset(chip8.V, 0, sizeof(chip8.V));

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Video init failed\n");
        return -1;
    }

    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        printf("Audio init failed\n");
        return -1;
    }

    if (MIX_INIT_MP3 != Mix_Init(MIX_INIT_MP3)) {
        printf("Could not init audio mixer\n");
        return -1;
    }

    Mix_OpenAudio(22050, AUDIO_S16SYS, 2, 640);
    Mix_Music *beep = Mix_LoadMUS("./beep.mp3");

    srand(time(0));

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    file.read(reinterpret_cast<char *>(&chip8.memory) + 0x200, size);

    window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 320, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, 0, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, 64, 32);

    bool running = true;

    while (running) {
        emulate(chip8, window, beep);

        if (chip8.draw) {
            SDL_UpdateTexture(texture, nullptr, chip8.pixels, 64 * sizeof(uint32_t));
            SDL_RenderCopy(renderer, texture, nullptr, nullptr);
            SDL_RenderPresent(renderer);

            chip8.draw = false;
        }
        poll_events(window, &running, chip8);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_FreeMusic(beep);
    SDL_Quit();

    return 0;
}