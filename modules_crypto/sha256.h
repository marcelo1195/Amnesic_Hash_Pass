#ifndef AMNESIC_SHA256_H
#define AMNESIC_SHA256_H

#include <stddef.h>
#include <stdint.h>

#define SHA256_BLOCK_SIZE 64
#define SHA256_DIGEST_SIZE 32

typedef struct {
    uint32_t state[8];
    uint64_t count;
    uint8_t buffer[64];
} AmnesicSHA256Context;

void sha256_init(AmnesicSHA256Context *ctx);
void sha256_update(AmnesicSHA256Context *ctx, const uint8_t *data, size_t len);
void sha256_final(AmnesicSHA256Context *ctx, uint8_t digest[32]);
void sha256_hash(const uint8_t *data, size_t len, uint8_t digest[32]);

#endif
