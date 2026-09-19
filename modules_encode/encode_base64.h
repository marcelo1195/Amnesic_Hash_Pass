#ifndef AMNESIC_ENCODE_BASE64_H
#define AMNESIC_ENCODE_BASE64_H

#include <stddef.h>

int encode_base64(const unsigned char *input, size_t input_len, char *output, size_t max_out_len, size_t *out_len);

#endif
