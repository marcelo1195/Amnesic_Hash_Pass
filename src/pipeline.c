#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

int run_amnesic_pipeline(const PipelineConfig *config) {
    if (!config) {
        return AMNESIC_ERR_ARGS;
    }

    lock_process_memory();

    unsigned char input_buffer[MAX_INPUT_SIZE];
    size_t input_len = 0;
    secure_wipe(input_buffer, sizeof(input_buffer));

    int ret = read_input_vector(config->file_path, input_buffer, MAX_INPUT_SIZE, &input_len);
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

    switch (config->mode) {
        case MODE_SIMPLE:
            ret = execute_mode_simple(config->algo, input_buffer, input_len, raw_output, config->length, &raw_output_len);
            break;

        case MODE_PROCESS: {
            const ProcessVariant *variant = get_variant_by_id(config->variant_id);
            if (!variant || !variant->process_func) {
                fprintf(stderr, "Error: Invalid or unregistered variant ID [%d].\n", config->variant_id);
                ret = AMNESIC_ERR_VARIANT;
                break;
            }
            ret = variant->process_func(config->algo, input_buffer, input_len, raw_output, sizeof(raw_output), &raw_output_len);
            break;
        }

        case MODE_EXPLODE:
            ret = execute_mode_explode(config->algo, input_buffer, input_len, config->length, raw_output, sizeof(raw_output), &raw_output_len);
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

    switch (config->encoding) {
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
        if (config->is_child_terminal) {
            clear_terminal_screen();
            printf("============================================================\n");
            printf("         AMNESIC HASHER - AIR-GAPPED PASSKEY GENERATOR\n");
            printf("============================================================\n\n");
            printf("GENERATED PASSKEY:\n%s\n\n", encoded_output);
            printf("------------------------------------------------------------\n");
            printf("Press <ENTER> to wipe memory and close terminal window...");
            fflush(stdout);
            getchar();
            clear_terminal_screen();
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
