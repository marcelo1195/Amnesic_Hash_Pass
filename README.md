# Amnesic Hasher

**Amnesic Hasher** is an air-gapped, modular Command-Line Interface (CLI) utility written in pure C (C99). It is designed to deterministically transform simple input vectors into high-entropy passkeys and cryptographic seeds while guaranteeing strict anti-forensic security.

---

## Security & Anti-Forensic Principles

1. **Zero Volatile Footprint**
   - Mandatory `explicit_bzero` memory sanitization wipes all sensitive input, intermediate buffers, and output buffers before process exit.
2. **Swap Paging Prevention**
   - Memory pages are locked in physical RAM using `mlockall(MCL_CURRENT | MCL_FUTURE)` to prevent secret data from being written to Linux swap partitions or swap files (`VmSwap = 0`).
3. **Terminal Hygiene**
   - Disables TTY echo during interactive passkey entry to prevent shoulder surfing.
   - Clears terminal screens using VT100 control sequences (`\033[2J\033[1;1H`) upon process termination.
4. **Air-Gapped & Offline Compilation**
   - 100% self-contained code base. Requires zero external runtime or cryptographic dynamic libraries (no OpenSSL, libsodium, or network dependencies).
   - Statically compiled binary using `-static` and stripped of debug symbols.

---

## Directory Structure

```text
Amnesic_Hash_Pass/
├── Makefile                 # Static compilation rules (-static, strip, tests)
├── README.md                # Project documentation
├── DEVELOPMENT_LOG.md       # Development iteration log
├── include/
│   ├── amnesic.h            # Global macros and core definitions
│   ├── crypto_api.h         # Unified interface for cryptographic hash engines
│   ├── io_handler.h         # Silent input vector ingestion prototypes
│   ├── memory.h            # Memory locking and zeroization prototypes
│   ├── module_registry.h    # Static plugin registry prototypes
│   ├── pipeline.h          # Execution pipeline prototypes
│   └── terminal.h          # TTY echo control and VT100 clearing prototypes
├── src/
│   ├── main.c               # CLI router and argument parser
│   ├── memory.c             # mlock & explicit_bzero implementations
│   ├── terminal.c           # termios and VT100 terminal handling
│   ├── io_handler.c         # STDIN, file, and TTY input processing
│   └── pipeline.c           # Mode -> Hash -> Encoder pipeline orchestration
├── modules_crypto/          # Standalone C Hash Engines
│   ├── sha256.c / .h        # Pure C SHA-256 implementation
│   ├── sha512.c / .h        # Pure C SHA-512 implementation
│   ├── blake3.c / .h        # Pure C BLAKE3 (XOF capable) implementation
│   └── crypto_api.c         # Unified hash dispatcher
├── modules_process/         # Operation Modes & Dispatcher
│   ├── mode_simple.c        # Direct 1:1 hashing mode
│   ├── mode_explode.c       # Iterative/XOF entropy expansion mode
│   ├── process_dispatcher.c # Variant registry lookup and routing
│   └── variants/            # Drop-in folder for custom transformation modules
│       ├── variant_1_sequential.c
│       └── variant_2_xor.c
├── modules_encode/          # Standalone Output Encoders
│   ├── encode_hex.c / .h    # Lowercase & uppercase hexadecimal formatting
│   ├── encode_base64.c / .h # RFC 4648 Base64 encoding
│   └── encode_base85.c / .h # Base85 / Ascii85 encoding
└── tests/
    └── test_suite.c         # Automated unit and security verification suite
```

---

## Compilation

The utility can be compiled locally in any Linux environment with standard `gcc` tools without internet connectivity.

```bash
# Build release binary (statically linked and stripped)
make

# Build debug binary (with debugging symbols)
make debug

# Run unit and cryptographic vector tests
make test

# Clean build artifacts
make clean
```

---

## Usage & CLI Options

```bash
amnesic_hasher [MODE] [OPTIONS]
```

### Modes (Select One)
- `--simple` : Direct 1:1 hash mapping (equivalent to standard GNU hash utilities).
- `--process <N>` : Process input using registered transformation variant `<N>`.
- `--explode` : Entropy expansion mode using iterative hashing / BLAKE3 XOF.

### Cryptographic Algorithms (`--algo`)
- `sha256` : SHA-256 (default)
- `sha512` : SHA-512
- `blake3` : BLAKE3 (supports arbitrary output lengths)

### Encoders (`--encode`)
- `hex` : Hexadecimal format (default)
- `base64` : Standard Base64 format
- `base85` : High-density Ascii85 format

### Additional Options
- `--length <N>` : Output size in bytes (used with BLAKE3 / Explode mode, default 32)
- `--file <PATH>` : Read input vector from file instead of interactive TTY / STDIN
- `--list-variants` : List all registered drop-in processing variants
- `--help` : Show help screen

---

## Examples

1. **Interactive passkey generation using SHA-512 and Base85 encoding:**
   ```bash
   ./amnesic_hasher --simple --algo sha512 --encode base85
   ```

2. **Pipe input file through Variant 2 (XOR Bifurcation) encoded in Base64:**
   ```bash
   ./amnesic_hasher --process 2 --file my_seed.txt --encode base64
   ```

3. **Expand entropy to 64 bytes using BLAKE3 XOF in Explode mode:**
   ```bash
   echo -n "MasterKey" | ./amnesic_hasher --explode --algo blake3 --length 64 --encode hex
   ```

---

## Creating Drop-in Processing Variants

To add a new custom transformation variant:
1. Create a new `.c` file inside `modules_process/variants/` (e.g., `variant_3_custom.c`).
2. Implement a function matching the `ProcessVariantFunc` signature:
   ```c
   int process_variant_3(const unsigned char *input, size_t input_len,
                         unsigned char *output, size_t max_out_len, size_t *out_len);
   ```
3. Register the function in `modules_process/process_dispatcher.c` within the static registry array `g_variants`.
4. Recompile with `make`.
