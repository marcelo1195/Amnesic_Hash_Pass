#include <string.h>
#include "amnesic.h"
#include "crypto_api.h"
#include "blake3.h"
#include "sha256.h"
#include "sha512.h"
#include "memory.h"

int execute_mode_explode(CryptoAlgo algo,
                         const unsigned char *input, size_t input_len,
                         size_t req_length,
                         unsigned char *output, size_t max_out_len, size_t *out_len) {
    if (!input || !output || !out_len) {
        return AMNESIC_ERR_ARGS;
    }

    size_t target_len = (req_length > 0) ? req_length : 64;
    if (target_len > max_out_len) {
        target_len = max_out_len;
    }

    if (algo == ALGO_BLAKE3) {
        blake3_hash(input, input_len, output, target_len);
        *out_len = target_len;
        return AMNESIC_SUCCESS;
    }

    if (algo == ALGO_SHA256) {
        uint8_t current[SHA256_DIGEST_SIZE];
        sha256_hash(input, input_len, current);

        /* Perform 50,000 key stretching rounds */
        for (int r = 0; r < 50000; r++) {
            uint8_t temp[SHA256_DIGEST_SIZE];
            sha256_hash(current, SHA256_DIGEST_SIZE, temp);
            memcpy(current, temp, SHA256_DIGEST_SIZE);
            secure_wipe(temp, sizeof(temp));
        }

        size_t copy_len = (target_len < SHA256_DIGEST_SIZE) ? target_len : SHA256_DIGEST_SIZE;
        memcpy(output, current, copy_len);
        *out_len = copy_len;
        secure_wipe(current, sizeof(current));
        return AMNESIC_SUCCESS;
    }

    if (algo == ALGO_SHA512) {
        uint8_t current[SHA512_DIGEST_SIZE];
        sha512_hash(input, input_len, current);

        /* Perform 50,000 key stretching rounds */
        for (int r = 0; r < 50000; r++) {
            uint8_t temp[SHA512_DIGEST_SIZE];
            sha512_hash(current, SHA512_DIGEST_SIZE, temp);
            memcpy(current, temp, SHA512_DIGEST_SIZE);
            secure_wipe(temp, sizeof(temp));
        }

        size_t copy_len = (target_len < SHA512_DIGEST_SIZE) ? target_len : SHA512_DIGEST_SIZE;
        memcpy(output, current, copy_len);
        *out_len = copy_len;
        secure_wipe(current, sizeof(current));
        return AMNESIC_SUCCESS;
    }

    return AMNESIC_ERR_ALGO;
}
