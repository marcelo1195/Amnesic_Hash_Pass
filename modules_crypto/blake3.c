#include <string.h>
#include "blake3.h"
#include "memory.h"

static const uint32_t IV[8] = {
    0x6A09E667, 0xBB67AE85, 0x3C6EF372, 0xA54FF53A,
    0x510E527F, 0x9B05688C, 0x1F83D9AB, 0x5BE0CD19
};

static const uint8_t MSG_PERMUTATION[16] = {
    2, 6, 3, 10, 7, 0, 4, 13, 1, 11, 12, 5, 9, 14, 15, 8
};

static inline uint32_t rotr32(uint32_t w, unsigned int c) {
    return (w >> c) | (w << (32 - c));
}

static void g(uint32_t *state, size_t a, size_t b, size_t c, size_t d, uint32_t mx, uint32_t my) {
    state[a] = state[a] + state[b] + mx;
    state[d] = rotr32(state[d] ^ state[a], 16);
    state[c] = state[c] + state[d];
    state[b] = rotr32(state[b] ^ state[c], 12);
    state[a] = state[a] + state[b] + my;
    state[d] = rotr32(state[d] ^ state[a], 8);
    state[c] = state[c] + state[d];
    state[b] = rotr32(state[b] ^ state[c], 7);
}

static void round_fn(uint32_t *state, const uint32_t *msg) {
    g(state, 0, 4, 8, 12, msg[0], msg[1]);
    g(state, 1, 5, 9, 13, msg[2], msg[3]);
    g(state, 2, 6, 10, 14, msg[4], msg[5]);
    g(state, 3, 7, 11, 15, msg[6], msg[7]);
    g(state, 0, 5, 10, 15, msg[8], msg[9]);
    g(state, 1, 6, 11, 12, msg[10], msg[11]);
    g(state, 2, 7, 8, 13, msg[12], msg[13]);
    g(state, 3, 4, 9, 14, msg[14], msg[15]);
}

static void compress(const uint32_t cv[8], const uint8_t block[64], uint8_t block_len,
                     uint64_t counter, uint8_t flags, uint32_t out[16]) {
    uint32_t block_words[16];
    for (size_t i = 0; i < 16; i++) {
        block_words[i] = ((uint32_t)block[i * 4]) |
                         ((uint32_t)block[i * 4 + 1] << 8) |
                         ((uint32_t)block[i * 4 + 2] << 16) |
                         ((uint32_t)block[i * 4 + 3] << 24);
    }

    uint32_t state[16];
    memcpy(state, cv, 8 * sizeof(uint32_t));
    memcpy(state + 8, IV, 4 * sizeof(uint32_t));
    state[12] = (uint32_t)counter;
    state[13] = (uint32_t)(counter >> 32);
    state[14] = (uint32_t)block_len;
    state[15] = (uint32_t)flags;

    uint32_t msg[16];
    memcpy(msg, block_words, 16 * sizeof(uint32_t));

    for (size_t r = 0; r < 7; r++) {
        round_fn(state, msg);
        uint32_t next_msg[16];
        for (size_t i = 0; i < 16; i++) {
            next_msg[i] = msg[MSG_PERMUTATION[i]];
        }
        memcpy(msg, next_msg, 16 * sizeof(uint32_t));
    }

    for (size_t i = 0; i < 8; i++) {
        out[i] = state[i] ^ state[i + 8];
        out[i + 8] = state[i + 8] ^ cv[i];
    }

    secure_wipe(block_words, sizeof(block_words));
    secure_wipe(state, sizeof(state));
    secure_wipe(msg, sizeof(msg));
}

static void chunk_state_init(AmnesicBLAKE3ChunkState *self, const uint32_t key[8],
                             uint64_t chunk_counter, uint8_t flags) {
    memcpy(self->cv, key, 8 * sizeof(uint32_t));
    self->chunk_counter = chunk_counter;
    memset(self->buf, 0, BLAKE3_BLOCK_LEN);
    self->buf_len = 0;
    self->blocks_compressed = 0;
    self->flags = flags;
}

static uint8_t chunk_state_flags(const AmnesicBLAKE3ChunkState *self) {
    uint8_t flags = self->flags;
    if (self->blocks_compressed == 0) {
        flags |= BLAKE3_CHUNK_START;
    }
    return flags;
}

static void chunk_state_update(AmnesicBLAKE3ChunkState *self, const uint8_t *input, size_t input_len) {
    while (input_len > 0) {
        if (self->buf_len == BLAKE3_BLOCK_LEN) {
            uint32_t out16[16];
            compress(self->cv, self->buf, BLAKE3_BLOCK_LEN, self->chunk_counter,
                     chunk_state_flags(self), out16);
            memcpy(self->cv, out16, 8 * sizeof(uint32_t));
            self->blocks_compressed++;
            memset(self->buf, 0, BLAKE3_BLOCK_LEN);
            self->buf_len = 0;
            secure_wipe(out16, sizeof(out16));
        }

        size_t take = BLAKE3_BLOCK_LEN - self->buf_len;
        if (take > input_len) take = input_len;
        memcpy(self->buf + self->buf_len, input, take);
        self->buf_len += (uint8_t)take;
        input += take;
        input_len -= take;
    }
}

static void parent_cv(const uint32_t left_child_cv[8], const uint32_t right_child_cv[8],
                      const uint32_t key[8], uint8_t flags, uint32_t out_cv[8]) {
    uint8_t block[64];
    for (size_t i = 0; i < 8; i++) {
        block[i * 4]     = (uint8_t)(left_child_cv[i]);
        block[i * 4 + 1] = (uint8_t)(left_child_cv[i] >> 8);
        block[i * 4 + 2] = (uint8_t)(left_child_cv[i] >> 16);
        block[i * 4 + 3] = (uint8_t)(left_child_cv[i] >> 24);

        block[32 + i * 4]     = (uint8_t)(right_child_cv[i]);
        block[32 + i * 4 + 1] = (uint8_t)(right_child_cv[i] >> 8);
        block[32 + i * 4 + 2] = (uint8_t)(right_child_cv[i] >> 16);
        block[32 + i * 4 + 3] = (uint8_t)(right_child_cv[i] >> 24);
    }
    uint32_t out16[16];
    compress(key, block, BLAKE3_BLOCK_LEN, 0, flags | BLAKE3_PARENT, out16);
    memcpy(out_cv, out16, 8 * sizeof(uint32_t));
    secure_wipe(block, sizeof(block));
    secure_wipe(out16, sizeof(out16));
}

static void hasher_push_cv(AmnesicBLAKE3Hasher *self, uint32_t new_cv[8]) {
    memcpy(&self->cv_stack[self->cv_stack_len * 8], new_cv, 8 * sizeof(uint32_t));
    self->cv_stack_len++;
}

static void hasher_pop_cv(AmnesicBLAKE3Hasher *self, uint32_t out_cv[8]) {
    self->cv_stack_len--;
    memcpy(out_cv, &self->cv_stack[self->cv_stack_len * 8], 8 * sizeof(uint32_t));
}

static void hasher_add_chunk_cv(AmnesicBLAKE3Hasher *self, uint32_t new_cv[8], uint64_t total_chunks) {
    while ((total_chunks & 1) == 0) {
        uint32_t left_child[8];
        hasher_pop_cv(self, left_child);
        parent_cv(left_child, new_cv, self->key, self->flags, new_cv);
        total_chunks >>= 1;
        secure_wipe(left_child, sizeof(left_child));
    }
    hasher_push_cv(self, new_cv);
}

void blake3_hasher_init(AmnesicBLAKE3Hasher *self) {
    if (!self) return;
    memcpy(self->key, IV, 8 * sizeof(uint32_t));
    chunk_state_init(&self->chunk, self->key, 0, 0);
    self->cv_stack_len = 0;
    self->flags = 0;
}

void blake3_hasher_update(AmnesicBLAKE3Hasher *self, const void *input, size_t input_len) {
    if (!self || !input || input_len == 0) return;
    const uint8_t *in = (const uint8_t *)input;

    while (input_len > 0) {
        if (self->chunk.buf_len == BLAKE3_BLOCK_LEN && self->chunk.blocks_compressed == (BLAKE3_CHUNK_LEN / BLAKE3_BLOCK_LEN - 1)) {
            uint32_t out16[16];
            compress(self->chunk.cv, self->chunk.buf, BLAKE3_BLOCK_LEN, self->chunk.chunk_counter,
                     chunk_state_flags(&self->chunk) | BLAKE3_CHUNK_END, out16);
            uint32_t chunk_cv[8];
            memcpy(chunk_cv, out16, 8 * sizeof(uint32_t));

            uint64_t total_chunks = self->chunk.chunk_counter + 1;
            hasher_add_chunk_cv(self, chunk_cv, total_chunks);

            chunk_state_init(&self->chunk, self->key, total_chunks, self->flags);
            secure_wipe(out16, sizeof(out16));
            secure_wipe(chunk_cv, sizeof(chunk_cv));
        }

        size_t take = BLAKE3_CHUNK_LEN - (self->chunk.blocks_compressed * BLAKE3_BLOCK_LEN + self->chunk.buf_len);
        if (take > input_len) take = input_len;

        chunk_state_update(&self->chunk, in, take);
        in += take;
        input_len -= take;
    }
}

void blake3_hasher_finalize(const AmnesicBLAKE3Hasher *self, uint8_t *out, size_t out_len) {
    if (!self || !out || out_len == 0) return;

    AmnesicBLAKE3Hasher copy = *self;

    uint32_t current_cv[8];
    uint32_t out16[16];
    compress(copy.chunk.cv, copy.chunk.buf, copy.chunk.buf_len, copy.chunk.chunk_counter,
             chunk_state_flags(&copy.chunk) | BLAKE3_CHUNK_END, out16);
    memcpy(current_cv, out16, 8 * sizeof(uint32_t));

    uint8_t stack_len = copy.cv_stack_len;
    while (stack_len > 0) {
        stack_len--;
        uint32_t left_child[8];
        memcpy(left_child, &copy.cv_stack[stack_len * 8], 8 * sizeof(uint32_t));
        uint8_t flags = copy.flags;
        if (stack_len == 0) {
            flags |= BLAKE3_ROOT;
        }
        parent_cv(left_child, current_cv, copy.key, flags, current_cv);
        secure_wipe(left_child, sizeof(left_child));
    }

    if (copy.cv_stack_len == 0) {
        /* Single chunk root node */
        uint8_t root_flags = chunk_state_flags(&copy.chunk) | BLAKE3_CHUNK_END | BLAKE3_ROOT;
        size_t out_offset = 0;
        uint64_t output_block_counter = 0;
        while (out_offset < out_len) {
            compress(copy.chunk.cv, copy.chunk.buf, copy.chunk.buf_len, output_block_counter,
                     root_flags, out16);
            for (size_t i = 0; i < 64 && out_offset < out_len; i++) {
                out[out_offset++] = ((uint8_t *)out16)[i];
            }
            output_block_counter++;
        }
    } else {
        /* Tree root node XOF squeeze */
        size_t out_offset = 0;
        uint64_t output_block_counter = 0;
        uint32_t last_left_child[8];
        memcpy(last_left_child, &copy.cv_stack[0], 8 * sizeof(uint32_t));
        uint8_t block[64];
        for (size_t i = 0; i < 8; i++) {
            block[i * 4]     = (uint8_t)(last_left_child[i]);
            block[i * 4 + 1] = (uint8_t)(last_left_child[i] >> 8);
            block[i * 4 + 2] = (uint8_t)(last_left_child[i] >> 16);
            block[i * 4 + 3] = (uint8_t)(last_left_child[i] >> 24);

            block[32 + i * 4]     = (uint8_t)(current_cv[i]);
            block[32 + i * 4 + 1] = (uint8_t)(current_cv[i] >> 8);
            block[32 + i * 4 + 2] = (uint8_t)(current_cv[i] >> 16);
            block[32 + i * 4 + 3] = (uint8_t)(current_cv[i] >> 24);
        }

        while (out_offset < out_len) {
            compress(copy.key, block, BLAKE3_BLOCK_LEN, output_block_counter,
                     copy.flags | BLAKE3_PARENT | BLAKE3_ROOT, out16);
            for (size_t i = 0; i < 64 && out_offset < out_len; i++) {
                out[out_offset++] = ((uint8_t *)out16)[i];
            }
            output_block_counter++;
        }
        secure_wipe(block, sizeof(block));
        secure_wipe(last_left_child, sizeof(last_left_child));
    }

    secure_wipe(&copy, sizeof(copy));
    secure_wipe(current_cv, sizeof(current_cv));
    secure_wipe(out16, sizeof(out16));
}

void blake3_hash(const uint8_t *input, size_t input_len, uint8_t *out, size_t out_len) {
    AmnesicBLAKE3Hasher hasher;
    blake3_hasher_init(&hasher);
    blake3_hasher_update(&hasher, input, input_len);
    blake3_hasher_finalize(&hasher, out, out_len);
}
