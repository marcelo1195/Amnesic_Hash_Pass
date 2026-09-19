

# AMNESIC HASHER: Master Blueprint & Development Plan

## 1. Project Overview
The **Amnesic Hasher** is an air-gapped, modular CLI utility written in C. It is designed strictly to generate high-entropy deterministic passkeys (seeds) from simple inputs. 

**Core Philosophies:**
* **Anti-Forensic by Design:** The tool leaves no volatile footprint. It utilizes strict memory pinning (`mlockall`) to prevent swap paging and `explicit_bzero` for mandatory zeroization of all buffers before exit.
* **Air-Gapped Independence:** The binary must be statically compiled with zero external dynamic dependencies. All cryptographic engines are embedded as raw C files.
* **Drop-in Modularity:** The processing layer operates on a static plugin architecture. Developers can add new mathematical transformations (variants) simply by dropping a new `.c` file into the `variants/` folder and registering its function pointer.

---

## 2. Directory Structure (V1.2 Modular Architecture)

```text
amnesic-hasher/
├── Makefile                 # Static compilation rules (-static, strip)
├── src/
│   ├── main.c               # CLI router, argument parsing, secure exit
│   ├── memory.c             # mlock wrappers, explicit_bzero logic
│   ├── terminal.c           # TTY echo control, VT100 screen clearing
│   ├── io_handler.c         # Handles STDIN, --file, and TTY silently
│   └── pipeline.c           # Orchestrates Mode -> Algo -> Encoder
├── include/
│   ├── amnesic.h            # Global macros and core prototypes
│   ├── crypto_api.h         # Unified interface for hash engines
│   └── module_registry.h    # Defines the 'ProcessVariant' function pointer struct
├── modules_crypto/          # Local, raw C Hash Engines
│   ├── sha256.c / .h
│   ├── sha512.c / .h
│   └── blake3.c / .h        
├── modules_process/         # The Operation Modes
│   ├── mode_simple.c        # Direct 1:1 hashing 
│   ├── mode_explode.c       # Iterative/XOF entropy expansion
│   ├── process_dispatcher.c # Reads registry and routes to [N] variant
│   └── variants/            # DROP-IN FOLDER FOR CUSTOM MODULES
│       ├── variant_1_sequential.c
│       └── variant_2_xor.c
└── modules_encode/          # Output Formatters
    ├── encode_hex.c
    ├── encode_base64.c
    └── encode_base85.c

```

---

## 3. Development Plan & Component Specifications

The development lifecycle is divided into 7 sequential phases. A component is only considered "Complete" when it passes its strict Definition of Done (DoD).

### PHASE 1: Anti-Forensic Foundation & OS Control

Before any sensitive data is ingested, the hostile environment (RAM and Screen) must be tamed.

* **Component:** `memory.c`
* **Functionality:** Wraps `mlockall(MCL_CURRENT | MCL_FUTURE)` to lock the process pages in RAM. Implements `secure_wipe()` using `explicit_bzero()` (or compiler-specific pragmas) to ensure zeroization cannot be optimized away.
* **DoD:** A test compilation runs, and tools like `htop` or `/proc/[pid]/status` confirm `VmSwap` is 0. Memory dumps confirm buffers are physically overwritten with zeros.


* **Component:** `terminal.c`
* **Functionality:** Uses `termios` to disable TTY echo during interactive input. Issues VT100 escape sequence (`\033[2J\033[1;1H`) upon exit to annihilate the visual buffer.
* **DoD:** Passwords typed interactively do not appear on the screen. The terminal screen is completely wiped upon process termination, defeating mouse-scroll shoulder surfing.



### PHASE 2: Silent Data Ingestion

Safely routing input vectors without leaking data to disk or `argv`.

* **Component:** `io_handler.c`
* **Functionality:** Uses `isatty(STDIN_FILENO)` to detect pipe redirection. Reads `--file` inputs as raw byte blocks. Enforces a strict memory ceiling (e.g., 4096 bytes) to prevent buffer overflows.
* **DoD:** Can successfully ingest a multi-line text file containing spaces and line-breaks (`\n`) perfectly preserving the exact layout in the locked RAM buffer without truncating at the first newline.



### PHASE 3: Cryptographic Engines (The Core)

Integrating the mathematical hashing algorithms offline.

* **Component:** `modules_crypto/*` & `crypto_api.h`
* **Functionality:** Raw C implementations of SHA-256, SHA-512, and BLAKE3 (XOF). `crypto_api.h` abstracts these so the pipeline only calls `generate_hash(ALGO_ID, input, output)`.
* **DoD:** Passing the string "Amnesic" through the isolated SHA-512 module produces a raw byte array that matches 100% with the output of the GNU `echo -n "Amnesic" | sha512sum` command.



### PHASE 4: The Module Registry (Plugin Architecture)

The core of the tool's extensibility.

* **Component:** `module_registry.h`, `process_dispatcher.c`, and `variants/*`
* **Functionality:** Defines the `ProcessVariant` struct containing `id`, `name`, `description`, and a `(*process_func)` pointer. The dispatcher parses `--process [N]` and routes the raw buffer to the corresponding variant in the array.
* **DoD:** A developer can drop a new file `variant_3_custom.c` into the folder, add it to the registry array, and run `./amnesic_hasher --process 3` to execute the new logic without altering a single line of the core `main.c` pipeline. The command `--list-variants` correctly prints the new module's name.



### PHASE 5: Output Encoders

Translating raw entropy bytes into usable, high-density strings.

* **Component:** `modules_encode/*`
* **Functionality:** Takes the raw byte array output from Phase 3/4 and encodes it into Hexadecimal, Base64, or Base85 (Ascii85).
* **DoD:** Passing a 64-byte raw buffer (512 bits) into the Base85 encoder reliably outputs exactly 80 ASCII characters, correctly padded, with no illegal symbols.



### PHASE 6: CLI Orchestration & Compilation

Tying the pipeline together securely.

* **Component:** `main.c`, `pipeline.c`, `Makefile`
* **Functionality:** `main.c` parses `argv` (ensuring the secret is never passed here). `pipeline.c` executes the flow: `Init -> Mlock -> Input -> Dispatch -> Hash -> Encode -> Wipe -> Exit`. The `Makefile` builds the binary.
* **DoD:** The `Makefile` utilizes `-O2` and `-static`. Running `ldd amnesic_hasher` reports "not a dynamic executable". Running `strip --strip-all` successfully removes all debugging symbols to hinder reverse engineering.



### PHASE 7: Hostile Audit (Production Readiness)

The tool is not complete until it survives aggressive laboratory testing.

* **Audit 1: Memory Forensics:**
* **DoD:** Attaching GDB and forcing a core dump immediately after the `secure_wipe()` function executes proves that the raw secret, the intermediate hashes, and the final output string have been physically overwritten with `\0` in RAM.


* **Audit 2: Fuzzing & Overflow:**
* **DoD:** Piping 50MB of random garbage data (`/dev/urandom`) into the tool via STDIN does not cause a Segmentation Fault. The tool hits the 4096-byte limit, gracefully aborts, wipes memory, and exits safely.


* **Audit 3: Valgrind (Leak Detection):**
* **DoD:** Running the tool under `valgrind` reports "0 bytes in 0 blocks are definitely lost" indicating no memory leaks (which could leave key fragments stranded in the Heap).



