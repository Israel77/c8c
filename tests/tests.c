/*
 * Versão automatizada dos testes do Timendus
 *
 * Nem todos os testes foram implementados, apenas os que
 * apontaram problemas no código original.
 * Possivelmente, implementarei mais testes no futuro.
 */
#include "../src/system.h"
#include <assert.h>
#include <stdint.h>

void test_registers(Chip8 *chip8);

int main(void) {
        Chip8 chip8 = {0};
        reset(&chip8);

        test_registers(&chip8);

        return 0;
}

void test_registers(Chip8 *chip8) {
        // Inicio dos testes
        // Testes de adição sem overflow
        set_register(chip8, 6, 255);
        add_to_register(chip8, 6, 10);
        assert(chip8->registers[6] == 9);

        set_rshift(chip8, 6);
        assert(chip8->registers[6] == 4);

        set_register(chip8, 6, 255);
        set_register(chip8, 0, 10);
        set_add(chip8, 6, 0);
        assert(chip8->registers[6] == 9);

        set_rshift(chip8, 6);
        assert(chip8->registers[6] == 4);

        // Testes dos operadores de bit-shift
        set_register(chip8, 6, 255);
        set_lshift(chip8, 6);
        set_rshift(chip8, 6);
        assert(chip8->registers[6] == 127);
        set_rshift(chip8, 6);
        set_lshift(chip8, 6);
        assert(chip8->registers[6] == 126);

        // Testes de subtração
        set_register(chip8, 6, 5);
        add_to_register(chip8, 6, 0xF6);
        assert(chip8->registers[6] == 251);

        set_register(chip8, 6, 5);
        set_sub(chip8, 6, 0);
        assert(chip8->registers[6] == 251);

        set_register(chip8, 6, 5);
        set_subn(chip8, 0, 6);
        assert(chip8->registers[0] == 251);
}