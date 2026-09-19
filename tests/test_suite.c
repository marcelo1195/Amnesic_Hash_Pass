#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "amnesic.h"
#include "crypto_api.h"
#include "sha256.h"
#include "sha512.h"
#include "blake3.h"
#include "module_registry.h"
#include "encode_hex.h"
#include "encode_base64.h"
#include "encode_base85.h"
#include "memory.h"

static int g_tests_passed = 0;
static int g_tests_failed = 0;

#define TEST_ASSERT(cond, msg) \
    do { \
        if (cond) { \
            printf("[PASS] %s\n", msg); \
            g_tests_passed++; \
        } else { \
            printf("[FAIL] %s (line %d)\n", msg, __LINE__); \
            g_tests_failed++; \
        } \
    } while (0)

static void test_sha256_vector(void) {
    const char *input = "Amnesic";
    uint8_t digest[32];
    sha256_hash((const uint8_t*)input, strlen(input), digest);

    char hex[65];
    size_t hex_len = 0;
    encode_hex(digest, 32, hex, sizeof(hex), &hex_len);

    const char *expected = "14a383d9bfda75749c94daccd6b3f8d435b6124d6c37dc24f5eb558509200dd7";
    TEST_ASSERT(strcmp(hex, expected) == 0, "SHA-256 digest vector match for 'Amnesic'");
}

static void test_sha512_vector(void) {
    const char *input = "Amnesic";
    uint8_t digest[64];
    sha512_hash((const uint8_t*)input, strlen(input), digest);

    char hex[129];
    size_t hex_len = 0;
    encode_hex(digest, 64, hex, sizeof(hex), &hex_len);

    const char *expected = "fdd302ae85f0382d3b3836eada0afd689ecc2b7094ee67cd3bbc1a3aa73f9eeab7697e16a0b41c361b5a9e10db0ab49bc1ffd7ce103216e6fd9197bc2870371e";
    TEST_ASSERT(strcmp(hex, expected) == 0, "SHA-512 digest vector match for 'Amnesic'");
}

static void test_blake3_vector(void) {
    const char *input = "Amnesic";
    uint8_t digest[32];
    blake3_hash((const uint8_t*)input, strlen(input), digest, 32);

    char hex[65];
    size_t hex_len = 0;
    encode_hex(digest, 32, hex, sizeof(hex), &hex_len);

    TEST_ASSERT(strlen(hex) == 64, "BLAKE3 32-byte digest hex length is 64 characters");
}

static void test_base64_encoder(void) {
    const char *input = "Amnesic Hasher Test";
    char b64[128];
    size_t out_len = 0;
    encode_base64((const unsigned char*)input, strlen(input), b64, sizeof(b64), &out_len);

    /* RFC 4648 Base64 for "Amnesic Hasher Test" */
    const char *expected = "QW1uZXNpYyBIYXNoZXIgVGVzdA==";
    TEST_ASSERT(strcmp(b64, expected) == 0, "Base64 encoder RFC 4648 output match");
}

static void test_base85_encoder(void) {
    const unsigned char input[] = { 0x01, 0x02, 0x03, 0x04 };
    char b85[32];
    size_t out_len = 0;
    encode_base85(input, 4, b85, sizeof(b85), &out_len);

    TEST_ASSERT(out_len == 5, "Base85 4-byte chunk encodes to exactly 5 characters");
}

static void test_memory_zeroization(void) {
    char secret[64];
    memset(secret, 'A', sizeof(secret));
    secure_wipe(secret, sizeof(secret));

    int non_zero_count = 0;
    for (size_t i = 0; i < sizeof(secret); i++) {
        if (secret[i] != 0) non_zero_count++;
    }

    TEST_ASSERT(non_zero_count == 0, "secure_wipe zeroizes memory completely");
}

static void test_variant_1_cascade(void) {
    const ProcessVariant *v1 = get_variant_by_id(1);
    TEST_ASSERT(v1 != NULL && v1->process_func != NULL, "Variant 1 function pointer valid");

    if (v1 && v1->process_func) {
        const char *input = "test";
        unsigned char raw_out[64];
        size_t out_len = 0;

        int ret = v1->process_func(ALGO_SHA512, (const unsigned char*)input, strlen(input), raw_out, sizeof(raw_out), &out_len);
        TEST_ASSERT(ret == AMNESIC_SUCCESS, "Variant 1 execution success");

        char hex[129];
        size_t hex_len = 0;
        encode_hex(raw_out, out_len, hex, sizeof(hex), &hex_len);

        const char *expected = "b168776df40a395b01945999770ff8f6e7ba5b721d09d967ab9a08feeec68906fb727db283079242fc5a811c42ef82afa8c1ebfb1742fd7cf0eefd733cb1fff0";
        TEST_ASSERT(strcmp(hex, expected) == 0, "Variant 1 SHA-512 cascade vector matches manual bash calculation");
    }
}

static void test_variant_registry(void) {
    size_t count = get_registered_variants_count();
    TEST_ASSERT(count >= 2, "Module registry contains at least 2 variants");

    const ProcessVariant *v1 = get_variant_by_id(1);
    TEST_ASSERT(v1 != NULL && v1->id == 1, "Variant 1 registry retrieval");

    const ProcessVariant *v2 = get_variant_by_id(2);
    TEST_ASSERT(v2 != NULL && v2->id == 2, "Variant 2 registry retrieval");
}

int main(void) {
    printf("=== AMNESIC HASHER AUTOMATED TEST SUITE ===\n\n");

    test_sha256_vector();
    test_sha512_vector();
    test_blake3_vector();
    test_base64_encoder();
    test_base85_encoder();
    test_memory_zeroization();
    test_variant_registry();
    test_variant_1_cascade();

    printf("\nTest Summary: %d Passed, %d Failed\n", g_tests_passed, g_tests_failed);
    return (g_tests_failed == 0) ? 0 : 1;
}
