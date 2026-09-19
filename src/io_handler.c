#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "amnesic.h"
#include "io_handler.h"
#include "terminal.h"
#include "memory.h"

int read_input_vector(const char *file_path, unsigned char *buffer, size_t max_len, size_t *out_len) {
    if (!buffer || !out_len || max_len == 0) {
        return AMNESIC_ERR_ARGS;
    }

    *out_len = 0;
    size_t total_read = 0;

    if (file_path != NULL) {
        FILE *fp = fopen(file_path, "rb");
        if (!fp) {
            return AMNESIC_ERR_IO;
        }

        while (total_read < max_len) {
            size_t bytes_read = fread(buffer + total_read, 1, max_len - total_read, fp);
            if (bytes_read == 0) {
                break;
            }
            total_read += bytes_read;
        }

        /* Check if there's remaining data exceeding max_len limit */
        unsigned char dummy[1];
        if (total_read >= max_len && fread(dummy, 1, 1, fp) > 0) {
            fclose(fp);
            secure_wipe(buffer, total_read);
            return AMNESIC_ERR_IO;
        }

        fclose(fp);
        *out_len = total_read;
        return AMNESIC_SUCCESS;
    }

    bool is_interactive = isatty(STDIN_FILENO);

    if (is_interactive) {
        fprintf(stderr, "Enter input vector: ");
        fflush(stderr);
        disable_terminal_echo();
    }

    while (total_read < max_len) {
        ssize_t n = read(STDIN_FILENO, buffer + total_read, max_len - total_read);
        if (n < 0) {
            if (is_interactive) {
                restore_terminal_echo();
            }
            secure_wipe(buffer, total_read);
            return AMNESIC_ERR_IO;
        }
        if (n == 0) {
            break;
        }

        total_read += (size_t)n;

        /* Interactive input: stop reading on trailing newline */
        if (is_interactive && total_read > 0 && (buffer[total_read - 1] == '\n' || buffer[total_read - 1] == '\r')) {
            /* Trim trailing line endings from interactive entry */
            while (total_read > 0 && (buffer[total_read - 1] == '\n' || buffer[total_read - 1] == '\r')) {
                buffer[total_read - 1] = 0;
                total_read--;
            }
            break;
        }
    }

    if (is_interactive) {
        restore_terminal_echo();
    }

    /* Check if STDIN stream has leftover data beyond max_len */
    if (!is_interactive && total_read >= max_len) {
        unsigned char dummy[1];
        ssize_t extra = read(STDIN_FILENO, dummy, 1);
        if (extra > 0) {
            secure_wipe(buffer, total_read);
            return AMNESIC_ERR_IO;
        }
    }

    *out_len = total_read;
    return AMNESIC_SUCCESS;
}
