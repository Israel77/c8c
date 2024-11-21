// nob.c
#define NOB_IMPLEMENTATION
#include "nob.h"

int main(int argc, char **argv) {
        NOB_GO_REBUILD_URSELF(argc, argv);
        Nob_Cmd cmd = {0};

        // Create build directory
        if (!nob_mkdir_if_not_exists("bin/"))
                return 1;
        if (!nob_mkdir_if_not_exists("bin/tests"))
                return 1;

        // Basic compiler options
        nob_cmd_append(&cmd, "cc", "-Wall", "-Wextra", "-o", "bin/c8c");
        // Files to compile
        nob_cmd_append(&cmd, "src/main.c", "src/system.c", "src/errors.c");
        // SDL3 flags
        nob_cmd_append(&cmd, "-I/usr/local/lib64/pkgconfig/../../include",
                       "-L/usr/local/lib64/pkgconfig/../../lib64",
                       "-Wl,-rpath,/usr/local/lib64/pkgconfig/../../lib64",
                       "-Wl,--enable-new-dtags", "-lSDL3");
        if (!nob_cmd_run_sync_and_reset(&cmd))
                return 1;

        nob_cmd_append(&cmd, "cc", "-Wall", "-Wextra", "-o", "bin/tests/tests");
        nob_cmd_append(&cmd, "-DNO_LOGGING");
        nob_cmd_append(&cmd, "tests/tests.c", "src/system.c");
        if (!nob_cmd_run_sync_and_reset(&cmd))
                return 1;

        nob_cmd_append(&cmd, "bin/tests/tests");
        if (!nob_cmd_run_sync_and_reset(&cmd)) {
                nob_log(NOB_ERROR, "Tests failed\n");
                return 1;
        }

        return 0;
}