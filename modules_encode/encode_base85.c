#include <stdint.h>
#include "amnesic.h"
#include "encode_base85.h"

int encode_base85(const unsigned char *input, size_t input_len, char *output, size_t max_out_len, size_t *out_len) {
    if (!input || !output || !out_len) {
        return AMNESIC_ERR_ARGS;
    }

    size_t full_chunks = input_len / 4;
    size_t remainder = input_len % 4;
    size_t req_len = full_chunks * 5 + (remainder ? (remainder + 1) : 0);

    if (max_out_len <= req_len) {
        return AMNESIC_ERR_ENCODER;
    }

    size_t o = 0;
    for (size_t i = 0; i < full_chunks; i++) {
        uint32_t val = ((uint32_t)input[i * 4] << 24) |
                       ((uint32_t)input[i * 4 + 1] << 16) |
                       ((uint32_t)input[i * 4 + 2] << 8) |
                       ((uint32_t)input[i * 4 + 3]);

        char block[5];
        for (int j = 4; j >= 0; j--) {
            block[j] = (char)((val % 85) + 33);
            val /= 85;
        }

        for (int j = 0; j < 5; j++) {
            output[o++] = block[j];
        }
    }

    if (remainder > 0) {
        uint32_t val = 0;
        for (size_t j = 0; j < remainder; j++) {
            val |= ((uint32_t)input[full_chunks * 4 + j] << (24 - j * 8));
        }

        char block[5];
        for (int j = 4; j >= 0; j--) {
            block[j] = (char)((val % 85) + 33);
            val /= 85;
        }

        for (size_t j = 0; j < remainder + 1; j++) {
            output[o++] = block[j];
        }
    }

    output[o] = '\0';
    *out_len = o;

    return AMNESIC_SUCCESS;
}
