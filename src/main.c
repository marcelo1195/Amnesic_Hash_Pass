#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "amnesic.h"
#include "pipeline.h"
#include "module_registry.h"

static void print_usage(const char *prog_name) {
    printf("Amnesic Hasher - Air-Gapped High-Entropy Passkey Generator\n\n");
    printf("Usage: %s [MODE] [OPTIONS]\n\n", prog_name);
    printf("Modes (Select One):\n");
    printf("  --simple              Direct 1:1 hash mapping\n");
    printf("  --process <N>         Execute drop-in transformation variant <N>\n");
    printf("  --explode             Iterative key-stretching / XOF entropy expansion\n\n");
    printf("Options:\n");
    printf("  --algo <engine>       Hash engine: sha256 (default), sha512, blake3\n");
    printf("  --encode <format>     Output format: hex (default), base64, base85\n");
    printf("  --length <bytes>      Requested output length in bytes (BLAKE3 / Explode)\n");
    printf("  --file <path>         Read input vector from raw file\n");
    printf("  --list-variants       List all registered drop-in processing variants\n");
    printf("  --help                Display this help message\n\n");
}

int main(int argc, char *argv[]) {
    PipelineConfig config = {
        .mode = MODE_SIMPLE,
        .algo = ALGO_SHA256,
        .encoding = ENCODE_HEX,
        .variant_id = 0,
        .length = 0,
        .file_path = NULL
    };

    bool mode_selected = false;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return AMNESIC_SUCCESS;
        }

        if (strcmp(argv[i], "--list-variants") == 0) {
            list_registered_variants();
            return AMNESIC_SUCCESS;
        }

        if (strcmp(argv[i], "--simple") == 0) {
            config.mode = MODE_SIMPLE;
            mode_selected = true;
        } else if (strcmp(argv[i], "--process") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: --process requires variant ID argument.\n");
                return AMNESIC_ERR_ARGS;
            }
            config.mode = MODE_PROCESS;
            config.variant_id = atoi(argv[++i]);
            mode_selected = true;
        } else if (strcmp(argv[i], "--explode") == 0) {
            config.mode = MODE_EXPLODE;
            mode_selected = true;
        } else if (strcmp(argv[i], "--algo") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: --algo requires algorithm name.\n");
                return AMNESIC_ERR_ARGS;
            }
            i++;
            if (strcmp(argv[i], "sha256") == 0) {
                config.algo = ALGO_SHA256;
            } else if (strcmp(argv[i], "sha512") == 0) {
                config.algo = ALGO_SHA512;
            } else if (strcmp(argv[i], "blake3") == 0) {
                config.algo = ALGO_BLAKE3;
            } else {
                fprintf(stderr, "Error: Unknown algorithm '%s'.\n", argv[i]);
                return AMNESIC_ERR_ALGO;
            }
        } else if (strcmp(argv[i], "--encode") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: --encode requires encoding format.\n");
                return AMNESIC_ERR_ARGS;
            }
            i++;
            if (strcmp(argv[i], "hex") == 0) {
                config.encoding = ENCODE_HEX;
            } else if (strcmp(argv[i], "base64") == 0) {
                config.encoding = ENCODE_BASE64;
            } else if (strcmp(argv[i], "base85") == 0) {
                config.encoding = ENCODE_BASE85;
            } else {
                fprintf(stderr, "Error: Unknown encoder '%s'.\n", argv[i]);
                return AMNESIC_ERR_ENCODER;
            }
        } else if (strcmp(argv[i], "--length") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: --length requires byte count.\n");
                return AMNESIC_ERR_ARGS;
            }
            config.length = (size_t)atol(argv[++i]);
        } else if (strcmp(argv[i], "--file") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: --file requires file path.\n");
                return AMNESIC_ERR_ARGS;
            }
            config.file_path = argv[++i];
        } else {
            fprintf(stderr, "Error: Unknown argument '%s'. Use --help for usage.\n", argv[i]);
            return AMNESIC_ERR_ARGS;
        }
    }

    if (!mode_selected && argc > 1) {
        /* Default mode is MODE_SIMPLE if not specified */
        config.mode = MODE_SIMPLE;
    }

    return run_amnesic_pipeline(&config);
}
