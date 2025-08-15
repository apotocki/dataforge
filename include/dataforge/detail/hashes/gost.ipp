/*=============================================================================
    Copyright (c) 2022 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#include <algorithm>
#include <mutex>

namespace dataforge::gost_detail {

inline uint_least8_t default_sbox[8][16] = {
	{  4, 10,  9,  2, 13,  8,  0, 14,  6, 11,  1, 12,  7, 15,  5,  3 },
	{ 14, 11,  4, 12,  6, 13, 15, 10,  2,  3,  8,  1,  0,  7,  5,  9 },
	{  5,  8,  1, 13, 10,  3,  4,  2, 14, 15, 12,  7,  6,  0,  9, 11 },
	{  7, 13, 10,  1,  0,  8,  9, 15, 14,  4,  6, 12, 11,  2,  5,  3 },
	{  6, 12,  7,  1,  5, 15, 13,  8,  4, 10,  9, 14,  0,  3, 11,  2 },
	{  4, 11, 10,  0,  7,  2,  1, 13,  3,  6,  8,  5,  9, 12, 15, 14 },
	{ 13, 11,  4,  1,  3, 15,  5,  9,  0, 10, 14,  7,  6,  8,  2, 12 },
	{  1, 15, 13,  0,  5,  7, 10,  4,  9,  2,  3, 14,  6, 11,  8, 12 }
};

inline uint_least8_t cpro_sbox[8][16] = {
    { 10,  4,  5,  6,  8,  1,  3,  7, 13, 12, 14,  0,  9,  2, 11, 15 },
    {  5, 15,  4,  0,  2, 13, 11,  9,  1,  7,  6,  3, 12, 14, 10,  8 },
    {  7, 15, 12, 14,  9,  4,  1,  0,  3, 11,  5,  2,  6, 10,  8, 13 },
    {  4, 10,  7, 12,  0, 15,  2,  8, 14,  1,  6,  5, 13, 11,  9,  3 },
    {  7,  6,  4, 11,  9, 12,  2, 10,  1,  8,  0, 14, 15, 13,  3,  5 },
    {  7,  6,  2,  4, 13,  9, 15,  0, 10,  1,  5, 11,  8, 14, 12,  3 },
    { 13, 14,  4,  1,  7,  0,  5, 10,  3, 12,  8, 15,  6,  2,  9, 11 },
    {  1,  3, 10,  9,  5, 11,  4, 15,  8,  6,  7, 14, 13,  0,  2, 12 }
};

inline std::array<uint_least32_t, 256> default_sbox_1;
inline std::array<uint_least32_t, 256> default_sbox_2;
inline std::array<uint_least32_t, 256> default_sbox_3;
inline std::array<uint_least32_t, 256> default_sbox_4;

inline std::array<uint_least32_t, 256> cpro_sbox_1;
inline std::array<uint_least32_t, 256> cpro_sbox_2;
inline std::array<uint_least32_t, 256> cpro_sbox_3;
inline std::array<uint_least32_t, 256> cpro_sbox_4;

inline std::once_flag default_flag{}, crypto_pro_flag{};

inline gost_impl::gost_impl(bool crypto_pro_sbox)
{
    if (crypto_pro_sbox) {
        sbox_1 = cpro_sbox_1.data();
        sbox_2 = cpro_sbox_2.data();
        sbox_3 = cpro_sbox_3.data();
        sbox_4 = cpro_sbox_4.data();
        std::call_once(crypto_pro_flag, [this]() { init_sbox(cpro_sbox); });
    } else {
        sbox_1 = default_sbox_1.data();
        sbox_2 = default_sbox_2.data();
        sbox_3 = default_sbox_3.data();
        sbox_4 = default_sbox_4.data();
        std::call_once(default_flag, [this]() { init_sbox(default_sbox); });
    }
    reset();
}

inline void gost_impl::init_sbox(uint_least8_t(&sbox)[8][16])
{
    uint_least32_t ax, bx, cx, dx;
    int i = 0;
    for (int a = 0; a < 16; ++a)
    {
        ax = uint_least32_t(sbox[1][a]) << 15;
        bx = uint_least32_t(sbox[3][a]) << 23;
        cx = sbox[5][a];
        cx = (cx >> 1) | (cx << 31);
        dx = uint_least32_t(sbox[7][a]) << 7;

        for (int b = 0; b < 16; ++b)
        {
            sbox_1[i] = ax | (uint_least32_t(sbox[0][b]) << 11);
            sbox_2[i] = bx | (uint_least32_t(sbox[2][b]) << 19);
            sbox_3[i] = cx | (uint_least32_t(sbox[4][b]) << 27);
            sbox_4[i++] = dx | (uint_least32_t(sbox[6][b]) << 3);
        }
    }
}

template <typename InitQuarkT>
gost_impl::gost_impl(InitQuarkT const& dt)
    : gost_impl { dt.crypto_pro_sbox }
{}

inline void gost_impl::reset() noexcept
{
    std::fill(sum.begin(), sum.end(), 0);
	std::fill(hash.begin(), hash.end(), 0);
    l = r = 0;
}

inline void gost_impl::gRound(uint_least32_t k1, uint_least32_t k2)
{
    uint_least32_t t = k1 + r;
    l ^= sbox_1[t & 0xff] ^ sbox_2[(t >> 8) & 0xff] ^ sbox_3[(t >> 16) & 0xff] ^ sbox_4[t >> 24];
    t = k2 + l;
    r ^= sbox_1[t & 0xff] ^ sbox_2[(t >> 8) & 0xff] ^ sbox_3[(t >> 16) & 0xff] ^ sbox_4[t >> 24];
}

inline void gost_impl::gH(uint_least32_t const* m)
{
    uint_least32_t key[8];
    uint_least32_t u[8];
    uint_least32_t v[8];
    uint_least32_t w[8];
    uint_least32_t s[8];

    memcpy(u, hash.data(), sizeof(u));
    memcpy(v, m, sizeof(u));

    for (int i = 0; i < 8; i += 2)
    {
        w[0] = u[0] ^ v[0];	       /* w = u xor v */
        w[1] = u[1] ^ v[1];
        w[2] = u[2] ^ v[2];
        w[3] = u[3] ^ v[3];
        w[4] = u[4] ^ v[4];
        w[5] = u[5] ^ v[5];
        w[6] = u[6] ^ v[6];
        w[7] = u[7] ^ v[7];

        /* P-Transformation */

        key[0] = (w[0] & 0x000000ff) | ((w[2] & 0x000000ff) << 8) |
            ((w[4] & 0x000000ff) << 16) | ((w[6] & 0x000000ff) << 24);
        key[1] = ((w[0] & 0x0000ff00) >> 8) | (w[2] & 0x0000ff00) |
            ((w[4] & 0x0000ff00) << 8) | ((w[6] & 0x0000ff00) << 16);
        key[2] = ((w[0] & 0x00ff0000) >> 16) | ((w[2] & 0x00ff0000) >> 8) |
            (w[4] & 0x00ff0000) | ((w[6] & 0x00ff0000) << 8);
        key[3] = ((w[0] & 0xff000000) >> 24) | ((w[2] & 0xff000000) >> 16) |
            ((w[4] & 0xff000000) >> 8) | (w[6] & 0xff000000);
        key[4] = (w[1] & 0x000000ff) | ((w[3] & 0x000000ff) << 8) |
            ((w[5] & 0x000000ff) << 16) | ((w[7] & 0x000000ff) << 24);
        key[5] = ((w[1] & 0x0000ff00) >> 8) | (w[3] & 0x0000ff00) |
            ((w[5] & 0x0000ff00) << 8) | ((w[7] & 0x0000ff00) << 16);
        key[6] = ((w[1] & 0x00ff0000) >> 16) | ((w[3] & 0x00ff0000) >> 8) |
            (w[5] & 0x00ff0000) | ((w[7] & 0x00ff0000) << 8);
        key[7] = ((w[1] & 0xff000000) >> 24) | ((w[3] & 0xff000000) >> 16) |
            ((w[5] & 0xff000000) >> 8) | (w[7] & 0xff000000);

        r = hash[i];			       /* encriphering transformation */
        l = hash[i + 1];

        for (int j = 0; j < 24; j += 2) {
            gRound(key[j & 7], key[(j & 7) + 1]);
        }
        for (int j = 7; j > 0; j -= 2) {
            gRound(key[j], key[j - 1]);
        }
        
        s[i] = l;
        s[i + 1] = r;

        if (i == 6)
            break;

        l = u[0] ^ u[2];		       /* U = A(U) */
        r = u[1] ^ u[3];
        u[0] = u[2];
        u[1] = u[3];
        u[2] = u[4];
        u[3] = u[5];
        u[4] = u[6];
        u[5] = u[7];
        u[6] = l;
        u[7] = r;

        if (i == 2)		       /* Constant C_3 */
        {
            u[0] ^= 0xff00ff00;
            u[1] ^= 0xff00ff00;
            u[2] ^= 0x00ff00ff;
            u[3] ^= 0x00ff00ff;
            u[4] ^= 0x00ffff00;
            u[5] ^= 0xff0000ff;
            u[6] ^= 0x000000ff;
            u[7] ^= 0xff00ffff;
        }

        l = v[0];			       /* V = A(A(V)) */
        r = v[2];
        v[0] = v[4];
        v[2] = v[6];
        v[4] = l ^ r;
        v[6] = v[0] ^ r;
        l = v[1];
        r = v[3];
        v[1] = v[5];
        v[3] = v[7];
        v[5] = l ^ r;
        v[7] = v[1] ^ r;
    }

    /* 12 rounds of the LFSR (computed from a product matrix) and xor in M */

    u[0] = m[0] ^ s[6];
    u[1] = m[1] ^ s[7];
    u[2] = m[2] ^ (s[0] << 16) ^ (s[0] >> 16) ^ (s[0] & 0xffff) ^
        (s[1] & 0xffff) ^ (s[1] >> 16) ^ (s[2] << 16) ^ s[6] ^ (s[6] << 16) ^
        (s[7] & 0xffff0000) ^ (s[7] >> 16);
    u[3] = m[3] ^ (s[0] & 0xffff) ^ (s[0] << 16) ^ (s[1] & 0xffff) ^
        (s[1] << 16) ^ (s[1] >> 16) ^ (s[2] << 16) ^ (s[2] >> 16) ^
        (s[3] << 16) ^ s[6] ^ (s[6] << 16) ^ (s[6] >> 16) ^ (s[7] & 0xffff) ^
        (s[7] << 16) ^ (s[7] >> 16);
    u[4] = m[4] ^
        (s[0] & 0xffff0000) ^ (s[0] << 16) ^ (s[0] >> 16) ^
        (s[1] & 0xffff0000) ^ (s[1] >> 16) ^ (s[2] << 16) ^ (s[2] >> 16) ^
        (s[3] << 16) ^ (s[3] >> 16) ^ (s[4] << 16) ^ (s[6] << 16) ^
        (s[6] >> 16) ^ (s[7] & 0xffff) ^ (s[7] << 16) ^ (s[7] >> 16);
    u[5] = m[5] ^ (s[0] << 16) ^ (s[0] >> 16) ^ (s[0] & 0xffff0000) ^
        (s[1] & 0xffff) ^ s[2] ^ (s[2] >> 16) ^ (s[3] << 16) ^ (s[3] >> 16) ^
        (s[4] << 16) ^ (s[4] >> 16) ^ (s[5] << 16) ^ (s[6] << 16) ^
        (s[6] >> 16) ^ (s[7] & 0xffff0000) ^ (s[7] << 16) ^ (s[7] >> 16);
    u[6] = m[6] ^ s[0] ^ (s[1] >> 16) ^ (s[2] << 16) ^ s[3] ^ (s[3] >> 16) ^
        (s[4] << 16) ^ (s[4] >> 16) ^ (s[5] << 16) ^ (s[5] >> 16) ^ s[6] ^
        (s[6] << 16) ^ (s[6] >> 16) ^ (s[7] << 16);
    u[7] = m[7] ^ (s[0] & 0xffff0000) ^ (s[0] << 16) ^ (s[1] & 0xffff) ^
        (s[1] << 16) ^ (s[2] >> 16) ^ (s[3] << 16) ^ s[4] ^ (s[4] >> 16) ^
        (s[5] << 16) ^ (s[5] >> 16) ^ (s[6] >> 16) ^ (s[7] & 0xffff) ^
        (s[7] << 16) ^ (s[7] >> 16);

    /* 16 * 1 round of the LFSR and xor in H */

    v[0] = hash[0] ^ (u[1] << 16) ^ (u[0] >> 16);
    v[1] = hash[1] ^ (u[2] << 16) ^ (u[1] >> 16);
    v[2] = hash[2] ^ (u[3] << 16) ^ (u[2] >> 16);
    v[3] = hash[3] ^ (u[4] << 16) ^ (u[3] >> 16);
    v[4] = hash[4] ^ (u[5] << 16) ^ (u[4] >> 16);
    v[5] = hash[5] ^ (u[6] << 16) ^ (u[5] >> 16);
    v[6] = hash[6] ^ (u[7] << 16) ^ (u[6] >> 16);
    v[7] = hash[7] ^ (u[0] & 0xffff0000) ^ (u[0] << 16) ^ (u[7] >> 16) ^
        (u[1] & 0xffff0000) ^ (u[1] << 16) ^ (u[6] << 16) ^ (u[7] & 0xffff0000);

    /* 61 rounds of LFSR, mixing up h (computed from a product matrix) */

    hash[0] = (v[0] & 0xffff0000) ^ (v[0] << 16) ^ (v[0] >> 16) ^ (v[1] >> 16) ^
        (v[1] & 0xffff0000) ^ (v[2] << 16) ^ (v[3] >> 16) ^ (v[4] << 16) ^
        (v[5] >> 16) ^ v[5] ^ (v[6] >> 16) ^ (v[7] << 16) ^ (v[7] >> 16) ^
        (v[7] & 0xffff);
    hash[1] = (v[0] << 16) ^ (v[0] >> 16) ^ (v[0] & 0xffff0000) ^ (v[1] & 0xffff) ^
        v[2] ^ (v[2] >> 16) ^ (v[3] << 16) ^ (v[4] >> 16) ^ (v[5] << 16) ^
        (v[6] << 16) ^ v[6] ^ (v[7] & 0xffff0000) ^ (v[7] >> 16);
    hash[2] = (v[0] & 0xffff) ^ (v[0] << 16) ^ (v[1] << 16) ^ (v[1] >> 16) ^
        (v[1] & 0xffff0000) ^ (v[2] << 16) ^ (v[3] >> 16) ^ v[3] ^ (v[4] << 16) ^
        (v[5] >> 16) ^ v[6] ^ (v[6] >> 16) ^ (v[7] & 0xffff) ^ (v[7] << 16) ^
        (v[7] >> 16);
    hash[3] = (v[0] << 16) ^ (v[0] >> 16) ^ (v[0] & 0xffff0000) ^
        (v[1] & 0xffff0000) ^ (v[1] >> 16) ^ (v[2] << 16) ^ (v[2] >> 16) ^ v[2] ^
        (v[3] << 16) ^ (v[4] >> 16) ^ v[4] ^ (v[5] << 16) ^ (v[6] << 16) ^
        (v[7] & 0xffff) ^ (v[7] >> 16);
    hash[4] = (v[0] >> 16) ^ (v[1] << 16) ^ v[1] ^ (v[2] >> 16) ^ v[2] ^
        (v[3] << 16) ^ (v[3] >> 16) ^ v[3] ^ (v[4] << 16) ^ (v[5] >> 16) ^
        v[5] ^ (v[6] << 16) ^ (v[6] >> 16) ^ (v[7] << 16);
    hash[5] = (v[0] << 16) ^ (v[0] & 0xffff0000) ^ (v[1] << 16) ^ (v[1] >> 16) ^
        (v[1] & 0xffff0000) ^ (v[2] << 16) ^ v[2] ^ (v[3] >> 16) ^ v[3] ^
        (v[4] << 16) ^ (v[4] >> 16) ^ v[4] ^ (v[5] << 16) ^ (v[6] << 16) ^
        (v[6] >> 16) ^ v[6] ^ (v[7] << 16) ^ (v[7] >> 16) ^ (v[7] & 0xffff0000);
    hash[6] = v[0] ^ v[2] ^ (v[2] >> 16) ^ v[3] ^ (v[3] << 16) ^ v[4] ^
        (v[4] >> 16) ^ (v[5] << 16) ^ (v[5] >> 16) ^ v[5] ^ (v[6] << 16) ^
        (v[6] >> 16) ^ v[6] ^ (v[7] << 16) ^ v[7];
    hash[7] = v[0] ^ (v[0] >> 16) ^ (v[1] << 16) ^ (v[1] >> 16) ^ (v[2] << 16) ^
        (v[3] >> 16) ^ v[3] ^ (v[4] << 16) ^ v[4] ^ (v[5] >> 16) ^ v[5] ^
        (v[6] << 16) ^ (v[6] >> 16) ^ (v[7] << 16) ^ v[7];
}

inline void gost_impl::process_block(const void* msg)
{
    uint_least32_t mps[8];
    process_block(le_to_T<8, 32, const uint_least32_t>(mps, msg, 32));
}

inline void gost_impl::process_block(const uint_least32_t* mps)
{
    gH(mps);
    bool carry = false, overflow = false;
    for (int i = 0; i < 8; i++)
    {
        uint_least32_t t = sum[i] + mps[i];
        overflow = t < sum[i] || t < mps[i];
        if (carry) {
            sum[i] = t + 1;
            carry = overflow || !sum[i];
        } else {
            sum[i] = t;
            carry = overflow;
        }
    }
}

inline void gost_impl::finalize()
{
	unsigned int bytes_in_buf = this->bytes_in_buf();
    if (bytes_in_buf) {
	    if (bytes_in_buf != 32) {
		    memset(buffer_ + bytes_in_buf, 0, 32 - bytes_in_buf);
        }
	    process_block(buffer_);
    }
    gH(bit_count.data());
    gH(sum.data());
}

}
