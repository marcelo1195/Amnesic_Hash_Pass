#include <string.h>
#include "sha512.h"
#include "memory.h"

#define SHA512_ROTR(x, n) (((x) >> (n)) | ((x) << (64 - (n))))
#define SHA512_Ch(x, y, z) (((x) & (y)) ^ ((~(x)) & (z)))
#define SHA512_Maj(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define SHA512_Sigma0(x) (SHA512_ROTR(x, 28) ^ SHA512_ROTR(x, 34) ^ SHA512_ROTR(x, 39))
#define SHA512_Sigma1(x) (SHA512_ROTR(x, 14) ^ SHA512_ROTR(x, 18) ^ SHA512_ROTR(x, 41))
#define SHA512_sigma0(x) (SHA512_ROTR(x, 1) ^ SHA512_ROTR(x, 8) ^ ((x) >> 7))
#define SHA512_sigma1(x) (SHA512_ROTR(x, 19) ^ SHA512_ROTR(x, 61) ^ ((x) >> 6))

static const uint64_t K512[80] = {
    0x428a2f98d728ae22ULL, 0x7137449123ef65cdULL, 0xb5c0fbcfec4d3b2fULL, 0xe9b5dba58189dbbcULL,
    0x3956c25bf348b538ULL, 0x59f111f1b605d019ULL, 0x923f82a4af194f9bULL, 0xab1c5ed5da6d8118ULL,
    0xd807aa98a3030242ULL, 0x12835b0145706fbeULL, 0x243185be4ee4b28cULL, 0x550c7dc3d5ffb4e2ULL,
    0x72be5d74f27b896fULL, 0x80deb1fe3b1696b1ULL, 0x9bdc06a725c71235ULL, 0xc19bf174cf692694ULL,
    0xe49b69c19ef14ad2ULL, 0xefbe4786384f25e3ULL, 0x0fc19dc68b8cd5b5ULL, 0x240ca1cc77ac9c65ULL,
    0x2de92c6f592b0275ULL, 0x4a7484aa6ea6e483ULL, 0x5cb0a9dcbd41fbd4ULL, 0x76f988da831153b5ULL,
    0x983e5152ee66dfabULL, 0xa831c66d2db43210ULL, 0xb00327c898fb213fULL, 0xbf597fc7beef0ee4ULL,
    0xc6e00bf33da88fc2ULL, 0xd5a79147930aa725ULL, 0x06ca6351e003826fULL, 0x142929670a0e6e70ULL,
    0x27b70a8546d22ffcULL, 0x2e1b21385c26c926ULL, 0x4d2c6dfc5ac42aedULL, 0x53380d139d95b3dfULL,
    0x650a73548baf63deULL, 0x766a0abb3c77b2a8ULL, 0x81c2c92e47edaee6ULL, 0x92722c851482353bULL,
    0xa2bfe8a14cf10364ULL, 0xa81a664bbc423001ULL, 0xc24b8b70d0f89791ULL, 0xc76c51a30654be30ULL,
    0xd192e819d6ef5218ULL, 0xd69906245565a910ULL, 0xf40e35855771202aULL, 0x106aa07032bbd1b8ULL,
    0x19a4c116b8d2d0c8ULL, 0x1e376c085141ab53ULL, 0x2748774cdf8eeb99ULL, 0x34b0bcb5e19b48a8ULL,
    0x391c0cb3c5c95a63ULL, 0x4ed8aa4ae3418acbULL, 0x5b9cca4f7763e373ULL, 0x682e6ff3d6b2b8a3ULL,
    0x748f82ee5defb2fcULL, 0x78a5636f43172f60ULL, 0x84c87814a1f0ab72ULL, 0x8cc702081a6439ecULL,
    0x90befffa23631e28ULL, 0xa4506cebde82bde9ULL, 0xbef9a3f7b2c67915ULL, 0xc67178f2e372532bULL,
    0xca273eceea26619cULL, 0xd186b8c721c0c207ULL, 0xeada7dd6cde0eb1eULL, 0xf57d4f7fee6ed178ULL,
    0x06f067aa72176fbaULL, 0x0a637dc5a2c898a6ULL, 0x113f9804bef90daeULL, 0x1b710b35131c471bULL,
    0x28db77f523047d84ULL, 0x32caab7b40c72493ULL, 0x3c9ebe0a15c9bebcULL, 0x431d67c49c100d4cULL,
    0x4cc5d4becb3e42b6ULL, 0x597f299cfc657e2aULL, 0x5fcb6fab3ad6faecULL, 0x6c44198c4a475817ULL
};

static void sha512_transform(AmnesicSHA512Context *ctx, const uint8_t data[128]) {
    uint64_t a, b, c, d, e, f, g, h, t1, t2, W[80];

    for (int t = 0; t < 16; t++) {
        W[t] = ((uint64_t)data[t * 8] << 56) |
               ((uint64_t)data[t * 8 + 1] << 48) |
               ((uint64_t)data[t * 8 + 2] << 40) |
               ((uint64_t)data[t * 8 + 3] << 32) |
               ((uint64_t)data[t * 8 + 4] << 24) |
               ((uint64_t)data[t * 8 + 5] << 16) |
               ((uint64_t)data[t * 8 + 6] << 8)  |
               ((uint64_t)data[t * 8 + 7]);
    }
    for (int t = 16; t < 80; t++) {
        W[t] = SHA512_sigma1(W[t - 2]) + W[t - 7] + SHA512_sigma0(W[t - 15]) + W[t - 16];
    }

    a = ctx->state[0];
    b = ctx->state[1];
    c = ctx->state[2];
    d = ctx->state[3];
    e = ctx->state[4];
    f = ctx->state[5];
    g = ctx->state[6];
    h = ctx->state[7];

    for (int t = 0; t < 80; t++) {
        t1 = h + SHA512_Sigma1(e) + SHA512_Ch(e, f, g) + K512[t] + W[t];
        t2 = SHA512_Sigma0(a) + SHA512_Maj(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }

    ctx->state[0] += a;
    ctx->state[1] += b;
    ctx->state[2] += c;
    ctx->state[3] += d;
    ctx->state[4] += e;
    ctx->state[5] += f;
    ctx->state[6] += g;
    ctx->state[7] += h;

    secure_wipe(W, sizeof(W));
}

void sha512_init(AmnesicSHA512Context *ctx) {
    if (!ctx) return;
    ctx->count[0] = 0;
    ctx->count[1] = 0;
    ctx->state[0] = 0x6a09e667f3bcc908ULL;
    ctx->state[1] = 0xbb67ae8584caa73bULL;
    ctx->state[2] = 0x3c6ef372fe94f82bULL;
    ctx->state[3] = 0xa54ff53a5f1d36f1ULL;
    ctx->state[4] = 0x510e527fade682d1ULL;
    ctx->state[5] = 0x9b05688c2b3e6c1fULL;
    ctx->state[6] = 0x1f83d9abfb41bd6bULL;
    ctx->state[7] = 0x5be0cd19137e2179ULL;
    secure_wipe(ctx->buffer, sizeof(ctx->buffer));
}

void sha512_update(AmnesicSHA512Context *ctx, const uint8_t *data, size_t len) {
    if (!ctx || !data || len == 0) return;

    size_t index = (size_t)(ctx->count[0] & 0x7F);

    ctx->count[0] += len;
    if (ctx->count[0] < len) {
        ctx->count[1]++;
    }

    size_t part_len = 128 - index;
    size_t i = 0;

    if (len >= part_len) {
        memcpy(&ctx->buffer[index], data, part_len);
        sha512_transform(ctx, ctx->buffer);
        for (i = part_len; i + 127 < len; i += 128) {
            sha512_transform(ctx, &data[i]);
        }
        index = 0;
    }

    if (i < len) {
        memcpy(&ctx->buffer[index], &data[i], len - i);
    }
}

void sha512_final(AmnesicSHA512Context *ctx, uint8_t digest[64]) {
    if (!ctx || !digest) return;

    uint8_t final_block[256];
    memset(final_block, 0, sizeof(final_block));

    size_t current_len = (size_t)(ctx->count[0] & 0x7F);
    memcpy(final_block, ctx->buffer, current_len);
    final_block[current_len] = 0x80;

    size_t total_padded = current_len + 1;
    size_t target_len = (total_padded <= 112) ? 128 : 256;

    uint64_t bits_low = ctx->count[0] * 8;
    uint64_t bits_high = (ctx->count[1] * 8) | (ctx->count[0] >> 61);

    for (int i = 0; i < 8; i++) {
        final_block[target_len - 16 + i] = (uint8_t)(bits_high >> (56 - i * 8));
        final_block[target_len - 8 + i]  = (uint8_t)(bits_low >> (56 - i * 8));
    }

    for (size_t b = 0; b < target_len; b += 128) {
        sha512_transform(ctx, &final_block[b]);
    }

    for (int i = 0; i < 8; i++) {
        digest[i * 8]     = (uint8_t)(ctx->state[i] >> 56);
        digest[i * 8 + 1] = (uint8_t)(ctx->state[i] >> 48);
        digest[i * 8 + 2] = (uint8_t)(ctx->state[i] >> 40);
        digest[i * 8 + 3] = (uint8_t)(ctx->state[i] >> 32);
        digest[i * 8 + 4] = (uint8_t)(ctx->state[i] >> 24);
        digest[i * 8 + 5] = (uint8_t)(ctx->state[i] >> 16);
        digest[i * 8 + 6] = (uint8_t)(ctx->state[i] >> 8);
        digest[i * 8 + 7] = (uint8_t)(ctx->state[i]);
    }

    secure_wipe(final_block, sizeof(final_block));
    secure_wipe(ctx, sizeof(AmnesicSHA512Context));
}

void sha512_hash(const uint8_t *data, size_t len, uint8_t digest[64]) {
    AmnesicSHA512Context ctx;
    sha512_init(&ctx);
    sha512_update(&ctx, data, len);
    sha512_final(&ctx, digest);
}
