[![Windows MSVC Tests](https://github.com/apotocki/dataforge/actions/workflows/msvc-tests.yml/badge.svg)](https://github.com/apotocki/dataforge/actions/workflows/msvc-tests.yml)
[![Linux GCC Tests](https://github.com/apotocki/dataforge/actions/workflows/linux-tests.yml/badge.svg)](https://github.com/apotocki/dataforge/actions/workflows/linux-tests.yml)
[![macOS Tests](https://github.com/apotocki/dataforge/actions/workflows/macos-tests.yml/badge.svg)](https://github.com/apotocki/dataforge/actions/workflows/macos-tests.yml)
# DataForge

**DataForge** is a modern C++20 header-only library for building declarative, composable data transformation pipelines.  
It provides both push (output) and pull (input) iterator-based interfaces for applying arbitrary chains of conversions, including encoding, decoding, compression, encryption, hashing, and Unicode operations.  
Transformations are described using *quarks* — small, composable objects that can be chained together with the `|` operator.

## Quick Example

```cpp
#include "dataforge/quark_push_iterator.hpp"
#include "dataforge/quark_pull_iterator.hpp"
#include "dataforge/base_xx/base64.hpp"

using namespace dataforge;

std::string input = "Hello, World!";
std::string base64_result;

// Create a pipeline: input bytes → Base64 encoding → output
auto push_it = quark_push_iterator(int8 | base64, std::back_inserter(base64_result));
*push_it = input;
push_it.finish();

std::cout << "Encoded: " << base64_result << std::endl;  // Output: SGVsbG8sIFdvcmxkIQ==

// Reverse the process: Base64 → decoded bytes
std::string decoded_result;
auto pull_it = quark_pull_iterator(base64 | int8, base64_result);
for (auto span = *pull_it; !span.empty(); span = *++pull_it) {
    std::copy(span.begin(), span.end(), std::back_inserter(decoded_result));
}

std::cout << "Decoded: " << decoded_result << std::endl;  // Output: Hello, World!
```

**More complex pipelines** can chain multiple transformations:
```cpp
// Example: text → UTF-8 → compression → encryption → Base64
auto pipeline = utf8 | deflated() | aes(128, key) | base64;
```

> 📁 **See the [examples/](examples/) folder for complete working examples** including MD5 hashing, AES encryption, and more advanced use cases.
> 
> 🧪 **For comprehensive algorithm coverage and advanced pipeline patterns, explore the [tests/](tests/) directory** — it contains hundreds of real-world examples demonstrating every supported algorithm, from basic CRC checksums to complex multi-stage encryption pipelines.

## Why DataForge is Unique

DataForge combines multiple types of data transformations in one consistent framework, unlike other libraries that cover only subsets of functionality.

| Feature / Capability         | DataForge | Crypto++ | Boost | ICU | range-v3 |
|-------------------------------|:---------:|:--------:|:----:|:---:|:--------:|
| Integer ↔ Bytes + Endian     | ✅        | ❌       | ❌   | ❌  | ❌       |
| base16/32/58/64/ascii85/z85  | ✅        | ✅       | ❌   | ❌  | ❌       |
| Custom Base 1 < N < 256      | ✅        | ❌       | ❌   | ❌  | ❌       |
| Checksums (crc, adler, bsd) | ✅        | ❌       | ❌   | ❌  | ❌       |
| Hashes (MD, SHA, Blake, etc)| ✅        | ✅       | ❌   | ❌  | ❌       |
| Encryption/Decryption        | ✅        | ✅       | ❌   | ❌  | ❌       |
| Compression / Decompression  | ✅        | ❌       | ❌   | ❌  | ❌       |
| Unicode Conversions (UTF)    | ✅        | ❌       | ❌   | ✅  | ❌       |
| ICU Charset Conversions      | ✅        | ❌       | ❌   | ✅  | ❌       |
| Grapheme Breaking            | ✅        | ❌       | ❌   | ✅  | ❌       |
| Header-only                  | ✅        | ❌       | ✅   | ❌  | ✅       |
| Push/Pull iterator pipelines | ✅        | ❌       | ✅ (filters) | ❌ | ✅       |

**Key point:** DataForge allows chaining transformations like integer → endian → compression → encryption → base encoding in one declarative pipeline.

## Key Features

### 1. Integer ↔ Byte sequence conversions (with endianness)
- Convert sequences of integers of various sizes to/from byte sequences.
- Configurable **little-endian** or **big-endian** representation.

### 2. Encoding / Decoding
- **Base16, Base32, Base58, Base64, ASCII85, Z85**.
- Arbitrary base conversion with `1 < N < 256` and a custom alphabet — effectively a positional numeral system transformation.

### 3. Checksums
- BSD checksum
- Adler32
- CRC8, CRC16, CRC32, CRC64

### 4. Hash Functions
- MD2, MD4, MD5, MD6
- RIPEMD, Tiger
- SHA1, SHA2, SHA3
- Belt, GOST, Streebog, Whirlpool, Blake

### 5. Encryption / Decryption
- RC2, RC4, RC5, RC6
- DES, AES, Blowfish
- Belt, Magma

### 6. Compression / Decompression
- Deflate
- Bzip2
- LZ4
- LZMA, LZMA2  
*(requires corresponding external libraries)*

### 7. Unicode Encoding Conversions
- UTF-7, UTF-8, UTF-16, UTF-32

### 8. ICU-based String Encoding Conversions
- Any encoding supported by the [ICU library](https://icu.unicode.org/)  
*(requires ICU library)*

### 9. Grapheme Breaker
- Splits a Unicode string into graphemes according to the [Unicode Standard](https://unicode.org/reports/tr29/).


## Installation for Running Tests

The library itself is **header-only** — nothing needs to be built for use in your projects.  
However, the test suite depends on external libraries (**zlib, icu, bzip2, lz4, liblzma, gtest**), which are managed via [vcpkg](https://github.com/microsoft/vcpkg).

### Steps to build and run tests:
1. **Install vcpkg** anywhere on your system (if not already installed).
2. **Set the environment variable** `VCPKG_ROOT` to the location of your vcpkg installation.  
   - Example (Windows PowerShell):
     ```powershell
     setx VCPKG_ROOT "C:\dev\vcpkg"
     ```
3. Open the Visual Studio solution for tests and build it.  
   - On the first build:
     - The project will automatically:
       1. Check that `VCPKG_ROOT` is set.
       2. Run:
          ```powershell
          $(VCPKG_ROOT)\vcpkg.exe install
          ```
          installing all required dependencies from `vcpkg.json` into a local `vcpkg_installed` folder.
       3. Configure `INCLUDE` and `LIB` paths to use these locally installed dependencies.
4. Run the tests from Visual Studio.

**No global vcpkg integration (`vcpkg integrate install`) is required** — everything is local to the repository.

## Hardware Acceleration

The **SHA-1** and **SHA-2** families support compile-time selectable
hardware-accelerated block processing. All variants always have a portable scalar
fallback, so acceleration never changes results — only throughput.

### Acceleration profile

A single macro `DATAFORGE_ACCEL_PROFILE` selects the ISA tier for **all**
accelerated algorithms. Set it via `-DDATAFORGE_ACCEL_PROFILE=<value>`:

| Value | Name | Description |
|------:|------|-------------|
| `-1` | `AUTO` *(default)* | Runtime CPU detection — picks the best backend at first use via CPUID / `getauxval`, cached for the lifetime of the process. |
| `0` | `SCALAR` | Pure C++ — no ISA intrinsics. Compiles and runs anywhere. |
| `1` | `X86_SHA_NI` | x86 forced: SHA-NI + SSE4.1 for SHA-1/224/256; SSE4.1 for SHA-512 family. No runtime probing — CPU **must** support these. |
| `2` | `X86_AVX512` | x86 forced: SHA-NI for SHA-1/224/256 + AVX-512 message schedule for **all** SHA-2 variants including SHA-224/256. CPU **must** support AVX-512F/VL. |
| `3` | `ARM_NEON` | AArch64 forced: NEON-vectorised SHA-512 message schedule; SHA-1/224/256 use scalar. Runs on **any** AArch64 CPU. |
| `4` | `ARM_CRYPTO` | AArch64 forced: SHA2 crypto extensions for SHA-1/224/256; SHA-512 hardware extension (with NEON fallback). CPU **must** support ARMv8+crypto. |

> **Note:** "Forced" profiles (`1`–`4`) call intrinsics directly without any
> runtime CPU check. Running a forced binary on a CPU that lacks the required ISA
> will cause an illegal-instruction signal. A profile targeting the wrong
> architecture (e.g. `X86_SHA_NI` on ARM) is silently downgraded to `SCALAR`.

### Backends

#### SHA-1

- **x86 SHA-NI** — Intel SHA Extensions (`_mm_sha1rnds4` / `_mm_sha1nexte` /
  `_mm_sha1msg1` / `_mm_sha1msg2`). Fastest single-block path for SHA-1.
  Used by `X86_SHA_NI`, `X86_AVX512`, and `AUTO` (when CPUID detects SHA).
- **ARM SHA1** — AArch64 crypto extensions (`vsha1cq` / `vsha1pq` / `vsha1mq` /
  `vsha1su0q` / `vsha1su1q`). Used by `ARM_CRYPTO` and `AUTO` (when
  `HWCAP_SHA1` is set in `getauxval(AT_HWCAP)`).

#### SHA-2 (224 / 256)

- **x86 SHA-NI** — Intel SHA Extensions (`_mm_sha256rnds2` / `msg1` / `msg2`).
  Fastest single-block path. Used by `X86_SHA_NI`, `X86_AVX512`, and `AUTO`.
- **x86 AVX-512** — vectorised message schedule (AVX-512F + AVX-512VL, `vprord`)
  for SHA-224/256. Used by `X86_AVX512` only (SHA-NI is faster per block, so it
  is never selected in `AUTO` mode for this family).
- **ARM SHA2** — AArch64 crypto extensions (`vsha256hq` / `vsha256su*`).
  Used by `ARM_CRYPTO` and `AUTO` (when `HWCAP_SHA2` is present).

#### SHA-2 (384 / 512 / 512-224 / 512-256)

- **x86 AVX-512** — vectorised message schedule using `vprorq` (64-bit lane
  rotate), 8-way unrolled compression. Used by `X86_AVX512` and `AUTO` (when
  AVX-512F/VL is available and the OS has enabled the register state via `XCR0`).
- **x86 SSE4.1** — same two-wide XMM schedule but 64-bit rotations decomposed
  into shift-or pairs. Used by `X86_SHA_NI` and `AUTO` (when AVX-512 is absent).
  Present on virtually all x86-64 hardware since ~2008.
- **ARM SHA-512** — ARMv8.2-A extension (`vsha512hq` / `vsha512h2q` /
  `vsha512su0q` / `vsha512su1q`). Used by `ARM_CRYPTO` and `AUTO` (when
  `HWCAP2_SHA512` is set in `getauxval(AT_HWCAP2)`).
- **ARM NEON** — vectorised message schedule using baseline NEON 64-bit lane
  operations (shift-or pairs). Available on **all AArch64** CPUs (Cortex-A53
  and later). Used by `ARM_NEON`, `ARM_CRYPTO` (as fallback when SHA-512
  extension is absent), and `AUTO` (when SHA-512 extension is absent).

On GCC/Clang the intrinsics for each backend are enabled per-function via
`__attribute__((target(...)))`, so no global `-msha` / `-mavx512*` /
`-march=armv8-a+sha2` / `-march=armv8.2-a+sha3` flags are needed to *build*
them; on MSVC they are always available.

### How `AUTO` chooses a backend

| Algorithm | x86 | AArch64 |
|-----------|-----|---------|
| SHA-1 | SHA-NI → scalar | SHA1 crypto ext → scalar |
| SHA-224/256 | SHA-NI → scalar | SHA2 crypto ext → scalar |
| SHA-384/512/… | AVX-512 → SSE4.1 → scalar | SHA-512 ext → NEON → scalar |

### CMake / compiler examples

```bash
# Default (auto-detect)
cmake -S . -B build

# Force scalar everywhere
cmake -S . -B build -DDATAFORGE_ACCEL_PROFILE=0

# Force x86 SHA-NI + SSE4.1  (profile 1)
cmake -S . -B build -DDATAFORGE_ACCEL_PROFILE=1

# Force x86 AVX-512 for all SHA-2 including SHA-224/256  (profile 2)
cmake -S . -B build -DDATAFORGE_ACCEL_PROFILE=2

# Force AArch64 NEON for SHA-512, scalar for SHA-1/256  (profile 3)
cmake -S . -B build -DDATAFORGE_ACCEL_PROFILE=3

# Force AArch64 crypto extensions (SHA2 + SHA-512)  (profile 4)
cmake -S . -B build -DDATAFORGE_ACCEL_PROFILE=4
```

Without CMake (direct compiler define):

```bash
g++ ... -DDATAFORGE_ACCEL_PROFILE=2
```

## License

Distributed under the [Boost Software License, Version 1.0](LICENSE).

---

## As an advertisement...
The Dataforge library is used in my iOS application on the App Store:

[<table align="center" border=0 cellspacing=0 cellpadding=0><tr><td><img src="https://is4-ssl.mzstatic.com/image/thumb/Purple112/v4/78/d6/f8/78d6f802-78f6-267a-8018-751111f52c10/AppIcon-0-1x_U007emarketing-0-10-0-85-220.png/460x0w.webp" width="70"/></td><td><a href="https://apps.apple.com/us/app/potohex/id1620963302">PotoHEX</a><br>HEX File Viewer & Editor</td><tr></table>]()

This application is designed to view and edit files at the byte or character level; calculate different hashes, encode/decode, and compress/decompress desired byte regions.
  
You can support my open-source development by trying the [App](https://apps.apple.com/us/app/potohex/id1620963302).

Feedback is welcome!

