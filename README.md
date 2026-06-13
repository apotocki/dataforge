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

## Hardware Acceleration Flags

The **SHA-1** and **SHA-2** families support compile-time selectable
hardware-accelerated block processing. All variants always have a portable scalar
fallback, so acceleration never changes results — only throughput.

Configuration is controlled by these macros:

- `DATAFORGE_ACCEL_AUTODETECT` (default `1`)
  - `1`: pick the fastest backend supported by the **running CPU** (detected via
    CPUID / `getauxval`) the first time a block is processed.
  - `0`: disable auto-detection (use scalar unless a backend is forced).
- `DATAFORGE_ACCEL_FORCE` (default `-1`)
  - `-1`: use the auto-detection result.
  - `0`: force the scalar implementation.
  - `1`: force the x86 backend (no run-time probing — the target CPU must support it).
  - `2`: force the ARM SHA backend.
- `DATAFORGE_ACCEL_X86_SHA256_USE_AVX512` (default `0`)
  - `0`: SHA-224/SHA-256 use **SHA-NI** when available (AVX-512 is **not** used).
  - `1`: opt in to the x86 **AVX-512** schedule for SHA-224/SHA-256 instead of
    SHA-NI. SHA-NI is faster for a single block, so this is off by default and
    only useful for specific workloads/parts.

### Backends

#### SHA-1

- **x86 SHA-NI** — Intel SHA Extensions (`_mm_sha1rnds4` / `_mm_sha1nexte` /
  `_mm_sha1msg1` / `_mm_sha1msg2`). Fastest single-block path for SHA-1; requires
  an x86 target.
- **ARM SHA1** — AArch64 crypto extensions (`vsha1cq` / `vsha1pq` / `vsha1mq` /
  `vsha1su0q` / `vsha1su1q`). Requires an AArch64 target. Runtime detection via
  `HWCAP_SHA1` bit from `getauxval(AT_HWCAP)` on Linux.

#### SHA-2

- **x86 SHA-NI** — Intel SHA Extensions (`_mm_sha256rnds2` / `msg1` / `msg2`).
  Fastest single-block path for SHA-224/SHA-256; requires an x86 target.
- **x86 AVX-512** — vectorized message schedule (AVX-512F + AVX-512VL, using the
  `vprord` / `vprorq` vector rotate) with 8-way unrolled scalar compression rounds.
  - For **SHA-224/SHA-256** this is **opt-in** (`DATAFORGE_ACCEL_X86_SHA256_USE_AVX512=1`).
  - For **SHA-384 / SHA-512 / SHA-512-224 / SHA-512-256** there is no SHA-NI
    equivalent, so the AVX-512 schedule is the default hardware path and is
    preferred over scalar whenever the CPU supports AVX-512.
  - Requires an x86 target.
- **x86 SSE4.1** — same two-wide XMM message schedule as the AVX-512 path, but
  64-bit lane rotations are decomposed into shift-or pairs instead of using the
  AVX-512VL `vprorq` instruction. Compression rounds are also 8-way unrolled.
  Selected automatically for the **SHA-384 / SHA-512 / SHA-512-224 / SHA-512-256**
  family on x86-64 CPUs that lack AVX-512 (e.g. Intel Raptor Lake / Alder Lake
  desktop with both P- and E-cores active). Requires only SSE4.1, present on
  virtually all x86-64 hardware since ~2008.
- **ARM SHA2** — AArch64 crypto extensions (`vsha256hq` / `vsha256su*`), for
  SHA-224/SHA-256. Requires an AArch64 target.

On GCC/Clang the intrinsics for each backend are enabled per-function via
`__attribute__((target(...)))`, so no global `-msha` / `-mavx512*` /
`-march=armv8-a+sha2` flags are needed to *build* them; on MSVC they are always
available. The architecture (x86 vs ARM) is the only compile-time requirement.
AVX-512 selection also verifies, at run time, that the OS has enabled the AVX-512
register state (via `XCR0`), not just that the CPUID feature bits are present.

### How a backend is chosen

- **Auto-detect (`AUTODETECT=1`, `FORCE=-1`)**
  - SHA-1 on x86: **SHA-NI** if present, else **scalar**.
  - SHA-1 on ARM: **SHA1** crypto extension if present (via `HWCAP_SHA1`), else **scalar**.
  - SHA-224/SHA-256 on x86: **SHA-NI** if present, else **scalar** (or, when
    `DATAFORGE_ACCEL_X86_SHA256_USE_AVX512=1`: **AVX-512** if present, then SHA-NI,
    then scalar).
  - SHA-512 family on x86: **AVX-512** if present, else **SSE4.1** if present, else **scalar**.
  - SHA-224/SHA-256 on ARM: **SHA2** if present, else scalar.
- **Forced x86 (`FORCE=1`)** — the intrinsic code is emitted **directly** with no
  run-time CPU probing; the caller guarantees the CPU supports it. SHA-1 and
  SHA-224/SHA-256 emit SHA-NI (or AVX-512 for SHA-256 if
  `DATAFORGE_ACCEL_X86_SHA256_USE_AVX512=1` **and** the build targets AVX-512).
  The SHA-512 family emits AVX-512 when the build targets it (`-mavx512f -mavx512vl`
  or `/arch:AVX512`, i.e. `__AVX512F__ && __AVX512VL__`), otherwise **SSE4.1** if
  the build defines `__SSE4_1__` (e.g. `-msse4.1`), else scalar.
  A forced-x86 build on a non-x86 target safely degrades to scalar.
- **Forced ARM (`FORCE=2`)** — SHA-1 and SHA-224/SHA-256 use the AArch64 crypto
  extension intrinsics directly with no runtime probing. A forced-ARM build on a
  non-ARM target safely degrades to scalar.
- **Forced scalar / acceleration disabled** — the portable scalar implementation
  is always used.

### CMake configuration examples

```bash
cmake -S . -B build -DDATAFORGE_ACCEL_AUTODETECT=ON  -DDATAFORGE_ACCEL_FORCE=-1   # auto (default)
cmake -S . -B build -DDATAFORGE_ACCEL_AUTODETECT=OFF -DDATAFORGE_ACCEL_FORCE=1    # force x86 (SHA-NI)
cmake -S . -B build -DDATAFORGE_ACCEL_AUTODETECT=OFF -DDATAFORGE_ACCEL_FORCE=2    # force ARM (SHA-1 + SHA2 crypto ext)
cmake -S . -B build -DDATAFORGE_ACCEL_AUTODETECT=OFF -DDATAFORGE_ACCEL_FORCE=0    # force scalar
```

Opt in to AVX-512 for SHA-224/SHA-256 (otherwise SHA-NI is used):

```bash
cmake -S . -B build -DDATAFORGE_ACCEL_X86_SHA256_USE_AVX512=ON
```

The SHA-512 family uses AVX-512 automatically under auto-detect. To force the
SHA-512 AVX-512 path with no run-time probing, force the x86 backend and build
with AVX-512 enabled, e.g. (GCC/Clang):

```bash
cmake -S . -B build -DDATAFORGE_ACCEL_AUTODETECT=OFF -DDATAFORGE_ACCEL_FORCE=1 \
      -DCMAKE_CXX_FLAGS="-mavx512f -mavx512vl"
```

### Direct compiler defines (without CMake options)

```bash
-DDATAFORGE_ACCEL_AUTODETECT=1 -DDATAFORGE_ACCEL_FORCE=-1
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

