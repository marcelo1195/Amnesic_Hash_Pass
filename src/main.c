#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "amnesic.h"
#include "pipeline.h"
#include "terminal.h"
#include "module_registry.h"

static void print_usage(const char *prog_name) {
    printf("================================================================================\n");
    printf("  AMNESIC HASHER - Air-Gapped High-Entropy Passkey Generator\n");
    printf("================================================================================\n\n");
    printf("USAGE:\n");
    printf("  %s [MODE] [OPTIONS]\n\n", prog_name);
    printf("MODES (Select One):\n");
    printf("  --simple              Direct 1:1 hashing (Default mode)\n");
    printf("  --process <N>         Execute drop-in transformation variant <N>\n");
    printf("  --explode             Iterative key-stretching / XOF entropy expansion\n\n");
    printf("TERMINAL & ANTI-FORENSICS:\n");
    printf("  -t, --terminal        Launch interactive popup window; leaves zero trace in\n");
    printf("                        your main terminal history. Self-destructs on completion.\n\n");
    printf("CRYPTOGRAPHIC ENGINES:\n");
    printf("  --algo <engine>       Hash engine: sha512 (default), sha256, blake3\n\n");
    printf("OUTPUT ENCODERS & FORMATTING:\n");
    printf("  --encode <format>     Output encoder: hex (default), base64, base85\n");
    printf("  --length <bytes>      Requested output length in bytes (BLAKE3 / Explode)\n\n");
    printf("INPUT SOURCES & UTILITIES:\n");
    printf("  --file <path>         Read raw input vector from file\n");
    printf("  --list-variants       List all registered drop-in processing variants\n");
    printf("  -h, --help            Display this comprehensive help menu\n\n");
    printf("EXAMPLES:\n");
    printf("  # Interactive popup terminal mode (Leaves 0 trace in shell history):\n");
    printf("  %s -t --algo sha512 --encode base64\n\n", prog_name);
    printf("  # Fast simple mode piped input (Default: SHA-512 Hex):\n");
    printf("  echo -n \"secret\" | %s --simple\n\n", prog_name);
    printf("  # Execute Variant 1 (Sequential Cascade Hashing):\n");
    printf("  echo -n \"secret\" | %s --process 1 --algo sha512 --encode hex\n\n", prog_name);
    printf("================================================================================\n");
}

int main(int argc, char *argv[]) {
    PipelineConfig config = {
        .mode = MODE_SIMPLE,
        .algo = ALGO_SHA512,  /* Default algorithm: SHA-512 */
        .encoding = ENCODE_HEX,
        .variant_id = 0,
        .length = 0,
        .file_path = NULL,
        .use_terminal = false,
        .is_child_terminal = false
    };

    bool mode_selected = false;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            print_usage(argv[0]);
            return AMNESIC_SUCCESS;
        }

        if (strcmp(argv[i], "--list-variants") == 0) {
            list_registered_variants();
            return AMNESIC_SUCCESS;
        }

        if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--terminal") == 0) {
            config.use_terminal = true;
        } else if (strcmp(argv[i], "--child-terminal") == 0) {
            config.is_child_terminal = true;
        } else if (strcmp(argv[i], "--simple") == 0) {
            config.mode = MODE_SIMPLE;
            config.mode_explicit = true;
            mode_selected = true;
        } else if (strcmp(argv[i], "--process") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: --process requires variant ID argument.\n");
                return AMNESIC_ERR_ARGS;
            }
            config.mode = MODE_PROCESS;
            config.variant_id = atoi(argv[++i]);
            config.mode_explicit = true;
            config.variant_explicit = true;
            mode_selected = true;
        } else if (strcmp(argv[i], "--explode") == 0) {
            config.mode = MODE_EXPLODE;
            config.mode_explicit = true;
            mode_selected = true;
        } else if (strcmp(argv[i], "--algo") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: --algo requires algorithm name.\n");
                return AMNESIC_ERR_ARGS;
            }
            i++;
            config.algo_explicit = true;
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
            config.encode_explicit = true;
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

    if (!mode_selected) {
        config.mode = MODE_SIMPLE;
    }

    if (config.use_terminal && !config.is_child_terminal) {
        return launch_in_standalone_terminal(argc, argv);
    }

    return run_amnesic_pipeline(&config);
}
