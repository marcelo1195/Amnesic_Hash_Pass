#include <stdio.h>
#include <string.h>
#include "amnesic.h"
#include "module_registry.h"

/* Forward declarations of drop-in variant functions */
int variant_1_sequential(CryptoAlgo algo, const unsigned char *input, size_t input_len,
                         unsigned char *output, size_t max_out_len, size_t *out_len);

int variant_2_xor(CryptoAlgo algo, const unsigned char *input, size_t input_len,
                  unsigned char *output, size_t max_out_len, size_t *out_len);

static const ProcessVariant g_variants[] = {
    {
        .id = 1,
        .name = "Sequential Cascade Hashing",
        .description = "Human-reproducible character-by-character hash cascade",
        .process_func = variant_1_sequential
    },
    {
        .id = 2,
        .name = "Bifurcated XOR Folding",
        .description = "Computes algorithm digest and folds halves using bitwise XOR",
        .process_func = variant_2_xor
    }
};

static const size_t g_variants_count = sizeof(g_variants) / sizeof(g_variants[0]);

const ProcessVariant *get_variant_by_id(int id) {
    for (size_t i = 0; i < g_variants_count; i++) {
        if (g_variants[i].id == id) {
            return &g_variants[i];
        }
    }
    return NULL;
}

size_t get_registered_variants_count(void) {
    return g_variants_count;
}

void list_registered_variants(void) {
    printf("Registered Transformation Variants (%lu total):\n", (unsigned long)g_variants_count);
    for (size_t i = 0; i < g_variants_count; i++) {
        printf("  [%d] %s - %s\n",
               g_variants[i].id,
               g_variants[i].name,
               g_variants[i].description);
    }
}
