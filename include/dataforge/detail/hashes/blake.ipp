/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#include <algorithm>

namespace dataforge::blake_detail {

inline const int sigma[10][16] = {
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
    {14, 10, 4, 8, 9, 15, 13, 6, 1, 12, 0, 2, 11, 7, 5, 3},
    {11, 8, 12, 0, 5, 2, 15, 13, 10, 14, 3, 6, 7, 1, 9, 4},
    {7, 9, 3, 1, 13, 12, 11, 14, 2, 6, 5, 10, 4, 0, 15, 8},
    {9, 0, 5, 7, 2, 4, 10, 15, 14, 1, 11, 12, 6, 8, 3, 13},
    {2, 12, 6, 10, 0, 11, 8, 3, 4, 13, 7, 5, 15, 14, 1, 9},
    {12, 5, 1, 15, 14, 13, 4, 10, 0, 7, 6, 3, 9, 2, 8, 11},
    {13, 11, 7, 14, 12, 1, 3, 9, 5, 0, 15, 4, 8, 6, 2, 10},
    {6, 15, 14, 9, 11, 3, 0, 8, 12, 2, 13, 7, 1, 4, 10, 5},
    {10, 2, 8, 4, 7, 6, 1, 5, 15, 11, 9, 14, 3, 12, 13, 0}
};

inline const uint_least32_t blake_def_base<32>::c[16] = {
    UINT32_C(0x243F6A88), UINT32_C(0x85A308D3),
    UINT32_C(0x13198A2E), UINT32_C(0x03707344),
    UINT32_C(0xA4093822), UINT32_C(0x299F31D0),
    UINT32_C(0x082EFA98), UINT32_C(0xEC4E6C89),
    UINT32_C(0x452821E6), UINT32_C(0x38D01377),
    UINT32_C(0xBE5466CF), UINT32_C(0x34E90C6C),
    UINT32_C(0xC0AC29B7), UINT32_C(0xC97C50DD),
    UINT32_C(0x3F84D5B5), UINT32_C(0xB5470917)
};

inline const uint_least64_t blake_def_base<64>::c[16] = {
    UINT64_C(0x243F6A8885A308D3), UINT64_C(0x13198A2E03707344),
    UINT64_C(0xA4093822299F31D0), UINT64_C(0x082EFA98EC4E6C89),
    UINT64_C(0x452821E638D01377), UINT64_C(0xBE5466CF34E90C6C),
    UINT64_C(0xC0AC29B7C97C50DD), UINT64_C(0x3F84D5B5B5470917),
    UINT64_C(0x9216D5D98979FB1B), UINT64_C(0xD1310BA698DFB5AC),
    UINT64_C(0x2FFD72DBD01ADFB7), UINT64_C(0xB8E1AFED6A267E96),
    UINT64_C(0xBA7C9045F12C7F99), UINT64_C(0x24A19947B3916CF7),
    UINT64_C(0x0801F2E2858EFC16), UINT64_C(0x636920D871574E69)
};

inline const uint_least32_t blake_impl_iv_base<224>::iv[8] = {
    UINT32_C(0xC1059ED8), UINT32_C(0x367CD507),
    UINT32_C(0x3070DD17), UINT32_C(0xF70E5939),
    UINT32_C(0xFFC00B31), UINT32_C(0x68581511),
    UINT32_C(0x64F98FA7), UINT32_C(0xBEFA4FA4)
};

inline const uint_least32_t blake_impl_iv_base<256>::iv[8] = {
    UINT32_C(0x6A09E667), UINT32_C(0xBB67AE85),
    UINT32_C(0x3C6EF372), UINT32_C(0xA54FF53A),
    UINT32_C(0x510E527F), UINT32_C(0x9B05688C),
    UINT32_C(0x1F83D9AB), UINT32_C(0x5BE0CD19)
};

inline const uint_least64_t blake_impl_iv_base<384>::iv[8] = {
    UINT64_C(0xCBBB9D5DC1059ED8), UINT64_C(0x629A292A367CD507),
    UINT64_C(0x9159015A3070DD17), UINT64_C(0x152FECD8F70E5939),
    UINT64_C(0x67332667FFC00B31), UINT64_C(0x8EB44A8768581511),
    UINT64_C(0xDB0C2E0D64F98FA7), UINT64_C(0x47B5481DBEFA4FA4)
};

inline const uint_least64_t blake_impl_iv_base<512>::iv[8] = {
    UINT64_C(0x6A09E667F3BCC908), UINT64_C(0xBB67AE8584CAA73B),
    UINT64_C(0x3C6EF372FE94F82B), UINT64_C(0xA54FF53A5F1D36F1),
    UINT64_C(0x510E527FADE682D1), UINT64_C(0x9B05688C2B3E6C1F),
    UINT64_C(0x1F83D9ABFB41BD6B), UINT64_C(0x5BE0CD19137E2179)
};

template <blake_type Type>
blake_impl<blake_type, Type>::blake_impl()
{
    std::memset(s, 0, 4 * sizeof(word_type));
    std::memcpy(h, blake_impl::iv, 8 * sizeof(word_type));
}

template <blake_type Type>
template <typename InitQuarkT>
blake_impl<blake_type, Type>::blake_impl(InitQuarkT const& dt)
{
    std::copy(dt.salt.begin(), dt.salt.end(), s);
    std::memcpy(h, blake_impl::iv, 8 * sizeof(word_type));
}

template <blake_type Type>
void blake_impl<blake_type, Type>::reset()
{
    this->bit_count = 0;
    std::memcpy(h, blake_impl::iv, 8 * sizeof(word_type));
}

template <blake_type Type>
void blake_impl<blake_type, Type>::G(word_type* m, word_type* v, int round, int a, int b, int c, int d, int i)
{
    v[a] = v[a] + v[b] + (m[sigma[round][2 * i]] ^ blake_impl::c[sigma[round][2 * i + 1]]);
    v[d] = right_rotate<blake_impl::word_size, word_type>(v[d] ^ v[a], blake_impl::word_size / 2); // 16/32
    v[c] = v[c] + v[d];
    v[b] = right_rotate<blake_impl::word_size, word_type>(v[b] ^ v[c], blake_impl::word_size == 32 ? 12 : 25); // 12/25
    v[a] = v[a] + v[b] + (m[sigma[round][2 * i + 1]] ^ blake_impl::c[sigma[round][2 * i]]);
    v[d] = right_rotate<blake_impl::word_size, word_type>(v[d] ^ v[a], blake_impl::word_size / 4); // 8/16
    v[c] = v[c] + v[d];
    v[b] = right_rotate<blake_impl::word_size, word_type>(v[b] ^ v[c], blake_impl::word_size == 32 ? 7 : 11);  // 7/11
}

template <blake_type Type>
void blake_impl<blake_type, Type>::process_block(const void* msg)
{
    word_type ms[16];
    word_type* m = be_to_T<8, blake_impl::word_size>(ms, msg, blake_impl::block_size);

    word_type v[16];
    std::memcpy(v, h, 8 * sizeof(word_type));
    std::memcpy(v + 8, blake_impl::c, 8 * sizeof(word_type));
    v[8] ^= s[0]; v[9] ^= s[1]; v[10] ^= s[2]; v[11] ^= s[3];
    v[12] ^= blake_impl::bit_count[0]; v[13] ^= blake_impl::bit_count[0]; v[14] ^= blake_impl::bit_count[1]; v[15] ^= blake_impl::bit_count[1];

    for (int round = 0; round < (blake_impl::word_size == 32 ? 14 : 16); ++round) {
        int r10 = round % 10;
        /* column step */
        G(m, v, r10, 0, 4, 8, 12, 0);
        G(m, v, r10, 1, 5, 9, 13, 1);
        G(m, v, r10, 2, 6, 10, 14, 2);
        G(m, v, r10, 3, 7, 11, 15, 3);

        /* diagonal step */
        G(m, v, r10, 0, 5, 10, 15, 4);
        G(m, v, r10, 1, 6, 11, 12, 5);
        G(m, v, r10, 2, 7, 8, 13, 6);
        G(m, v, r10, 3, 4, 9, 14, 7);
    }

    /* finalization */
    for (int i = 0; i < 8; ++i) {
        h[i] ^= v[i] ^ v[i + 8] ^ s[i & 3];
    }
}

template <blake_type Type>
void blake_impl<blake_type, Type>::store_bit_count(void*)
{
    blake_impl::buffer_[blake_impl::block_size - blake_impl::input_length_size - 1] |= blake_impl::special_padding_byte;
    this->bit_count.store_as_big_endian(blake_impl::buffer_ + blake_impl::block_size - blake_impl::input_length_size, 2);
    const unsigned int bytes_in_buf = this->bytes_in_buf();
    if (!bytes_in_buf || blake_impl::block_size - bytes_in_buf < blake_impl::input_length_size + 1) {
        this->bit_count = 0;
    }
}

template <blake2_type Type>
blake_impl<blake2_type, Type>::blake_impl()
{
    reset();
}

template <blake2_type Type>
template <typename InitQuarkT>
blake_impl<blake2_type, Type>::blake_impl(InitQuarkT const& dt)
{
    key_sz = static_cast<uint8_t>((std::min)(sizeof(key), dt.key.size()));
    if (key_sz) {
        std::memcpy(key, dt.key.data(), key_sz);
        std::memset(key + key_sz, 0, sizeof(key) - key_sz);
    }
    reset();
}

template <blake2_type Type>
void blake_impl<blake2_type, Type>::reset()
{
    last_node = 0;
    std::memcpy(h, blake_impl::iv, 8 * sizeof(word_type));
    std::fill(f, f + 2, 0);
    h[0] ^= blake_impl::magic_xor_value;
    h[0] ^= blake_impl::digest_length();
    h[0] ^= (static_cast<word_type>(key_sz) << 8);
    if (key_sz) {
        std::memcpy(blake_impl::buffer_, key, blake_impl::block_size);
        this->bit_count = blake_impl::block_size;
        full_buff = 1;
    } else {
        this->bit_count = 0;
        full_buff = 0;
    }
}

template <blake2_type Type>
void blake_impl<blake2_type, Type>::G(const word_type* m, word_type* v, int round, int a, int b, int c, int d, int i)
{
    v[a] = v[a] + v[b] + m[sigma[round][2 * i]];
    v[d] = right_rotate<blake_impl::word_size, word_type>(v[d] ^ v[a], blake_impl::word_size / 2); // 16/32
    v[c] = v[c] + v[d];
    v[b] = right_rotate<blake_impl::word_size, word_type>(v[b] ^ v[c], blake_impl::word_size / 8 * 3); // 12/24
    v[a] = v[a] + v[b] + m[sigma[round][2 * i + 1]];
    v[d] = right_rotate<blake_impl::word_size, word_type>(v[d] ^ v[a], blake_impl::word_size / 4); // 8/16
    v[c] = v[c] + v[d];
    v[b] = right_rotate<blake_impl::word_size, word_type>(v[b] ^ v[c], blake_impl::word_size == 32 ? 7 : 63);  // 7/11
}

template <blake2_type Type>
void blake_impl<blake2_type, Type>::process_block(const void* msg)
{
    word_type ms[16];
    const word_type* m = le_to_T<8, word_size, const word_type>(ms, msg, blake_impl::block_size);

    word_type v[16];
    std::memcpy(v, h, 8 * sizeof(word_type));
    std::memcpy(v + 8, blake_impl::iv, 8 * sizeof(word_type));
    v[12] ^= blake_impl::bit_count[0]; v[13] ^= blake_impl::bit_count[1];
    v[14] ^= f[0]; v[15] ^= f[1];

    for (int round = 0; round < (blake_impl::word_size == 32 ? 10 : 12); ++round) {
        int r10 = round % 10;
        /* column step */
        G(m, v, r10, 0, 4, 8, 12, 0);
        G(m, v, r10, 1, 5, 9, 13, 1);
        G(m, v, r10, 2, 6, 10, 14, 2);
        G(m, v, r10, 3, 7, 11, 15, 3);

        /* diagonal step */
        G(m, v, r10, 0, 5, 10, 15, 4);
        G(m, v, r10, 1, 6, 11, 12, 5);
        G(m, v, r10, 2, 7, 8, 13, 6);
        G(m, v, r10, 3, 4, 9, 14, 7);
    }

    /* finalization */
    for (int i = 0; i < 8; ++i) {
        h[i] ^= v[i] ^ v[i + 8];
    }
}

template <blake2_type Type>
void blake_impl<blake2_type, Type>::finalize()
{
    set_lastblock();
    const unsigned int bytes_in_buf = blake_impl::bytes_in_buf();
    std::memset(blake_impl::buffer_ + bytes_in_buf, 0, blake_impl::block_size - bytes_in_buf);
    this->process_block(blake_impl::buffer_);
}

}
