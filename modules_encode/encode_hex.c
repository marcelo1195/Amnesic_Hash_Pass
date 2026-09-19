#include "amnesic.h"
#include "encode_hex.h"

static const char HEX_CHARS[] = "0123456789abcdef";

int encode_hex(const unsigned char *input, size_t input_len, char *output, size_t max_out_len, size_t *out_len) {
    if (!input || !output || !out_len) {
        return AMNESIC_ERR_ARGS;
    }

    size_t req_len = input_len * 2;
    if (max_out_len <= req_len) {
        return AMNESIC_ERR_ENCODER;
    }

    for (size_t i = 0; i < input_len; i++) {
        output[i * 2]     = HEX_CHARS[(input[i] >> 4) & 0x0F];
        output[i * 2 + 1] = HEX_CHARS[input[i] & 0x0F];
    }
    output[req_len] = '\0';
    *out_len = req_len;

    return AMNESIC_SUCCESS;
}
