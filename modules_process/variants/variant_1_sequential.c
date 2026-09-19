#include <string.h>
#include "amnesic.h"
#include "sha256.h"
#include "memory.h"

int variant_1_sequential(const unsigned char *input, size_t input_len,
                         unsigned char *output, size_t max_out_len, size_t *out_len) {
    if (!input || !output || !out_len || max_out_len < SHA256_DIGEST_SIZE) {
        return AMNESIC_ERR_ARGS;
    }

    uint8_t buffer[SHA256_DIGEST_SIZE];
    sha256_hash(input, input_len, buffer);

    /* Perform 100 sequential rounds */
    for (int r = 0; r < 100; r++) {
        uint8_t temp[SHA256_DIGEST_SIZE];
        sha256_hash(buffer, SHA256_DIGEST_SIZE, temp);
        memcpy(buffer, temp, SHA256_DIGEST_SIZE);
        secure_wipe(temp, sizeof(temp));
    }

    memcpy(output, buffer, SHA256_DIGEST_SIZE);
    *out_len = SHA256_DIGEST_SIZE;
    secure_wipe(buffer, sizeof(buffer));

    return AMNESIC_SUCCESS;
}
