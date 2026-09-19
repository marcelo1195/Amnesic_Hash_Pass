#ifndef AMNESIC_H
#define AMNESIC_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define MAX_INPUT_SIZE 4096
#define MAX_OUTPUT_SIZE 8192

#define AMNESIC_SUCCESS 0
#define AMNESIC_ERR_MEMORY -1
#define AMNESIC_ERR_IO -2
#define AMNESIC_ERR_ALGO -3
#define AMNESIC_ERR_VARIANT -4
#define AMNESIC_ERR_ENCODER -5
#define AMNESIC_ERR_ARGS -6

typedef enum {
    MODE_SIMPLE = 1,
    MODE_PROCESS = 2,
    MODE_EXPLODE = 3
} AmnesicMode;

typedef enum {
    ALGO_SHA256 = 1,
    ALGO_SHA512 = 2,
    ALGO_BLAKE3 = 3
} CryptoAlgo;

typedef enum {
    ENCODE_HEX = 1,
    ENCODE_BASE64 = 2,
    ENCODE_BASE85 = 3
} OutputEncoding;

#endif
