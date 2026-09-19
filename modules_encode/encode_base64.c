#include "amnesic.h"
#include "encode_base64.h"

static const char B64_CHARS[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int encode_base64(const unsigned char *input, size_t input_len, char *output, size_t max_out_len, size_t *out_len) {
    if (!input || !output || !out_len) {
        return AMNESIC_ERR_ARGS;
    }

    size_t req_len = 4 * ((input_len + 2) / 3);
    if (max_out_len <= req_len) {
        return AMNESIC_ERR_ENCODER;
    }

    size_t o = 0;
    for (size_t i = 0; i < input_len; i += 3) {
        uint32_t octet_a = input[i];
        uint32_t octet_b = (i + 1 < input_len) ? input[i + 1] : 0;
        uint32_t octet_c = (i + 2 < input_len) ? input[i + 2] : 0;

        uint32_t triple = (octet_a << 16) | (octet_b << 8) | octet_c;

        output[o++] = B64_CHARS[(triple >> 18) & 0x3F];
        output[o++] = B64_CHARS[(triple >> 12) & 0x3F];
        output[o++] = (i + 1 < input_len) ? B64_CHARS[(triple >> 6) & 0x3F] : '=';
        output[o++] = (i + 2 < input_len) ? B64_CHARS[triple & 0x3F] : '=';
    }

    output[o] = '\0';
    *out_len = o;

    return AMNESIC_SUCCESS;
}
