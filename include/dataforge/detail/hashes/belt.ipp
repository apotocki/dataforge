/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#include "../ciphers/belt_encrypt.hpp"

namespace dataforge::belt_detail {

inline void belt_encrypt(const uint_least64_t* key, const uint_least64_t* inBlock, uint_least64_t* outBlock)
{
    if constexpr (
        sizeof(uint_least64_t) * CHAR_BIT == 64 &&
        sizeof(uint_least32_t) * CHAR_BIT == 32 &&
        std::endian::native == std::endian::little
    ) {
        belt_encrypt(reinterpret_cast<const uint_least32_t*>(key),
            reinterpret_cast<const uint_least32_t*>(inBlock),
            reinterpret_cast<uint_least32_t*>(outBlock));
    } else {
        uint_least32_t k32[8];
        le_copy<64, 32>(key, 4, k32);

        uint_least32_t in32[8];
        le_copy<64, 32>(inBlock, 4, in32);

        uint_least32_t out32[4];
        belt_encrypt(k32, in32, out32);

        outBlock[0] = (uint_least64_t(out32[1]) << 32) + out32[0];
        outBlock[1] = (uint_least64_t(out32[3]) << 32) + out32[2];
    }
}

inline belt_hash_impl::belt_hash_impl()
{
    reset();
}

template <typename InitQuarkT>
belt_hash_impl::belt_hash_impl(InitQuarkT const& dt)
    : belt_hash_impl { }
{}

inline void belt_hash_impl::reset() noexcept
{
    std::fill(s.begin(), s.end(), 0);
    h[0] = 0x3bf5080ac8ba94b1ull;
    h[1] = 0xe45d4a588e006d36ull;
    h[2] = 0xacc7b61b9dfa0485ull;
    h[3] = 0x0dcefd02c2722e25ull;
}

inline void belt_hash_impl::sigma2(const uint_least64_t* x, uint_least64_t* result)
{
    uint_least64_t teta[4];
    uint_least64_t h0 = h[0];
    uint_least64_t h1 = h[1];

    uint_least64_t u3u4[2] = { h[0] ^ h[2], h[1] ^ h[3] };

    belt_encrypt(x, u3u4, teta);

    teta[0] ^= u3u4[0];
    teta[1] ^= u3u4[1];

    // teta1 = sigma1(u) || u4
    teta[2] = h[2];
    teta[3] = h[3];

    // F_{teta1}(u1) xor u1
    belt_encrypt(teta, x, result);

    result[0] ^= x[0];
    result[1] ^= x[1];

    // (sigma1(u) xor 0xff..ff) || u3
    // invert first part of teta1
    teta[0] = ~teta[0];
    teta[1] = ~teta[1];

    // if result == h at this moment original h[0] and h[1] are lost
    teta[2] = h0;
    teta[3] = h1;

    belt_encrypt(teta, x + 2, result + 2);

    result[2] ^= x[2];
    result[3] ^= x[3];
}

inline void belt_hash_impl::process_block(const void* m)
{
    uint_least64_t ms[4];
    const uint_least64_t* pms = le_to_T<8, 64, const uint_least64_t>(ms, m, 32);

    uint_least64_t u3u4[2] = { h[0] ^ h[2], h[1] ^ h[3] };
    
    uint_least64_t tmp[2];
    belt_encrypt(pms, u3u4, tmp);
    s[0] ^= tmp[0] ^ u3u4[0];
    s[1] ^= tmp[1] ^ u3u4[1];

    // update h: h <- sigma2(x_i || h)
    sigma2(pms, h.data());
}

inline void belt_hash_impl::finalize()
{
	unsigned int bytes_in_buf = this->bytes_in_buf();
    if (bytes_in_buf) {
	    if (bytes_in_buf != 32) {
		    memset(buffer_ + bytes_in_buf, 0, 32 - bytes_in_buf);
        }
	    process_block(buffer_);
    }
    uint_least64_t fbuff[4];
    std::copy(bit_count.data(), bit_count.data() + 2, fbuff);
    fbuff[2] = s[0];
    fbuff[3] = s[1];
    sigma2(fbuff, h.data());
}

}
