#ifndef AMNESIC_BLAKE3_H
#define AMNESIC_BLAKE3_H

#include <stddef.h>
#include <stdint.h>

#define BLAKE3_KEY_LEN 32
#define BLAKE3_OUT_LEN 32
#define BLAKE3_BLOCK_LEN 64
#define BLAKE3_CHUNK_LEN 1024

/* Flag bits */
#define BLAKE3_CHUNK_START (1 << 0)
#define BLAKE3_CHUNK_END   (1 << 1)
#define BLAKE3_PARENT      (1 << 2)
#define BLAKE3_ROOT        (1 << 3)
#define BLAKE3_KEYED_HASH  (1 << 4)
#define BLAKE3_DERIVE_KEY_CONTEXT (1 << 5)
#define BLAKE3_DERIVE_KEY_MATERIAL (1 << 6)

typedef struct {
    uint32_t cv[8];
    uint64_t chunk_counter;
    uint8_t buf[BLAKE3_BLOCK_LEN];
    uint8_t buf_len;
    uint8_t blocks_compressed;
    uint8_t flags;
} AmnesicBLAKE3ChunkState;

typedef struct {
    uint32_t key[8];
    AmnesicBLAKE3ChunkState chunk;
    uint8_t cv_stack_len;
    /* Stack for tree parent nodes up to depth 54 */
    uint32_t cv_stack[54 * 8];
    uint8_t flags;
} AmnesicBLAKE3Hasher;

void blake3_hasher_init(AmnesicBLAKE3Hasher *self);
void blake3_hasher_update(AmnesicBLAKE3Hasher *self, const void *input, size_t input_len);
void blake3_hasher_finalize(const AmnesicBLAKE3Hasher *self, uint8_t *out, size_t out_len);
void blake3_hash(const uint8_t *input, size_t input_len, uint8_t *out, size_t out_len);

#endif
