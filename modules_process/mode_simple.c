#include "amnesic.h"
#include "crypto_api.h"

int execute_mode_simple(CryptoAlgo algo,
                        const unsigned char *input, size_t input_len,
                        unsigned char *output, size_t req_len, size_t *out_len) {
    return generate_hash(algo, input, input_len, output, req_len, out_len);
}
