#include "errors.h"
#include "system.h"
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define SCREEN_SCALE 20
// Intervalo para os timers de 60Hz
const uint64_t TIMER_INTERVAL = 1000000 / 60;
// Intervalo simulando o clock do Chip8
const uint64_t UPDATE_INTERVAL = 1000000 / 500;

typedef struct {
        SDL_Window *window;
        SDL_Renderer *renderer;
        SDL_FRect display_pixels[DISPLAY_WIDTH * DISPLAY_HEIGHT];
        Chip8 *chip8;
        // Marca se o interpretador deve avançar o programa
        uint64_t timestamp_update;
        uint64_t timestamp_delay;
        uint64_t timestamp_sound;
        bool update;
        bool quit;
} AppContext;

typedef struct {
        char *filename;
        SDL_LogPriority log_priority;
} CliArguments;

// Initialização
void init_app(AppContext *app_context, CliArguments *cli_arguments);
CliArguments parse_arguments(int argc, char *argv[]);
void load_instructions(Chip8 *chip8, char *filename);
// Funções principais do interpretador
void run_interpreter_loop(AppContext *app_context);
void handle_events(AppContext *app_context);
void render(AppContext *app_context);
void try_match_key(Chip8 *chip8, SDL_Keycode key);
// Funções associadas aos timers
void update_timers(AppContext *app_context);

int main(int argc, char *argv[]) {
        CliArguments cli_arguments = parse_arguments(argc, argv);
        AppContext app_context;

        init_app(&app_context, &cli_arguments);

        run_interpreter_loop(&app_context);

        SDL_Quit();
        return 0;
}

CliArguments parse_arguments(int argc, char *argv[]) {
        CliArguments cli_arguments;
        cli_arguments.log_priority = SDL_LOG_PRIORITY_INFO;

        for (int i = 0; i < argc; i++) {
                if (strncmp(argv[i], "-vv", 3) == 0) {
                        cli_arguments.log_priority = SDL_LOG_PRIORITY_TRACE;
                } else if (strncmp(argv[i], "-v", 2) == 0) {
                        cli_arguments.log_priority = SDL_LOG_PRIORITY_VERBOSE;
                } else {
                        cli_arguments.filename = argv[i];
                }
        }
        return cli_arguments;
}

void init_app(AppContext *app_context, CliArguments *cli_arguments) {
        app_context->chip8 = malloc(sizeof(Chip8));
        app_context->quit = false;
        app_context->update = true;

        SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
        SDL_CreateWindowAndRenderer("CHIP-8", DISPLAY_WIDTH * SCREEN_SCALE,
                                    DISPLAY_HEIGHT * SCREEN_SCALE, 0,
                                    &app_context->window,
                                    &app_context->renderer);
        SDL_SetLogPriority(SDL_LOG_CATEGORY_APPLICATION,
                           cli_arguments->log_priority);

        load_instructions(app_context->chip8, cli_arguments->filename);

        SDL_ShowWindow(app_context->window);
}

void run_interpreter_loop(AppContext *app_context) {
        while (!app_context->quit) {
                handle_events(app_context);

                update_timers(app_context);
                if (app_context->update) {
                        step(app_context->chip8);
                        app_context->update = false;
                }

                if (app_context->chip8->redraw) {
                        render(app_context);
                }
        }
}

void update_timers(AppContext *app_context) {
        const uint64_t now = SDL_GetTicksNS();

        if (now - app_context->timestamp_update > UPDATE_INTERVAL) {
                app_context->timestamp_update = now;
#ifndef DEBUG
                app_context->update = true;
#endif
        }

        if (app_context->chip8->delay_timer > 0 &&
            now - app_context->timestamp_delay > TIMER_INTERVAL) {
                app_context->timestamp_delay = now;
                app_context->chip8->delay_timer--;
        }

        if (app_context->chip8->sound_timer > 0 &&
            now - app_context->timestamp_sound > TIMER_INTERVAL) {
                app_context->timestamp_sound = now;
                app_context->chip8->sound_timer--;
        }
}

void handle_events(AppContext *app_context) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
                switch (event.type) {
                case SDL_EVENT_QUIT:
                        app_context->quit = true;
                        break;
                case SDL_EVENT_KEY_DOWN:
                        try_match_key(app_context->chip8, event.key.key);

                        if (event.key.key == SDLK_ESCAPE) {
                                app_context->quit = true;
                        }
#ifdef DEBUG
                        if (event.key.key == SDLK_SPACE) {
                                app_context->update = true;
                        }
#endif
                        break;
                default:
                        break;
                }
        }
}

void try_match_key(Chip8 *chip8, SDL_Keycode key) {
        SDL_Keycode emulated_keys[16] = {SDLK_X, SDLK_1, SDLK_2, SDLK_3, //
                                         SDLK_Q, SDLK_W, SDLK_E, SDLK_A, //
                                         SDLK_S, SDLK_D, SDLK_Z, SDLK_C, //
                                         SDLK_4, SDLK_R, SDLK_F, SDLK_V};

        for (uint8_t i = 0; i < 16; ++i) {
                chip8->keypad[i] = (key == emulated_keys[i]);
        }
}

void render(AppContext *app_context) {
        uint16_t n_pixels = 0;

        for (uint16_t x = 0; x < DISPLAY_WIDTH; ++x) {
                for (uint16_t y = 0; y < DISPLAY_HEIGHT; ++y) {
                        const uint16_t index = (y * DISPLAY_WIDTH) + x;
                        const SDL_FRect rect = {x * SCREEN_SCALE,
                                                y * SCREEN_SCALE, SCREEN_SCALE,
                                                SCREEN_SCALE};
                        if (app_context->chip8->display[index] > 0) {
                                app_context->display_pixels[n_pixels++] = rect;
                        }
                }
        }
        SDL_SetRenderDrawColor(app_context->renderer, 51, 51, 51, 0xFF);
        SDL_RenderClear(app_context->renderer);

        SDL_SetRenderDrawColor(app_context->renderer, 0xFF, 0xFF, 0xFF, 0xFF);

        SDL_LogTrace(SDL_LOG_CATEGORY_RENDER, "Renderizando %d pixels\n",
                     n_pixels);

        SDL_RenderFillRects(app_context->renderer, app_context->display_pixels,
                            n_pixels);
        SDL_RenderPresent(app_context->renderer);

        app_context->chip8->redraw = false;
}

void load_instructions(Chip8 *chip8, char *filename) {
        uint8_t *program =
            malloc(sizeof(uint8_t) * (MEMORY_SIZE - PROGRAM_START));
        size_t offset = 0;

        FILE *fileptr = fopen(filename, "rb");
        while (!feof(fileptr)) {
                uint8_t byte;
                fread(&byte, sizeof(uint8_t), 1, fileptr);
                program[offset++] = byte;
        }
        fclose(fileptr);

        init(chip8, program, offset);
        free(program);
}