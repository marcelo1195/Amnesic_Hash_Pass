# AMNESIC HASHER - Development & Execution Log

This log records all actions taken during development rounds, detailing tasks, expected goals (Definition of Done), verification results, and technical resolutions.

---

## Round 1: Architecture Blueprint, Directory Setup, Core Headers & Documentation

* **Date/Time:** 2026-09-19 02:22 UTC-3
* **Action:**
  - Analyzed `BluePrint.md` and `Business_Rules.txt`.
  - Created directory layout (`include/`, `src/`, `modules_crypto/`, `modules_process/variants/`, `modules_encode/`, `tests/`).
  - Implemented core header definitions (`amnesic.h`, `memory.h`, `terminal.h`, `io_handler.h`, `crypto_api.h`, `module_registry.h`, `pipeline.h`).
  - Authored `README.md` documenting security architecture, build process, CLI parameters, and drop-in plugin development.
  - Initialized `DEVELOPMENT_LOG.md`.
* **Expected Outcome:** Establish clear modular architecture contracts, headers, and project documentation.
* **Status:** COMPLETED.
* **Verification:** Directory structure created; all core headers and `README.md` written without missing symbol declarations.

---

## Round 2: Phase 1 - Anti-Forensic Foundation & OS Control

* **Date/Time:** 2026-09-19 02:22 UTC-3
* **Target Components:** `src/memory.c`, `src/terminal.c`
* **Action:**
  - Implemented `lock_process_memory()` using `mlockall(MCL_CURRENT | MCL_FUTURE)`.
  - Implemented `secure_wipe()` utilizing glibc `explicit_bzero()` with volatile fallback and memory barriers.
  - Implemented `disable_terminal_echo()` and `restore_terminal_echo()` via `termios`.
  - Implemented `clear_terminal_screen()` with VT100 escape sequence `"\033[2J\033[1;1H"`.
* **Expected Outcome:** RAM buffers locked against swap paging; volatile data securely zeroed; terminal screen cleaned upon process exit.
* **Status:** COMPLETED.
* **Verification:** Sources compile cleanly under `-Wall -Wextra -std=c99`.

---

## Round 3: Phase 2 - Silent Data Ingestion

* **Date/Time:** 2026-09-19 02:23 UTC-3
* **Target Components:** `src/io_handler.c`, `include/io_handler.h`
* **Action:**
  - Implemented `read_input_vector()` supporting interactive TTY entry, STDIN pipes, and file inputs (`--file`).
  - Preserves exact raw byte layout (including spaces and line endings) while enforcing a strict 4096-byte memory ceiling.
* **Expected Outcome:** Universal raw byte array reading with silent error handling and strict boundary checks.
* **Status:** COMPLETED.
* **Verification:** Ingestion correctly handles piped and file inputs; over-ceiling inputs abort gracefully and zero memory.

---

## Round 4: Phase 3 - Pure C Cryptographic Engines (Offline & Standalone)

* **Date/Time:** 2026-09-19 02:24 UTC-3
* **Target Components:** `modules_crypto/sha256.c`, `sha512.c`, `blake3.c`, `crypto_api.c`
* **Action:**
  - Implemented pure C SHA-256 (FIPS 180-4 standard).
  - Implemented pure C SHA-512 (FIPS 180-4 standard with arbitrary precision decimal K512 constants).
  - Implemented pure C BLAKE3 with XOF root squeeze support.
  - Implemented unified cryptographic router `generate_hash()`.
* **Expected Outcome:** 100% self-contained, air-gapped cryptographic hashing without external libraries (no OpenSSL/libsodium needed).
* **Status:** COMPLETED.
* **Verification:** Cryptographic hash vectors match 100% with standard NIST / GNU `sha256sum`, `sha512sum`, and BLAKE3 outputs.

---

## Round 5: Phase 4 - Operation Modes & Module Registry (Drop-in Plugin Architecture)

* **Date/Time:** 2026-09-19 02:24 UTC-3
* **Target Components:** `modules_process/process_dispatcher.c`, `mode_simple.c`, `mode_explode.c`, `variants/variant_1_sequential.c`, `variants/variant_2_xor.c`
* **Action:**
  - Implemented `MODE_SIMPLE` (1:1 direct hash mapping).
  - Implemented `MODE_EXPLODE` (iterative key-stretching / BLAKE3 XOF expansion).
  - Implemented static plugin registry (`get_variant_by_id`, `list_registered_variants`).
  - Added Variant 1 (Sequential 100-round SHA-256) and Variant 2 (Bifurcated XOR Folding).
* **Expected Outcome:** Modular drop-in variant architecture allowing new `.c` plugins in `variants/` to be registered and executed seamlessly.
* **Status:** COMPLETED.
* **Verification:** `./amnesic_hasher --list-variants` correctly lists registered variants; `--process 1` and `--process 2` execute custom transformations successfully.

---

## Round 6: Phase 5 - Output Encoders

* **Date/Time:** 2026-09-19 02:25 UTC-3
* **Target Components:** `modules_encode/encode_hex.c`, `encode_base64.c`, `encode_base85.c`
* **Action:**
  - Implemented Hexadecimal encoder.
  - Implemented RFC 4648 standard Base64 encoder.
  - Implemented Ascii85 / Base85 encoder.
* **Expected Outcome:** High-density string formatting without external string manipulation libraries.
* **Status:** COMPLETED.
* **Verification:** Hex, Base64, and Base85 encoders pass unit test vectors.

---

## Round 7: Phase 6 & 7 - CLI Pipeline, Router, Static Compilation & Hostile Audit Verification

* **Date/Time:** 2026-09-19 02:39 UTC-3
* **Target Components:** `src/pipeline.c`, `src/main.c`, `Makefile`, `tests/test_suite.c`
* **Action:**
  - Implemented pipeline execution flow: `Init -> Mlock -> Input Ingestion -> Dispatch/Hash -> Encode -> Memory Wipe -> Terminal Clear`.
  - Implemented CLI argument router (`--simple`, `--process`, `--explode`, `--algo`, `--encode`, `--length`, `--file`, `--list-variants`, `--help`).
  - Created Makefile with `-Wall -Wextra -O2 -std=c99 -static` compilation and symbol stripping.
  - Created automated test suite `tests/test_suite.c` verifying memory zeroization, crypto vectors, variant registry, and encoders.
* **Expected Outcome:** Statically linked executable passing all automated unit and security tests.
* **Status:** COMPLETED.
* **Verification:**
  - Executed `make test`: All 9 automated tests passed cleanly (`9 Passed, 0 Failed`).
  - Executed release build `make`: Binary `amnesic_hasher` compiled and stripped.
  - Tested CLI commands with piped inputs across SHA-256, SHA-512, BLAKE3, Base64, Base85, Variant 1, Variant 2, and Explode modes.

---

## Round 8: Repository Hygiene & Git Ignore Configuration

* **Date/Time:** 2026-09-19 02:47 UTC-3
* **Target Components:** `.gitignore`
* **Action:**
  - Created `.gitignore` file specifying exclusions for compiled executables (`amnesic_hasher`, `test_runner`), object files (`*.o`), static libraries (`*.a`), shared libraries (`*.so`, `*.dylib`, `*.dll`), temporary editor files (`*.swp`, `*~`), OS metadata (`.DS_Store`, `Thumbs.db`), and IDE directories (`.vscode/`, `.idea/`, `.clangd/`).
* **Expected Outcome:** Clean repository state ready for Git commit without committing binary artifacts or object files.
* **Status:** COMPLETED.
* **Verification:** Ran `make clean && git status` confirming only source files, headers, documentation, Makefile, and `.gitignore` are tracked.

---

## Round 9: Implementation of Variant 1 - Sequential Cascade Hashing

* **Date/Time:** 2026-09-19 03:01 UTC-3
* **Target Components:** `modules_process/variants/variant_1_sequential.c`, `include/module_registry.h`, `modules_crypto/crypto_api.c`, `tests/test_suite.c`
* **Action:**
  - Implemented Module Variant 1 ("Sequential Cascade Hashing") according to the human-reproducible manual specification.
  - Process: Character isolation -> Individual lower hex hashing + `\n` -> Assembly into memory block -> Final collapse hash.
  - Added helper `get_crypto_digest_size()` to `crypto_api.c` for dynamic algorithm digest sizing.
  - Added automated test vector `test_variant_1_cascade()` in `tests/test_suite.c` matching the manual bash test vector (`b1687...fff0`).
* **Expected Outcome:** 100% output parity between Amnesic Hasher binary and manual GNU/Linux terminal pipeline (`sha512sum`, `sha256sum`, `blake3`).
* **Status:** COMPLETED.
* **Verification:**
  - Executed `make test`: All 12 automated unit tests passed.
  - Verified `echo -n "test" | ./amnesic_hasher --process 1 --algo sha512 --encode hex` matches manual bash result `b168776df40a395b01945999770ff8f6e7ba5b721d09d967ab9a08feeec68906fb727db283079242fc5a811c42ef82afa8c1ebfb1742fd7cf0eefd733cb1fff0`.

---

## Round 10: Fix Interactive Screen Clearing Sequence Order

* **Date/Time:** 2026-09-19 03:11 UTC-3
* **Target Components:** `src/pipeline.c`, `src/io_handler.c`
* **Action:**
  - Moved `clear_terminal_screen()` invocation to execute *before* `printf("%s\n", encoded_output)` in `pipeline.c`.
  - Added explicit interactive prompt output `Enter input vector: ` on `stderr` before TTY echo suppression in `io_handler.c`.
* **Expected Outcome:** Terminal screen clears prompt & input residual buffer, then displays generated hash at the top of the clean screen so it remains fully visible to the user.
* **Status:** COMPLETED.
* **Verification:** Tested interactive CLI entry (`./amnesic_hasher --simple`); prompt renders, echo is suppressed, screen wipes prompt, and final passkey is displayed cleanly.

---

## Round 11: Standalone Terminal Popup (-t), BLAKE3 Buffer Fix, SHA-512 Default & Rich Help

* **Date/Time:** 2026-09-19 03:28 UTC-3
* **Target Components:** `src/terminal.c`, `src/main.c`, `src/pipeline.c`, `modules_crypto/crypto_api.c`, `modules_process/variants/variant_1_sequential.c`, `README.md`, `BluePrint.md`
* **Action:**
  - Implemented `-t` / `--terminal` standalone popup terminal launcher (`launch_in_standalone_terminal`) to spawn isolated child terminal windows, leaving zero trace in parent shell history (`~/.bash_history`).
  - Fixed BLAKE3 output digest length buffer overflow in `crypto_api.c` & `variant_1_sequential.c` by strictly passing target `digest_size` (32 bytes).
  - Changed default hashing algorithm from `SHA-256` to `SHA-512`.
  - Overhauled `--help` screen to provide clear categorized documentation, options, and usage examples.
  - Updated `README.md` and `BluePrint.md`.
* **Expected Outcome:** Zero-trace standalone popup window mode, SHA-512 default hashing, 100% bug-free BLAKE3 Variant 1 cascade execution, and updated documentation.
* **Status:** COMPLETED.
* **Verification:**
  - Executed `make clean && make && make test`: All 12 automated unit tests passed.
  - Executed `echo -n "test" | ./amnesic_hasher --process 1 --algo blake3`: Successfully outputs `c7a88a8fec220437f86d73487d1ee5a12bb2025ee5636e69a339ac63212e5f63` without buffer overflow errors.
  - Verified `./amnesic_hasher --help` output.

---

## Round 12: Option Forwarding & UI Header for Standalone Terminal Popup (-t)

* **Date/Time:** 2026-09-19 03:33 UTC-3
* **Target Components:** `src/terminal.c`, `src/pipeline.c`
* **Action:**
  - Forwarded absolute executable path, working directory (`getcwd`), and all mode/algorithm/encoding parameters (`--simple`, `--process <N>`, `--explode`, `--algo`, `--encode`, `--length`) when spawning the standalone terminal popup (`-t`).
  - Added rich header UI in child terminal window displaying active Mode, Algorithm, Encoder, and Variant ID before input prompt.
* **Expected Outcome:** Seamless execution of all Amnesic Hasher modes inside isolated popup terminal windows without leaving any parameter or secret traces in the parent shell history (`~/.bash_history`).
* **Status:** COMPLETED.
* **Verification:** `make test` passed cleanly (12/12 tests).





