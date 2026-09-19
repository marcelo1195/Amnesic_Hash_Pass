#include <string.h>
#include "amnesic.h"
#include "crypto_api.h"
#include "sha256.h"
#include "sha512.h"
#include "blake3.h"
#include "memory.h"

int generate_hash(CryptoAlgo algo,
                  const unsigned char *input, size_t input_len,
                  unsigned char *output, size_t req_output_len, size_t *out_len) {
    if (!input || !output || !out_len) {
        return AMNESIC_ERR_ARGS;
    }

    switch (algo) {
        case ALGO_SHA256: {
            uint8_t digest[SHA256_DIGEST_SIZE];
            sha256_hash(input, input_len, digest);
            size_t final_len = (req_output_len > 0 && req_output_len < SHA256_DIGEST_SIZE) ? req_output_len : SHA256_DIGEST_SIZE;
            memcpy(output, digest, final_len);
            *out_len = final_len;
            secure_wipe(digest, sizeof(digest));
            return AMNESIC_SUCCESS;
        }

        case ALGO_SHA512: {
            uint8_t digest[SHA512_DIGEST_SIZE];
            sha512_hash(input, input_len, digest);
            size_t final_len = (req_output_len > 0 && req_output_len < SHA512_DIGEST_SIZE) ? req_output_len : SHA512_DIGEST_SIZE;
            memcpy(output, digest, final_len);
            *out_len = final_len;
            secure_wipe(digest, sizeof(digest));
            return AMNESIC_SUCCESS;
        }

        case ALGO_BLAKE3: {
            size_t final_len = (req_output_len > 0) ? req_output_len : BLAKE3_OUT_LEN;
            blake3_hash(input, input_len, output, final_len);
            *out_len = final_len;
            return AMNESIC_SUCCESS;
        }

        default:
            return AMNESIC_ERR_ALGO;
    }
}

size_t get_crypto_digest_size(CryptoAlgo algo) {
    switch (algo) {
        case ALGO_SHA256: return SHA256_DIGEST_SIZE;
        case ALGO_SHA512: return SHA512_DIGEST_SIZE;
        case ALGO_BLAKE3: return BLAKE3_OUT_LEN;
        default: return 0;
    }
}
