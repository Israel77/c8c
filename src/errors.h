#include <SDL3/SDL_log.h>

typedef enum {
        NOT_IMPLEMENTED_YET,
        STACK_UNDERFLOW,
        STACK_OVERFLOW,
        INVALID_INSTRUCTION,
        NO_FILE_PROVIDED,
        INVALID_MEMORY_ADDRESS,
} ErrorCode;

void log_error(ErrorCode error_code);