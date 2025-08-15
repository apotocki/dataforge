/*=============================================================================
    Copyright (c) 2022 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#include <cstring>

namespace dataforge::md2_detail {

inline const uint_least8_t PI_SUBST[256] = {
  41, 46, 67, 201, 162, 216, 124, 1, 61, 54, 84, 161, 236, 240, 6,
  19, 98, 167, 5, 243, 192, 199, 115, 140, 152, 147, 43, 217, 188,
  76, 130, 202, 30, 155, 87, 60, 253, 212, 224, 22, 103, 66, 111, 24,
  138, 23, 229, 18, 190, 78, 196, 214, 218, 158, 222, 73, 160, 251,
  245, 142, 187, 47, 238, 122, 169, 104, 121, 145, 21, 178, 7, 63,
  148, 194, 16, 137, 11, 34, 95, 33, 128, 127, 93, 154, 90, 144, 50,
  39, 53, 62, 204, 231, 191, 247, 151, 3, 255, 25, 48, 179, 72, 165,
  181, 209, 215, 94, 146, 42, 172, 86, 170, 198, 79, 184, 56, 210,
  150, 164, 125, 182, 118, 252, 107, 226, 156, 116, 4, 241, 69, 157,
  112, 89, 100, 113, 135, 32, 134, 91, 207, 101, 230, 45, 168, 2, 27,
  96, 37, 173, 174, 176, 185, 246, 28, 70, 97, 105, 52, 64, 126, 15,
  85, 71, 163, 35, 221, 81, 175, 58, 195, 92, 249, 206, 186, 197,
  234, 38, 44, 83, 13, 110, 133, 40, 132, 9, 211, 223, 205, 244, 65,
  129, 77, 82, 106, 220, 55, 200, 108, 193, 171, 250, 36, 225, 123,
  8, 12, 189, 177, 74, 120, 136, 149, 139, 227, 99, 232, 109, 233,
  203, 213, 254, 59, 0, 29, 57, 242, 239, 183, 14, 102, 88, 208, 228,
  166, 119, 114, 248, 235, 117, 75, 10, 49, 68, 80, 180, 143, 237,
  31, 26, 219, 153, 141, 51, 159, 17, 131, 20
};

inline md2_impl::md2_impl()
{
    std::memset(state, 0, sizeof(state));
    std::memset(checksum, 0, sizeof(checksum));
}

inline void md2_impl::reset()
{
    std::memset(state, 0, sizeof(state));
    std::memset(checksum, 0, sizeof(checksum));
    bit_count = 0;
}

// processes one chunk of 16 bytes 
inline void md2_impl::process_block(const void* msg)
{
    uint_least8_t const* block = reinterpret_cast<uint_least8_t const*>(msg);
    unsigned char x[48];
    std::memcpy(x, state, 16);
    std::memcpy(x + 16, block, 16);
    for (int i = 0; i < 16; ++i) {
        x[i + 32] = state[i] ^ block[i];
    }

    // Encrypt block (18 rounds).
    int t = 0;
    for (int i = 0; i < 18; ++i) {
        for (int j = 0; j < 48; ++j)
            t = x[j] ^= PI_SUBST[t];
        t = (t + i) & 0xff;
    }

    // Save new state
    std::memcpy(state, x, 16);

    // Update checksum
    t = checksum[15];
    for (int i = 0; i < 16; ++i)
        t = checksum[i] ^= PI_SUBST[block[i] ^ t];
}

inline void md2_impl::finalize()
{
    uint_least8_t padding[16];
    const unsigned int bytes_in_buf = this->bytes_in_buf();
    unsigned int pval = 16 - bytes_in_buf;
    std::memcpy(padding, buffer_, bytes_in_buf);
    std::memset(padding + bytes_in_buf, pval, pval);
    process_block(padding);
    process_block(checksum);
}

}
