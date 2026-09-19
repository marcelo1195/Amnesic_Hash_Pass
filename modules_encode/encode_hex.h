#ifndef AMNESIC_ENCODE_HEX_H
#define AMNESIC_ENCODE_HEX_H

#include <stddef.h>

int encode_hex(const unsigned char *input, size_t input_len, char *output, size_t max_out_len, size_t *out_len);

#endif
