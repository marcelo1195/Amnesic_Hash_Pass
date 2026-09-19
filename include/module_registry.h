#ifndef AMNESIC_MODULE_REGISTRY_H
#define AMNESIC_MODULE_REGISTRY_H

#include "crypto_api.h"

typedef int (*ProcessVariantFunc)(CryptoAlgo algo,
                                  const unsigned char *input, size_t input_len,
                                  unsigned char *output, size_t max_out_len, size_t *out_len);

typedef struct {
    int id;
    const char *name;
    const char *description;
    ProcessVariantFunc process_func;
} ProcessVariant;

const ProcessVariant *get_variant_by_id(int id);
size_t get_registered_variants_count(void);
void list_registered_variants(void);

#endif
