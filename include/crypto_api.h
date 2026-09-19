#ifndef AMNESIC_CRYPTO_API_H
#define AMNESIC_CRYPTO_API_H

#include <stddef.h>
#include <stdint.h>
#include "amnesic.h"

int generate_hash(CryptoAlgo algo,
                  const unsigned char *input, size_t input_len,
                  unsigned char *output, size_t req_output_len, size_t *out_len);

size_t get_crypto_digest_size(CryptoAlgo algo);

#endif
