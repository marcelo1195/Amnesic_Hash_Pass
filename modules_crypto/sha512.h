#ifndef AMNESIC_SHA512_H
#define AMNESIC_SHA512_H

#include <stddef.h>
#include <stdint.h>

#define SHA512_BLOCK_SIZE 128
#define SHA512_DIGEST_SIZE 64

typedef struct {
    uint64_t state[8];
    uint64_t count[2];
    uint8_t buffer[128];
} AmnesicSHA512Context;

void sha512_init(AmnesicSHA512Context *ctx);
void sha512_update(AmnesicSHA512Context *ctx, const uint8_t *data, size_t len);
void sha512_final(AmnesicSHA512Context *ctx, uint8_t digest[64]);
void sha512_hash(const uint8_t *data, size_t len, uint8_t digest[64]);

#endif
