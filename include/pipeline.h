#ifndef AMNESIC_PIPELINE_H
#define AMNESIC_PIPELINE_H

#include <stddef.h>
#include <stdbool.h>
#include "amnesic.h"

typedef struct {
    AmnesicMode mode;
    CryptoAlgo algo;
    OutputEncoding encoding;
    int variant_id;
    size_t length;
    const char *file_path;
} PipelineConfig;

int run_amnesic_pipeline(const PipelineConfig *config);

#endif
