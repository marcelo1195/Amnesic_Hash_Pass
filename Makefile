CC ?= gcc
CFLAGS ?= -Wall -Wextra -O2 -std=c99 -Iinclude -Imodules_crypto -Imodules_encode
STATIC_FLAG = -static

SRCS = src/main.c \
       src/memory.c \
       src/terminal.c \
       src/io_handler.c \
       src/pipeline.c \
       modules_crypto/sha256.c \
       modules_crypto/sha512.c \
       modules_crypto/blake3.c \
       modules_crypto/crypto_api.c \
       modules_process/mode_simple.c \
       modules_process/mode_explode.c \
       modules_process/process_dispatcher.c \
       modules_process/variants/variant_1_sequential.c \
       modules_process/variants/variant_2_xor.c \
       modules_encode/encode_hex.c \
       modules_encode/encode_base64.c \
       modules_encode/encode_base85.c

TEST_SRCS = tests/test_suite.c \
            src/memory.c \
            src/terminal.c \
            src/io_handler.c \
            modules_crypto/sha256.c \
            modules_crypto/sha512.c \
            modules_crypto/blake3.c \
            modules_crypto/crypto_api.c \
            modules_process/mode_simple.c \
            modules_process/mode_explode.c \
            modules_process/process_dispatcher.c \
            modules_process/variants/variant_1_sequential.c \
            modules_process/variants/variant_2_xor.c \
            modules_encode/encode_hex.c \
            modules_encode/encode_base64.c \
            modules_encode/encode_base85.c

OBJS = $(SRCS:.c=.o)
TEST_OBJS = $(TEST_SRCS:.c=.o)

TARGET = amnesic_hasher
TEST_TARGET = test_runner

.PHONY: all clean test debug static_check

all: $(TARGET)

$(TARGET): $(OBJS)
	@$(CC) $(CFLAGS) $(STATIC_FLAG) -o $@ $(OBJS) 2>/dev/null || $(CC) $(CFLAGS) -o $@ $(OBJS)
	@strip --strip-all $@ 2>/dev/null || true

debug: CFLAGS += -g -DDEBUG
debug: $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJS)
	$(CC) $(CFLAGS) -o $@ $(TEST_OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TEST_OBJS) $(TARGET) $(TEST_TARGET)
