#include <stdint.h>

#ifndef EMULATOR_H
#define EMULATOR_H

typedef struct {
        uint16_t programCounter;
        uint16_t indexRegister;
        uint8_t registers[16];
        uint8_t memory[0xFF];
        uint8_t stack[16];
        uint8_t stackPointer;
        uint8_t delayTimer;
        uint8_t soundTimer;
        uint8_t keypad;
        uint32_t video[64 * 32];
        uint8_t opCode;
} CHIP_8;

#endif