#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "amnesic.h"
#include "pipeline.h"
#include "memory.h"
#include "terminal.h"
#include "io_handler.h"
#include "crypto_api.h"
#include "module_registry.h"
#include "encode_hex.h"
#include "encode_base64.h"
#include "encode_base85.h"

/* Mode function forward declarations */
int execute_mode_simple(CryptoAlgo algo,
                        const unsigned char *input, size_t input_len,
                        unsigned char *output, size_t req_len, size_t *out_len);

int execute_mode_explode(CryptoAlgo algo,
                         const unsigned char *input, size_t input_len,
                         size_t req_length,
                         unsigned char *output, size_t max_out_len, size_t *out_len);

static void prompt_interactive_config(PipelineConfig *active_cfg) {
    if (!isatty(STDIN_FILENO)) {
        return;
    }
    char line[64];

    clear_terminal_screen();
    printf("============================================================\n");
    printf("     AMNESIC HASHER - ZERO-TRACE INTERACTIVE PANEL\n");
    printf("============================================================\n\n");

    if (!active_cfg->mode_explicit) {
        printf("1. Select Operation Mode:\n");
        printf("   [1] Simple (Direct Hash Mapping - Default)\n");
        printf("   [2] Process (Drop-in Transformation Variant)\n");
        printf("   [3] Explode (Key-Stretching / XOF Expansion)\n");
        printf("Select Mode [1-3] (Default: 1): ");
        fflush(stdout);
        if (fgets(line, sizeof(line), stdin)) {
            int val = atoi(line);
            if (val == 2) active_cfg->mode = MODE_PROCESS;
            else if (val == 3) active_cfg->mode = MODE_EXPLODE;
            else active_cfg->mode = MODE_SIMPLE;
        }
        printf("\n");
    }

    if (active_cfg->mode == MODE_PROCESS && !active_cfg->variant_explicit) {
        printf("   Registered Variants:\n");
        list_registered_variants();
        printf("Select Variant ID (Default: 1): ");
        fflush(stdout);
        if (fgets(line, sizeof(line), stdin)) {
            int val = atoi(line);
            active_cfg->variant_id = (val > 0) ? val : 1;
        }
        printf("\n");
    }

    if (!active_cfg->algo_explicit) {
        printf("2. Select Cryptographic Engine:\n");
        printf("   [1] SHA-512 (Default)\n");
        printf("   [2] SHA-256\n");
        printf("   [3] BLAKE3\n");
        printf("Select Engine [1-3] (Default: 1): ");
        fflush(stdout);
        if (fgets(line, sizeof(line), stdin)) {
            int val = atoi(line);
            if (val == 2) active_cfg->algo = ALGO_SHA256;
            else if (val == 3) active_cfg->algo = ALGO_BLAKE3;
            else active_cfg->algo = ALGO_SHA512;
        }
        printf("\n");
    }

    if (!active_cfg->encode_explicit) {
        printf("3. Select Output Encoding:\n");
        printf("   [1] Hexadecimal (Default)\n");
        printf("   [2] Base64\n");
        printf("   [3] Base85\n");
        printf("Select Encoder [1-3] (Default: 1): ");
        fflush(stdout);
        if (fgets(line, sizeof(line), stdin)) {
            int val = atoi(line);
            if (val == 2) active_cfg->encoding = ENCODE_BASE64;
            else if (val == 3) active_cfg->encoding = ENCODE_BASE85;
            else active_cfg->encoding = ENCODE_HEX;
        }
        printf("\n");
    }
}

int run_amnesic_pipeline(const PipelineConfig *config) {
    if (!config) {
        return AMNESIC_ERR_ARGS;
    }

    lock_process_memory();

    PipelineConfig active_config = *config;
    if (active_config.is_child_terminal && isatty(STDIN_FILENO) && (!active_config.mode_explicit || !active_config.algo_explicit || !active_config.encode_explicit)) {
        prompt_interactive_config(&active_config);
    }

    unsigned char input_buffer[MAX_INPUT_SIZE];
    size_t input_len = 0;
    secure_wipe(input_buffer, sizeof(input_buffer));

    int ret = read_input_vector(active_config.file_path, input_buffer, MAX_INPUT_SIZE, &input_len);
    if (ret != AMNESIC_SUCCESS) {
        secure_wipe(input_buffer, sizeof(input_buffer));
        fprintf(stderr, "Error: Failed to read input vector or exceeded maximum ceiling (%d bytes).\n", MAX_INPUT_SIZE);
        return ret;
    }

    if (input_len == 0) {
        secure_wipe(input_buffer, sizeof(input_buffer));
        fprintf(stderr, "Error: Empty input vector provided.\n");
        return AMNESIC_ERR_IO;
    }

    unsigned char raw_output[MAX_OUTPUT_SIZE];
    size_t raw_output_len = 0;
    secure_wipe(raw_output, sizeof(raw_output));

    switch (active_config.mode) {
        case MODE_SIMPLE:
            ret = execute_mode_simple(active_config.algo, input_buffer, input_len, raw_output, active_config.length, &raw_output_len);
            break;

        case MODE_PROCESS: {
            const ProcessVariant *variant = get_variant_by_id(active_config.variant_id);
            if (!variant || !variant->process_func) {
                fprintf(stderr, "Error: Invalid or unregistered variant ID [%d].\n", active_config.variant_id);
                ret = AMNESIC_ERR_VARIANT;
                break;
            }
            ret = variant->process_func(active_config.algo, input_buffer, input_len, raw_output, sizeof(raw_output), &raw_output_len);
            break;
        }

        case MODE_EXPLODE:
            ret = execute_mode_explode(active_config.algo, input_buffer, input_len, active_config.length, raw_output, sizeof(raw_output), &raw_output_len);
            break;

        default:
            ret = AMNESIC_ERR_ARGS;
            break;
    }

    /* Wipe input buffer immediately after processing */
    secure_wipe(input_buffer, sizeof(input_buffer));

    if (ret != AMNESIC_SUCCESS) {
        secure_wipe(raw_output, sizeof(raw_output));
        return ret;
    }

    char encoded_output[MAX_OUTPUT_SIZE];
    size_t encoded_len = 0;
    secure_wipe(encoded_output, sizeof(encoded_output));

    switch (active_config.encoding) {
        case ENCODE_HEX:
            ret = encode_hex(raw_output, raw_output_len, encoded_output, sizeof(encoded_output), &encoded_len);
            break;
        case ENCODE_BASE64:
            ret = encode_base64(raw_output, raw_output_len, encoded_output, sizeof(encoded_output), &encoded_len);
            break;
        case ENCODE_BASE85:
            ret = encode_base85(raw_output, raw_output_len, encoded_output, sizeof(encoded_output), &encoded_len);
            break;
        default:
            ret = AMNESIC_ERR_ENCODER;
            break;
    }

    secure_wipe(raw_output, sizeof(raw_output));

    if (ret == AMNESIC_SUCCESS) {
        if (active_config.is_child_terminal) {
            clear_terminal_screen();
            printf("============================================================\n");
            printf("         AMNESIC HASHER - AIR-GAPPED PASSKEY GENERATOR\n");
            printf("============================================================\n");

            const char *algo_str = (active_config.algo == ALGO_SHA512) ? "SHA-512" :
                                   (active_config.algo == ALGO_SHA256) ? "SHA-256" : "BLAKE3";
            const char *enc_str = (active_config.encoding == ENCODE_BASE64) ? "BASE64" :
                                  (active_config.encoding == ENCODE_BASE85) ? "BASE85" : "HEX";
            const char *mode_str = (active_config.mode == MODE_PROCESS) ? "PROCESS" :
                                   (active_config.mode == MODE_EXPLODE) ? "EXPLODE" : "SIMPLE";

            if (active_config.mode == MODE_PROCESS) {
                printf("  Mode: %s (Variant %d) | Engine: %s | Encoder: %s\n", mode_str, active_config.variant_id, algo_str, enc_str);
            } else {
                printf("  Mode: %s | Engine: %s | Encoder: %s\n", mode_str, algo_str, enc_str);
            }
            printf("============================================================\n\n");
            printf("GENERATED PASSKEY:\n%s\n\n", encoded_output);
            if (isatty(STDIN_FILENO)) {
                printf("------------------------------------------------------------\n");
                printf("Press <ENTER> to wipe memory and close terminal window...");
                fflush(stdout);
                getchar();
                clear_terminal_screen();
            }
        } else {
            clear_terminal_screen();
            printf("%s\n", encoded_output);
        }
    } else {
        fprintf(stderr, "Error: Processing or encoding output failed.\n");
    }

    secure_wipe(encoded_output, sizeof(encoded_output));

    return ret;
}
