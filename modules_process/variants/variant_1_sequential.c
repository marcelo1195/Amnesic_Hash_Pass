#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "amnesic.h"
#include "crypto_api.h"
#include "encode_hex.h"
#include "memory.h"

int variant_1_sequential(CryptoAlgo algo, const unsigned char *input, size_t input_len,
                         unsigned char *output, size_t max_out_len, size_t *out_len) {
    if (!input || !output || !out_len || input_len == 0) {
        return AMNESIC_ERR_ARGS;
    }

    size_t digest_size = get_crypto_digest_size(algo);
    if (digest_size == 0 || max_out_len < digest_size) {
        return AMNESIC_ERR_ARGS;
    }

    /* Each character hash produces a hex string + Unix newline '\n' */
    size_t hex_line_len = (digest_size * 2) + 1;
    size_t total_block_len = input_len * hex_line_len;

    unsigned char *block_buffer = (unsigned char *)malloc(total_block_len);
    if (!block_buffer) {
        return AMNESIC_ERR_MEMORY;
    }
    secure_wipe(block_buffer, total_block_len);

    size_t current_offset = 0;
    for (size_t i = 0; i < input_len; i++) {
        unsigned char char_digest[64];
        size_t char_digest_len = 0;
        secure_wipe(char_digest, sizeof(char_digest));

        /* Step 2: Hash individual character */
        int ret = generate_hash(algo, &input[i], 1, char_digest, digest_size, &char_digest_len);
        if (ret != AMNESIC_SUCCESS) {
            secure_wipe(block_buffer, total_block_len);
            free(block_buffer);
            secure_wipe(char_digest, sizeof(char_digest));
            return ret;
        }

        /* Format digest as lowercase hexadecimal string */
        char hex_str[129];
        size_t hex_len = 0;
        secure_wipe(hex_str, sizeof(hex_str));

        ret = encode_hex(char_digest, char_digest_len, hex_str, sizeof(hex_str), &hex_len);
        secure_wipe(char_digest, sizeof(char_digest));

        if (ret != AMNESIC_SUCCESS) {
            secure_wipe(block_buffer, total_block_len);
            free(block_buffer);
            secure_wipe(hex_str, sizeof(hex_str));
            return ret;
        }

        /* Step 3: Append hex string and single '\n' to block buffer */
        memcpy(block_buffer + current_offset, hex_str, hex_len);
        current_offset += hex_len;
        block_buffer[current_offset] = '\n';
        current_offset += 1;

        secure_wipe(hex_str, sizeof(hex_str));
    }

    /* Step 4: Calculate final hash of the assembled text block */
    int ret = generate_hash(algo, block_buffer, current_offset, output, max_out_len, out_len);

    /* Zeroize temporary memory block */
    secure_wipe(block_buffer, total_block_len);
    free(block_buffer);

    return ret;
}
