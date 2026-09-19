#include <string.h>
#include "amnesic.h"
#include "crypto_api.h"
#include "sha512.h"
#include "memory.h"

int variant_2_xor(CryptoAlgo algo, const unsigned char *input, size_t input_len,
                  unsigned char *output, size_t max_out_len, size_t *out_len) {
    (void)algo; /* Currently operates on SHA-512 bifurcation */
    if (!input || !output || !out_len || max_out_len < 32) {
        return AMNESIC_ERR_ARGS;
    }

    uint8_t full_digest[SHA512_DIGEST_SIZE];
    sha512_hash(input, input_len, full_digest);

    /* Split 64-byte SHA-512 into two 32-byte halves and XOR them together */
    uint8_t folded[32];
    for (size_t i = 0; i < 32; i++) {
        folded[i] = full_digest[i] ^ full_digest[i + 32];
    }

    memcpy(output, folded, 32);
    *out_len = 32;

    secure_wipe(full_digest, sizeof(full_digest));
    secure_wipe(folded, sizeof(folded));

    return AMNESIC_SUCCESS;
}
