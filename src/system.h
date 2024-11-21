#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef SYSTEM_H
#define SYSTEM_H

#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32
#define REGISTER_COUNT 16
#define MEMORY_SIZE 0xFFF
#define STACK_DEPTH 16
#define KEY_COUNT 16

// Endereço onde serão armazenados os sprites dos caracteres
#define FONTSET_START 0x00
// Endereço onde se inicia o programa carregado
#define PROGRAM_START 0x200
// Número de caracteres pré-definidos
#define FONTSET_COUNT 16
// Quantidade de bytes para cada caractere
#define FONT_SPRITE_SIZE 5

typedef uint8_t Instruction[2];

typedef struct {
        uint16_t program_counter;
        uint16_t index_register;
        uint8_t registers[REGISTER_COUNT];
        uint8_t memory[MEMORY_SIZE];
        uint16_t stack[STACK_DEPTH];
        uint8_t stack_pointer;
        uint8_t delay_timer;
        uint8_t sound_timer;
        bool keypad[KEY_COUNT];
        bool display[DISPLAY_WIDTH * DISPLAY_HEIGHT];
        bool redraw;
        uint8_t op_code;
} Chip8;

void clear_display(Chip8 *chip8);
void return_from_subroutine(Chip8 *chip8);
void jump_to_address(Chip8 *chip8, uint16_t address);
void call_subroutine(Chip8 *chip8, uint16_t address);
void skip_if_equal(Chip8 *chip8, uint8_t reg, uint8_t value);
void skip_if_not_equal(Chip8 *chip8, uint8_t reg, uint8_t value);
void skip_if_equal_registers(Chip8 *chip8, uint8_t reg_x, uint8_t reg_y);
void set_register(Chip8 *chip8, uint8_t reg, uint8_t value);
void add_to_register(Chip8 *chip8, uint8_t reg, uint8_t value);
void copy_register(Chip8 *chip8, uint8_t reg_x, uint8_t reg_y);
void set_or(Chip8 *chip8, uint8_t reg_x, uint8_t reg_y);
void set_and(Chip8 *chip8, uint8_t reg_x, uint8_t reg_y);
void set_xor(Chip8 *chip8, uint8_t reg_x, uint8_t reg_y);
void set_add(Chip8 *chip8, uint8_t reg_x, uint8_t reg_y);
void set_sub(Chip8 *chip8, uint8_t reg_x, uint8_t reg_y);
void set_rshift(Chip8 *chip8, uint8_t reg_x);
void set_subn(Chip8 *chip8, uint8_t reg_x, uint8_t reg_y);
void set_lshift(Chip8 *chip8, uint8_t reg_x);
void skip_if_not_equal_registers(Chip8 *chip8, uint8_t reg_x, uint8_t reg_y);
void set_index_register(Chip8 *chip8, uint16_t address);
void jump_with_offset(Chip8 *chip8, uint16_t address);
void set_random_and(Chip8 *chip8, uint8_t reg, uint8_t value);
void draw_sprite(Chip8 *chip8, uint8_t reg_x, uint8_t reg_y, uint8_t n);
void skip_if_pressed(Chip8 *chip8, uint8_t key);
void skip_if_not_pressed(Chip8 *chip8, uint8_t key);
void load_delay_timer_to_register(Chip8 *chip8, uint8_t reg);
bool load_key_to_register(Chip8 *chip8, uint8_t reg);
void set_delay_timer(Chip8 *chip8, uint8_t reg);
void set_sound_timer(Chip8 *chip8, uint8_t reg);
void offset_index_register(Chip8 *chip8, uint8_t reg);
void load_sprite_font(Chip8 *chip8, uint8_t reg);
void store_bcd(Chip8 *chip8, uint8_t reg);
void store_registers(Chip8 *chip8, uint8_t reg_stop);
void load_to_registers(Chip8 *chip8, uint8_t reg_stop);

void init(Chip8 *chip8, uint8_t *program, size_t program_size);
void reset(Chip8 *chip8);
void step(Chip8 *chip8);
void reset_keys(Chip8 *chip8);

#endif