#ifndef AMNESIC_ENCODE_BASE85_H
#define AMNESIC_ENCODE_BASE85_H

#include <stddef.h>

int encode_base85(const unsigned char *input, size_t input_len, char *output, size_t max_out_len, size_t *out_len);

#endif
