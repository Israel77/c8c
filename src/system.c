#include "system.h"
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#ifndef NO_LOGGING
#include "errors.h"
#include <SDL3/SDL_log.h>
#endif

void __init_fonts(Chip8 *chip8);

void clear_display(Chip8 *chip8) {
#ifndef NO_LOGGING
        SDL_LogTrace(SDL_LOG_CATEGORY_APPLICATION, "Limpando tela\n");
#endif
        for (int i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++)
                chip8->display[i] = 0;
}

void return_from_subroutine(Chip8 *chip8) {
#ifndef NO_LOGGING
        if (chip8->stack_pointer > 0)
                SDL_LogTrace(SDL_LOG_CATEGORY_APPLICATION,
                             "Retornando para o endereço 0x%04X\n",
                             chip8->stack[chip8->stack_pointer - 1]);
#endif

        if (chip8->stack_pointer == 0) {
#ifndef NO_LOGGING
                log_error(STACK_UNDERFLOW);
#endif
                exit(EXIT_FAILURE);
        }
        chip8->stack_pointer--;
        chip8->program_counter = chip8->stack[chip8->stack_pointer];
}

void jump_to_address(Chip8 *chip8, uint16_t address) {
#ifndef NO_LOGGING
        SDL_LogTrace(SDL_LOG_CATEGORY_APPLICATION,
                     "Pulando para o endereço 0x%04X\n", address);
#endif

        chip8->program_counter = address;
}

void call_subroutine(Chip8 *chip8, uint16_t address) {
#ifndef NO_LOGGING
        SDL_LogTrace(SDL_LOG_CATEGORY_APPLICATION,
                     "Chamando subrotina para o endereço 0x%04X\n", address);
        SDL_LogTrace(SDL_LOG_CATEGORY_APPLICATION, "Retornar para: 0x%04X\n",
                     chip8->program_counter + 2);
#endif

        if (chip8->stack_pointer >= STACK_DEPTH - 1) {
                exit(EXIT_FAILURE);
        }
        chip8->stack[chip8->stack_pointer++] = chip8->program_counter + 2;
        chip8->program_counter = address;
}

void skip_if_equal(Chip8 *chip8, uint8_t reg, uint8_t value) {
#ifndef NO_LOGGING
        SDL_LogTrace(SDL_LOG_CATEGORY_APPLICATION,
                     "Pulando se V%X (0x%02X) == 0x%02X\n", reg,
                     chip8->registers[reg], value);
#endif

        chip8->program_counter += 2 * (chip8->registers[reg] == value);
}

void skip_if_not_equal(Chip8 *chip8, uint8_t reg, uint8_t value) {
#ifndef NO_LOGGING
        SDL_LogTrace(SDL_LOG_CATEGORY_APPLICATION,
                     "Pulando se V%X (0x%02X) != 0x%02X\n", reg,
                     chip8->registers[reg], value);
#endif

        chip8->program_counter += 2 * (chip8->registers[reg] != value);
}

void skip_if_equal_registers(Chip8 *chip8, uint8_t reg_x, uint8_t reg_y) {
#ifndef NO_LOGGING
        SDL_LogTrace(SDL_LOG_CATEGORY_APPLICATION,
                     "Pulando se V%X (0x%02X) == V%X (0x%02X)\n", reg_x,
                     chip8->registers[reg_x], reg_y, chip8->registers[reg_y]);
#endif

        chip8->program_counter +=
            2 * (chip8->registers[reg_x] == chip8->registers[reg_y]);
}

void set_register(Chip8 *chip8, uint8_t reg, uint8_t value) {
#ifndef NO_LOGGING
        SDL_LogTrace(SDL_LOG_CATEGORY_APPLICATION, "Definindo V%X = 0x%02X\n",
                     reg, value);
#endif

        chip8->registers[reg] = value;
}

void add_to_register(Chip8 *chip8, uint8_t reg, uint8_t value) {
#ifndef NO_LOGGING
        SDL_LogTrace(SDL_LOG_CATEGORY_APPLICATION,
                     "Adicionando 0x%02X ao V%X (0x%02X)\n", value, reg,
                     chip8->registers[reg]);
#endif

        chip8->registers[reg] += value;
}
void copy_register(Chip8 *chip8, uint8_t reg_x, uint8_t reg_y) {
#ifndef NO_LOGGING
        SDL_LogTrace(SDL_LOG_CATEGORY_APPLICATION,
                     "Copiando V%X = V%X (0x%02X)\n", reg_x, reg_y,
                     chip8->registers[reg_y]);
#endif

        chip8->registers[reg_x] = chip8->registers[reg_y];
}

void set_or(Chip8 *chip8, uint8_t reg_x, uint8_t reg_y) {
#ifndef NO_LOGGING
        SDL_LogTrace(SDL_LOG_CATEGORY_APPLICATION,
                     "Operação OR: V%X |= V%X (0x%02X | 0x%02X)\n", reg_x,
                     reg_y, chip8->registers[reg_x], chip8->registers[reg_y]);
#endif

        chip8->registers[reg_x] |= chip8->registers[reg_y];
}

void set_and(Chip8 *chip8, uint8_t reg_x, uint8_t reg_y) {
#ifndef NO_LOGGING
        SDL_LogTrace(SDL_LOG_CATEGORY_APPLICATION,
                     "Operação AND: V%X &= V%X (0x%02X & 0x%02X)\n", reg_x,
                     reg_y, chip8->registers[reg_x], chip8->registers[reg_y]);
#endif

        chip8->registers[reg_x] &= chip8->registers[reg_y];
}

void set_xor(Chip8 *chip8, uint8_t reg_x, uint8_t reg_y) {
#ifndef NO_LOGGING
        SDL_LogTrace(SDL_LOG_CATEGORY_APPLICATION,
                     "Operação XOR: V%X ^= V%X (0x%02X ^ 0x%02X)\n", reg_x,
                     reg_y, chip8->registers[reg_x], chip8->registers[reg_y]);
#endif

        chip8->registers[reg_x] ^= chip8->registers[reg_y];
}

void set_add(Chip8 *chip8, uint8_t reg_x, uint8_t reg_y) {
#ifndef NO_LOGGING
        SDL_LogTrace(SDL_LOG_CATEGORY_APPLICATION,
                     "Operação ADD: V%X += V%X (0x%02X + 0x%02X)\n", reg_x,
                     reg_y, chip8->registers[reg_x], chip8->registers[reg_y]);
#endif

        const bool carry =
            chip8->registers[reg_x] > (0xFF - chip8->registers[reg_y]);
        chip8->registers[reg_x] += chip8->registers[reg_y];
        chip8->registers[0xF] = carry;
}

void set_sub(Chip8 *chip8, uint8_t reg_x, uint8_t reg_y) {
#ifndef NO_LOGGING
        SDL_LogTrace(SDL_LOG_CATEGORY_APPLICATION,
                     "Operação SUB: V%X -= V%X (0x%02X - 0x%02X)\n", reg_x,
                     reg_y, chip8->registers[reg_x], chip8->registers[reg_y]);
#endif

        const uint8_t v_x = chip8->registers[reg_x];
        const uint8_t v_y = chip8->registers[reg_y];
        const bool not_borrow = v_x >= v_y;

        chip8->registers[reg_x] -= chip8->registers[reg_y];
        chip8->registers[0xF] = not_borrow;
}

void set_rshift(Chip8 *chip8, uint8_t reg_x) {
#ifndef NO_LOGGING
        SDL_LogTrace(SDL_LOG_CATEGORY_APPLICATION,
                     "Operação RSHIFT: V%X >>= 1 (0x%02X)\n", reg_x,
                     chip8->registers[reg_x]);
#endif

        const bool carry = chip8->registers[reg_x] & 0x1;
        chip8->registers[reg_x] >>= 1;
        chip8->registers[0xF] = carry;
}

void set_subn(Chip8 *chip8, uint8_t reg_x, uint8_t reg_y) {
#ifndef NO_LOGGING
        SDL_LogTrace(SDL_LOG_CATEGORY_APPLICATION,
                     "Operação SUBN: V%X = V%X - V%X (0x%02X - 0x%02X)\n",
                     reg_x, reg_y, reg_x, chip8->registers[reg_y],
                     chip8->registers[reg_x]);
#endif

        const uint8_t v_x = chip8->registers[reg_x];
        const uint8_t v_y = chip8->registers[reg_y];
        const bool not_borrow = v_y >= v_x;

        chip8->registers[reg_x] = v_y - v_x;
        chip8->registers[0xF] = not_borrow;
}

void set_lshift(Chip8 *chip8, uint8_t reg) {
#ifndef NO_LOGGING
        SDL_LogTrace(SDL_LOG_CATEGORY_APPLICATION,
                     "Operação LSHIFT: V%X <<= 1 (0x%02X)\n", reg,
                     chip8->registers[reg]);
#endif

        const uint8_t old_value = chip8->registers[reg];
        chip8->registers[reg] <<= 1;
        chip8->registers[0xF] = old_value > 128;
}

void skip_if_not_equal_registers(Chip8 *chip8, uint8_t reg_x, uint8_t reg_y) {
#ifndef NO_LOGGING
        SDL_LogTrace(SDL_LOG_CATEGORY_APPLICATION,
                     "Pulando se V%X != V%X (0x%02X != 0x%02X)\n", reg_x, reg_y,
                     chip8->registers[reg_x], chip8->registers[reg_y]);
#endif

        chip8->program_counter +=
            2 * (chip8->registers[reg_x] != chip8->registers[reg_y]);
}

void set_index_register(Chip8 *chip8, uint16_t address) {
#ifndef NO_LOGGING
        SDL_LogTrace(SDL_LOG_CATEGORY_APPLICATION,
                     "Definindo index para 0x%04X\n", address);
#endif

        if (address > 0xFFF) {
#ifndef NO_LOGGING
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                             "Endereço inválido: 0x%04X\n", address);
                exit(EXIT_FAILURE);
#endif
        }

        chip8->index_register = address;
}

void jump_with_offset(Chip8 *chip8, uint16_t address) {
#ifndef NO_LOGGING
        SDL_LogTrace(SDL_LOG_CATEGORY_APPLICATION,
                     "Pulando para o endereço: 0x%04X + V0 (0x%02X)\n", address,
                     chip8->registers[0]);
#endif

        chip8->index_register = address + chip8->registers[0];
}

void set_random_and(Chip8 *chip8, uint8_t reg, uint8_t value) {
#ifndef NO_LOGGING
        SDL_LogTrace(SDL_LOG_CATEGORY_APPLICATION,
                     "Definindo V%X para aleatório AND 0x%02X\n", reg, value);
#endif

        chip8->registers[reg] = (uint8_t)rand() & value;
}

void draw_sprite(Chip8 *chip8, uint8_t reg_x, uint8_t reg_y, uint8_t n) {
#ifndef NO_LOGGING
        SDL_LogTrace(
            SDL_LOG_CATEGORY_APPLICATION,
            "Desenhando sprite em V%X,V%X (0x%02X,0x%02X) com altura %d\n",
            reg_x, reg_y, chip8->registers[reg_x], chip8->registers[reg_y], n);
#endif

        chip8->registers[0xF] = false;

        for (uint8_t i = 0; i < n; ++i) {
                const uint8_t sprite_byte =
                    chip8->memory[chip8->index_register + i];
                for (uint8_t j = 0; j < 8; ++j) {
                        const bool sprite_bit = (sprite_byte >> (7 - j)) & 0x1;

                        const uint8_t x =
                            (chip8->registers[reg_x] + j) % DISPLAY_WIDTH;
                        const uint8_t y =
                            (chip8->registers[reg_y] + i) % DISPLAY_HEIGHT;

                        const uint16_t index = y * DISPLAY_WIDTH + x;

                        const bool prev_bit = chip8->display[index];

                        chip8->display[index] ^= sprite_bit;
                        // Colisão se o bit atual e anterior estão setados
                        chip8->registers[0xF] =
                            chip8->registers[0xF] || (prev_bit && sprite_bit);
                }
        }

        chip8->redraw = true;
}

void skip_if_pressed(Chip8 *chip8, uint8_t key) {
#ifndef NO_LOGGING
        SDL_LogTrace(SDL_LOG_CATEGORY_APPLICATION,
                     "Pulando se tecla %X estiver pressionada\n", key);
#endif

        chip8->program_counter += 2 * chip8->keypad[key];
}

void skip_if_not_pressed(Chip8 *chip8, uint8_t key) {
#ifndef NO_LOGGING
        SDL_LogTrace(SDL_LOG_CATEGORY_APPLICATION,
                     "Pulando se tecla %X não estiver pressionada\n", key);
#endif

        chip8->program_counter += 2 * !chip8->keypad[key];
}

void load_delay_timer_to_register(Chip8 *chip8, uint8_t reg) {
#ifndef NO_LOGGING
        SDL_LogTrace(SDL_LOG_CATEGORY_APPLICATION,
                     "Carregando delay timer para V%X\n", reg);
#endif
        chip8->registers[reg] = chip8->delay_timer;
}

void load_sound_timer_to_register(Chip8 *chip8, uint8_t reg) {
#ifndef NO_LOGGING
        SDL_LogTrace(SDL_LOG_CATEGORY_APPLICATION,
                     "Carregando sound timer para V%X\n", reg);
#endif

        chip8->registers[reg] = chip8->sound_timer;
}

bool load_key_to_register(Chip8 *chip8, uint8_t reg) {
#ifndef NO_LOGGING
        SDL_LogTrace(SDL_LOG_CATEGORY_APPLICATION,
                     "Aguardando tecla para carregar em V%X\n", reg);
#endif

        for (uint8_t i = 0; i < 16; ++i) {
                if (chip8->keypad[i]) {
                        chip8->registers[reg] = i;
                        return true;
                }
        }
        return false;
}

void set_delay_timer(Chip8 *chip8, uint8_t reg) {
#ifndef NO_LOGGING
        SDL_LogTrace(SDL_LOG_CATEGORY_APPLICATION,
                     "Definindo delay timer com valor de V%X (0x%02X)\n", reg,
                     chip8->registers[reg]);
#endif

        chip8->delay_timer = chip8->registers[reg];
}

void set_sound_timer(Chip8 *chip8, uint8_t reg) {
#ifndef NO_LOGGING
        SDL_LogTrace(SDL_LOG_CATEGORY_APPLICATION,
                     "Definindo sound timer com valor de V%X (0x%02X)\n", reg,
                     chip8->registers[reg]);
#endif

        chip8->sound_timer = chip8->registers[reg];
}

void offset_index_register(Chip8 *chip8, uint8_t reg) {
#ifndef NO_LOGGING
        SDL_LogTrace(SDL_LOG_CATEGORY_APPLICATION,
                     "Incrementando I com valor de V%X (0x%02X)\n", reg,
                     chip8->registers[reg]);
#endif

        chip8->index_register += chip8->registers[reg];
}

void load_sprite_font(Chip8 *chip8, uint8_t reg) {
#ifndef NO_LOGGING
        SDL_LogTrace(SDL_LOG_CATEGORY_APPLICATION,
                     "Carregando sprite de V%X em I\n", reg);
#endif

        chip8->index_register = FONTSET_START + chip8->registers[reg] * 5;
}

void store_bcd(Chip8 *chip8, uint8_t reg) {
#ifndef NO_LOGGING
        SDL_LogTrace(SDL_LOG_CATEGORY_APPLICATION,
                     "Armazenando BCD de V%X (%03d)\n", reg,
                     chip8->registers[reg]);
#endif
        uint8_t val = chip8->registers[reg];
        for (uint16_t i = 0; i <= 2; i++) {
                const uint16_t index = chip8->index_register + (2 - i);
                chip8->memory[index] = val % 10;
#ifndef NO_LOGGING
                SDL_LogTrace(SDL_LOG_CATEGORY_APPLICATION,
                             "Armazenando %d em 0x%02X\n", chip8->memory[index],
                             index);
#endif
                val /= 10;
        }
}

void store_registers(Chip8 *chip8, uint8_t reg_stop) {
#ifndef NO_LOGGING
        SDL_LogTrace(SDL_LOG_CATEGORY_APPLICATION,
                     "Armazenando registradores de V0 até V%X\n", reg_stop);
#endif

        for (uint16_t i = 0; i <= reg_stop; ++i) {
                chip8->memory[chip8->index_register + i] = chip8->registers[i];
        }
}

void load_to_registers(Chip8 *chip8, uint8_t reg_stop) {
#ifndef NO_LOGGING
        SDL_LogTrace(SDL_LOG_CATEGORY_APPLICATION,
                     "Carregando registradores de V0 até V%X\n", reg_stop);
#endif

        for (uint16_t i = 0; i <= reg_stop; ++i) {
                chip8->registers[i] = chip8->memory[chip8->index_register + i];
        }
}

void init(Chip8 *chip8, uint8_t *program, size_t program_size) {
        reset(chip8);

        memcpy((void *)&chip8->memory[PROGRAM_START], program, program_size);

        clear_display(chip8);
        __init_fonts(chip8);
}

void reset(Chip8 *chip8) {
        chip8->program_counter = PROGRAM_START;
        chip8->index_register = 0;
        chip8->delay_timer = 0;
        chip8->sound_timer = 0;
        chip8->stack_pointer = 0;

        for (uint8_t i = 0; i < REGISTER_COUNT; i++) {
                chip8->registers[i] = 0;
        }
        for (uint8_t i = 0; i < STACK_DEPTH; i++) {
                chip8->stack[i] = 0;
        }
        for (uint8_t i = 0; i < KEY_COUNT; i++) {
                chip8->keypad[i] = false;
        }
        for (uint16_t i = 0; i < MEMORY_SIZE; i++) {
                chip8->memory[i] = 0;
        }
}
void step(Chip8 *chip8) {
        const Instruction instruction = {
            chip8->memory[chip8->program_counter],
            chip8->memory[chip8->program_counter + 1]};

#ifndef NO_LOGGING
        SDL_LogTrace(SDL_LOG_CATEGORY_APPLICATION, "0x%04X: 0x%02X 0x%02X\n",
                     chip8->program_counter, instruction[0], instruction[1]);
#endif

        bool advance_pc = true;

        // Para instruções de 2 bytes
        const uint8_t second_byte = instruction[1];
        // Para instruções envolvendo 2 registradores
        const uint8_t first_nibble = instruction[0] >> 4;
        const uint8_t v_x = instruction[0] & 0x0F;
        const uint8_t v_y = instruction[1] >> 4;
        const uint8_t last_nibble = instruction[1] & 0x0F;
        // Para instruções na forma 1 nibble + endereço
        const uint16_t address = ((uint16_t)(v_x) << 8) | second_byte;

        switch (first_nibble) {
        case 0x0:
                switch (second_byte) {
                case 0xE0:
                        clear_display(chip8);
                        break;
                case 0xEE:
                        return_from_subroutine(chip8);
                        advance_pc = false;
                        break;
                }
                break;
        case 0x1:
                jump_to_address(chip8, address);
                advance_pc = false;
                break;
        case 0x2:
                call_subroutine(chip8, address);
                advance_pc = false;
                break;
        case 0x3:
                skip_if_equal(chip8, v_x, second_byte);
                break;
        case 0x4:
                skip_if_not_equal(chip8, v_x, second_byte);
                break;
        case 0x5:
                skip_if_equal_registers(chip8, v_x, v_y);
                break;
        case 0x6:
                set_register(chip8, v_x, second_byte);
                break;
        case 0x7:
                add_to_register(chip8, v_x, second_byte);
                break;
        case 0x8:
                switch (last_nibble) {
                case 0x0:
                        copy_register(chip8, v_x, v_y);
                        break;
                case 0x1:
                        set_or(chip8, v_x, v_y);
                        break;
                case 0x2:
                        set_and(chip8, v_x, v_y);
                        break;
                case 0x3:
                        set_xor(chip8, v_x, v_y);
                        break;
                case 0x4:
                        set_add(chip8, v_x, v_y);
                        break;
                case 0x5:
                        set_sub(chip8, v_x, v_y);
                        break;
                case 0x6:
                        set_rshift(chip8, v_x);
                        break;
                case 0x7:
                        set_subn(chip8, v_x, v_y);
                        break;
                case 0xE:
                        set_lshift(chip8, v_x);
                        break;
                }
                break;
        case 0x9:
                skip_if_not_equal_registers(chip8, v_x, v_y);
                break;
        case 0xA:
                set_index_register(chip8, address);
                break;
        case 0xB:
                jump_with_offset(chip8, address);
                break;
        case 0xC:
                set_random_and(chip8, v_x, second_byte);
                break;
        case 0xD:
                draw_sprite(chip8, v_x, v_y, last_nibble);
                break;
        case 0xE:
                switch (second_byte) {
                case 0x9E:
                        skip_if_pressed(chip8, v_x);
                        reset_keys(chip8);
                        break;
                case 0xA1:
                        skip_if_not_pressed(chip8, v_x);
                        reset_keys(chip8);
                        break;
                }
                break;
        case 0xF:
                switch (second_byte) {
                case 0x7:
                        load_delay_timer_to_register(chip8, v_x);
                        break;
                case 0xA:
                        advance_pc = load_key_to_register(chip8, v_x);
                        reset_keys(chip8);
                        break;
                case 0x15:
                        set_delay_timer(chip8, v_x);
                        break;
                case 0x18:
                        set_sound_timer(chip8, v_x);
                        break;
                case 0x1E:
                        offset_index_register(chip8, v_x);
                        break;
                case 0x29:
                        load_sprite_font(chip8, v_x);
                        break;
                case 0x33:
                        store_bcd(chip8, v_x);
                        break;
                case 0x55:
                        store_registers(chip8, v_x);
                        break;
                case 0x65:
                        load_to_registers(chip8, v_x);
                        break;
                }
                break;
        default:
#ifndef NO_LOGGING
                log_error(INVALID_INSTRUCTION);
#endif
                exit(EXIT_FAILURE);
        }
        chip8->program_counter += 2 * advance_pc;

#ifdef DEBUG
        printf("--------------------------------------------------------\n");
        printf("PC: 0x%04X\n", chip8->program_counter);
        printf("I: 0x%04X\n", chip8->index_register);
        printf("SP: 0x%04X\n", chip8->stack_pointer);
        printf("DT: 0x%04X\n", chip8->delay_timer);
        printf("ST: 0x%04X\n", chip8->sound_timer);
        printf("V0: %02X | V1: %02X | V2: %02X | V3: %02X\n",
               chip8->registers[0x0], chip8->registers[0x1],
               chip8->registers[0x2], chip8->registers[0x3]);
        printf("V4: %02X | V5: %02X | V6: %02X | V7: %02X\n",
               chip8->registers[0x4], chip8->registers[0x5],
               chip8->registers[0x6], chip8->registers[0x7]);
        printf("V8: %02X | V9: %02X | VA: %02X | VB: %02X\n",
               chip8->registers[0x8], chip8->registers[0x9],
               chip8->registers[0xA], chip8->registers[0xB]);
        printf("VC: %02X | VD: %02X | VE: %02X | VF: %02X\n",
               chip8->registers[0xC], chip8->registers[0xD],
               chip8->registers[0xE], chip8->registers[0xF]);
        printf("Call stack:\n");
        for (int i = chip8->stack_pointer - 1; i >= 0; i--) {
                printf("%2d: [0x%04X]\n", i, chip8->stack[i]);
        }
        printf("Próxima instrução: 0x%04X (%02X %02X)\n",
               chip8->program_counter, chip8->memory[chip8->program_counter],
               chip8->memory[chip8->program_counter + 1]);
        printf("--------------------------------------------------------\n");
#endif
}

void reset_keys(Chip8 *chip8) {
        for (uint8_t i = 0; i < 16; ++i) {
                chip8->keypad[i] = false;
        }
}

void __init_fonts(Chip8 *chip8) {
        static const uint8_t fontset[] = {
            0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
            0x20, 0x60, 0x20, 0x20, 0x70, // 1
            0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
            0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
            0x90, 0x90, 0xF0, 0x10, 0x10, // 4
            0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
            0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
            0xF0, 0x10, 0x20, 0x40, 0x40, // 7
            0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
            0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
            0xF0, 0x90, 0xF0, 0x90, 0x90, // A
            0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
            0xF0, 0x80, 0x80, 0x80, 0xF0, // C
            0xE0, 0x90, 0x90, 0x90, 0xE0, // D
            0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
            0xF0, 0x80, 0xF0, 0x80, 0x80, // F
        };

        static_assert(FONTSET_START <=
                          PROGRAM_START - FONTSET_COUNT * FONT_SPRITE_SIZE,
                      "O endereço de início do fontset não comporta a "
                      "quantidade de sprites");

        for (size_t i = 0; i < FONTSET_COUNT * FONT_SPRITE_SIZE; ++i) {
                chip8->memory[FONTSET_START + i] = fontset[i];
        }
}
