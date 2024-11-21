#include "errors.h"

void log_error(ErrorCode error_code) {
        switch (error_code) {
        case NOT_IMPLEMENTED_YET:
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                             "Not implemented yet");
                break;
        case STACK_UNDERFLOW:
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Stack underflow");
                break;
        case STACK_OVERFLOW:
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Stack overflow");
                break;
        case INVALID_INSTRUCTION:
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                             "Invalid instruction");
                break;
        case NO_FILE_PROVIDED:
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "No file provided");
                break;
        case INVALID_MEMORY_ADDRESS:
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                             "Invalid memory address");
                break;
        default:
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unknown error");
                break;
        }
};