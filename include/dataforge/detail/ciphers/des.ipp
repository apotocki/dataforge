/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#include "../utility/data_ops.hpp"

#define L64_MASK  0x00000000ffffffffull

namespace dataforge::des_detail {

inline const unsigned char IP[64] =
{
    58, 50, 42, 34, 26, 18, 10, 2,
    60, 52, 44, 36, 28, 20, 12, 4,
    62, 54, 46, 38, 30, 22, 14, 6,
    64, 56, 48, 40, 32, 24, 16, 8,
    57, 49, 41, 33, 25, 17,  9, 1,
    59, 51, 43, 35, 27, 19, 11, 3,
    61, 53, 45, 37, 29, 21, 13, 5,
    63, 55, 47, 39, 31, 23, 15, 7
};

inline const unsigned char FP[64] =
{
    40, 8, 48, 16, 56, 24, 64, 32,
    39, 7, 47, 15, 55, 23, 63, 31,
    38, 6, 46, 14, 54, 22, 62, 30,
    37, 5, 45, 13, 53, 21, 61, 29,
    36, 4, 44, 12, 52, 20, 60, 28,
    35, 3, 43, 11, 51, 19, 59, 27,
    34, 2, 42, 10, 50, 18, 58, 26,
    33, 1, 41,  9, 49, 17, 57, 25
};

inline const unsigned char PC1[56] =
{
    57, 49, 41, 33, 25, 17,  9,
     1, 58, 50, 42, 34, 26, 18,
    10,  2, 59, 51, 43, 35, 27,
    19, 11,  3, 60, 52, 44, 36,

    63, 55, 47, 39, 31, 23, 15,
     7, 62, 54, 46, 38, 30, 22,
    14,  6, 61, 53, 45, 37, 29,
    21, 13,  5, 28, 20, 12,  4
};

inline const unsigned char PC2[48] =
{
    14, 17, 11, 24,  1,  5,
     3, 28, 15,  6, 21, 10,
    23, 19, 12,  4, 26,  8,
    16,  7, 27, 20, 13,  2,
    41, 52, 31, 37, 47, 55,
    30, 40, 51, 45, 33, 48,
    44, 49, 39, 56, 34, 53,
    46, 42, 50, 36, 29, 32
};

inline const unsigned char ITERATION_SHIFT[16] =
{
    //  1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16
        1,  1,  2,  2,  2,  2,  2,  2,  1,  2,  2,  2,  2,  2,  2,  1
};

inline const unsigned char EXPANSION[48] =
{
    32,  1,  2,  3,  4,  5,
     4,  5,  6,  7,  8,  9,
     8,  9, 10, 11, 12, 13,
    12, 13, 14, 15, 16, 17,
    16, 17, 18, 19, 20, 21,
    20, 21, 22, 23, 24, 25,
    24, 25, 26, 27, 28, 29,
    28, 29, 30, 31, 32,  1
};

inline const unsigned char SBOX[8][64] =
{
    {
        // S1
        14,  4, 13,  1,  2, 15, 11,  8,  3, 10,  6, 12,  5,  9,  0,  7,
         0, 15,  7,  4, 14,  2, 13,  1, 10,  6, 12, 11,  9,  5,  3,  8,
         4,  1, 14,  8, 13,  6,  2, 11, 15, 12,  9,  7,  3, 10,  5,  0,
        15, 12,  8,  2,  4,  9,  1,  7,  5, 11,  3, 14, 10,  0,  6, 13
    },
    {
        // S2
        15,  1,  8, 14,  6, 11,  3,  4,  9,  7,  2, 13, 12,  0,  5, 10,
         3, 13,  4,  7, 15,  2,  8, 14, 12,  0,  1, 10,  6,  9, 11,  5,
         0, 14,  7, 11, 10,  4, 13,  1,  5,  8, 12,  6,  9,  3,  2, 15,
        13,  8, 10,  1,  3, 15,  4,  2, 11,  6,  7, 12,  0,  5, 14,  9
    },
    {
        // S3
        10,  0,  9, 14,  6,  3, 15,  5,  1, 13, 12,  7, 11,  4,  2,  8,
        13,  7,  0,  9,  3,  4,  6, 10,  2,  8,  5, 14, 12, 11, 15,  1,
        13,  6,  4,  9,  8, 15,  3,  0, 11,  1,  2, 12,  5, 10, 14,  7,
         1, 10, 13,  0,  6,  9,  8,  7,  4, 15, 14,  3, 11,  5,  2, 12
    },
    {
        // S4
         7, 13, 14,  3,  0,  6,  9, 10,  1,  2,  8,  5, 11, 12,  4, 15,
        13,  8, 11,  5,  6, 15,  0,  3,  4,  7,  2, 12,  1, 10, 14,  9,
        10,  6,  9,  0, 12, 11,  7, 13, 15,  1,  3, 14,  5,  2,  8,  4,
         3, 15,  0,  6, 10,  1, 13,  8,  9,  4,  5, 11, 12,  7,  2, 14
    },
    {
        // S5
         2, 12,  4,  1,  7, 10, 11,  6,  8,  5,  3, 15, 13,  0, 14,  9,
        14, 11,  2, 12,  4,  7, 13,  1,  5,  0, 15, 10,  3,  9,  8,  6,
         4,  2,  1, 11, 10, 13,  7,  8, 15,  9, 12,  5,  6,  3,  0, 14,
        11,  8, 12,  7,  1, 14,  2, 13,  6, 15,  0,  9, 10,  4,  5,  3
    },
    {
        // S6
        12,  1, 10, 15,  9,  2,  6,  8,  0, 13,  3,  4, 14,  7,  5, 11,
        10, 15,  4,  2,  7, 12,  9,  5,  6,  1, 13, 14,  0, 11,  3,  8,
         9, 14, 15,  5,  2,  8, 12,  3,  7,  0,  4, 10,  1, 13, 11,  6,
         4,  3,  2, 12,  9,  5, 15, 10, 11, 14,  1,  7,  6,  0,  8, 13
    },
    {
        // S7
         4, 11,  2, 14, 15,  0,  8, 13,  3, 12,  9,  7,  5, 10,  6,  1,
        13,  0, 11,  7,  4,  9,  1, 10, 14,  3,  5, 12,  2, 15,  8,  6,
         1,  4, 11, 13, 12,  3,  7, 14, 10, 15,  6,  8,  0,  5,  9,  2,
         6, 11, 13,  8,  1,  4, 10,  7,  9,  5,  0, 15, 14,  2,  3, 12
    },
    {
        // S8
        13,  2,  8,  4,  6, 15, 11,  1, 10,  9,  3, 14,  5,  0, 12,  7,
         1, 15, 13,  8, 10,  3,  7,  4, 12,  5,  6, 11,  0, 14,  9,  2,
         7, 11,  4,  1,  9, 12, 14,  2,  0,  6, 10, 13, 15,  3,  5,  8,
         2,  1, 14,  7,  4, 10,  8, 13, 15, 12,  9,  0,  3,  5,  6, 11
    }
};

inline const unsigned char PBOX[32] =
{
    16,  7, 20, 21,
    29, 12, 28, 17,
     1, 15, 23, 26,
     5, 18, 31, 10,
     2,  8, 24, 14,
    32, 27,  3,  9,
    19, 13, 30,  6,
    22, 11,  4, 25
};

inline uint_least32_t f(uint_least32_t R, uint_least64_t k) noexcept
{
    using namespace des_detail;
    // applying expansion permutation and returning 48-bit data
    uint_least64_t s_input = 0;
    for (int i = 0; i < 48; i++)
    {
        s_input <<= 1;
        s_input |= (uint_least64_t)((R >> (32 - EXPANSION[i])) & 1);
    }

    // XORing expanded Ri with Ki, the round key
    s_input = s_input ^ k;

    // applying S-Boxes function and returning 32-bit data
    uint_least32_t s_output = 0;
    for (int i = 0; i < 8; ++i)
    {
        // Outer bits
        char row = (char)((s_input & (0x0000840000000000ull >> 6 * i)) >> (42 - 6 * i));
        row = (row >> 4) | (row & 0x01);

        // Middle 4 bits of input
        char column = (char)((s_input & (0x0000780000000000ull >> 6 * i)) >> (43 - 6 * i));

        s_output <<= 4;
        s_output |= (uint_least32_t)(SBOX[i][16 * row + column] & 0x0f);
    }

    // applying the round permutation
    uint_least32_t f_result = 0;
    for (int i = 0; i < 32; i++)
    {
        f_result <<= 1;
        f_result |= (s_output >> (32 - PBOX[i])) & 1;
    }

    return f_result;
}

}

namespace dataforge {

template <typename DerivedT>
template <typename QrkT>
des_cipher<DerivedT>::des_cipher(QrkT const& q)
    : ed_mode_{ static_cast<uint_least8_t>(q.ed_mode ? 1 : 0) }
    , levels_{ (uint_least8_t)q.levels }
{}

template <typename DerivedT>
void des_cipher<DerivedT>::expand_key(std::span<const unsigned char> key)
{
    assert(key.size() == levels_ * 8);

    for (level_ = 0; level_ < levels_; ++level_) {
        do_expand_key(key.subspan(8 * level_, 8));
    }
}

template <typename DerivedT>
void des_cipher<DerivedT>::do_expand_key(std::span<const unsigned char> key)
{
    using namespace des_detail;

    word_type permuted_choice_1 = 0; // 56 bits
    for (int i = 0; i < 56; ++i)
    {
        permuted_choice_1 <<= 1;
        uint_least8_t shift = 64 - PC1[i];
        permuted_choice_1 |= (key[7 - (shift >> 3)] >> (shift & 7)) & 1;
    }

    // 28 bits
    uint_least32_t C = (uint_least32_t)((permuted_choice_1 >> 28) & 0x0fffffff);
    uint_least32_t D = (uint_least32_t)(permuted_choice_1 & 0x0fffffff);

    // Calculation of the 16 keys
    word_type* sk = sub_key();
    for (int i = 0; i < 16; ++i)
    {
        // key schedule, shifting Ci and Di
        for (uint_least8_t j = 0; j < ITERATION_SHIFT[i]; ++j)
        {
            C = (0x0fffffff & (C << 1)) | (0x00000001 & (C >> 27));
            D = (0x0fffffff & (D << 1)) | (0x00000001 & (D >> 27));
        }

        uint_least64_t permuted_choice_2 = (((uint_least64_t)C) << 28) | (uint_least64_t)D;

        sk[i] = 0; // 48 bits (2*24)
        for (int j = 0; j < 48; ++j)
        {
            sk[i] <<= 1;
            sk[i] |= (permuted_choice_2 >> (56 - PC2[j])) & 1;
        }
    }
}

template <typename DerivedT>
void des_cipher<DerivedT>::encrypt_block(const word_type* in, word_type* out) noexcept
{
    level_ = 0;
    process_block(in, out, false);
    for (++level_; level_ < (int16_t)levels_; ++level_) {
        process_block(out, out, ed_mode_ ? !!(level_ & 1) : false);
    }
}

template <typename DerivedT>
void des_cipher<DerivedT>::decrypt_block(const word_type* in, word_type* out) noexcept
{
    level_ = (int16_t)levels_ - 1;
    process_block(in, out, true);
    for (--level_; level_ >= 0; --level_) {
        process_block(out, out, ed_mode_ ? !(level_ & 1) : true);
    }
}

template <typename DerivedT>
void des_cipher<DerivedT>::process_block(const word_type* in, word_type* out, bool is_decryption) noexcept
{
    using namespace des_detail;
    // applying initial permutation ip(in)
    word_type block = 0;
    for (int i = 0; i < 64; ++i)
    {
        block <<= 1;
        int shift = 64 - IP[i];
        block |= ((*in) >> (56 - shift + 2 * (shift & 7))) & 1;
    }

    // dividing T' into two 32-bit parts
    uint_least32_t L = (uint_least32_t)(block >> 32) & L64_MASK;
    uint_least32_t R = (uint_least32_t)(block & L64_MASK);

    // 16 rounds
    word_type* sk = sub_key();
    for (int i = 0; i < 16; i++)
    {
        uint_least32_t F = f(R, sk[is_decryption ? (15 - i) : i]);
        uint_least32_t temp = R;
        R = L ^ F;
        L = temp;
    }

    // swapping the two parts
    block = (((uint_least64_t)R) << 32) | (uint_least64_t)L;
    // applying final permutation
    //fp(block, out);
    word_type result = 0;
    for (int i = 0; i < 64; ++i)
    {
        result <<= 1;
        result |= (block >> (64 - FP[i])) & 1;
    }
    *out = reverse_bytes<64>(result);
}

}
