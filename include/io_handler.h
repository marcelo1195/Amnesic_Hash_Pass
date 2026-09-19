#ifndef AMNESIC_IO_HANDLER_H
#define AMNESIC_IO_HANDLER_H

#include <stddef.h>
#include <stdbool.h>

int read_input_vector(const char *file_path, unsigned char *buffer, size_t max_len, size_t *out_len);

#endif
