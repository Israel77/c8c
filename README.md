# C8C

A simple chip 8 interpreter written in C.

## Compiling

### Requirements
- SDL3
- C compiler*

* Only tested on linux with gcc and clang.

Run the following command to bootstrap nob (you can replace cc with your compiler of choice):
```sh
cc -o nob nob.c
```
The nob executable will be used to compile the remaining of the project. After that, run the following command to compile the project:
```sh
./nob
```
The resulting executable will be in the bin/ directory, at `bin/c8c`.

## Usage
After compiling, you can run the following command to run the interpreter:
```sh
./bin/c8c <rom>.ch8
```

### Test suite
This project includes on its source code a copy of the excellent Timendus' [Chip 8 test suite](https://github.com/Timendus/chip8-test-suite). This suite was used to test the interpreter. You can find the roms and source code in the tests/timendus/ directory. A partial implementation of some of the tests as C code is also included in the tests/ directory and is run as part of the nob script. However, there are very few automatic tests implemented as code, as I only bothered to implement the ones that gave me trouble after I did my first implementation.