/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#include "../utility/data_ops.hpp"

// https://datatracker.ietf.org/doc/rfc8891/

namespace dataforge {
    
template <typename DerivedT, std::endian Endianness>
template <typename QrkT>
magma_cipher<DerivedT, Endianness>::magma_cipher(QrkT const& q)
	: sbox{ q.sbox }
{

}

template <typename DerivedT, std::endian Endianness>
void magma_cipher<DerivedT, Endianness>::expand_key(std::span<const unsigned char> key)
{
	auto* pkey = key_begin();
	std::fill(pkey, pkey + 8, 0);
	size_t ksz = (std::min)(key.size(), (size_t)32);
	if constexpr (cipher_endianness() == std::endian::big) {
		be_copy<8, 32>(key.data(), ksz, pkey);
	} else {
		le_copy<8, 32>(key.data(), ksz, pkey);
	}
}

template <typename DerivedT, std::endian Endianness>
void magma_cipher<DerivedT, Endianness>::encrypt_block(const word_type* in, word_type* out) noexcept
{
	word_type a = in[1], b = in[0];

	word_type const* pk = key_begin();
	for (int i = 0; i < 24; ++i) {
		word_type tmp = pk[i & 7] + a;
		tmp = t(tmp);
		tmp = left_rotate<word_size>(tmp, 11);
		b ^= tmp;
		std::swap(a, b);
	}
	for (int i = 24; i < 32; ++i) {
		word_type tmp = pk[7 - (i & 7)] + a;
		tmp = t(tmp);
		tmp = left_rotate<word_size>(tmp, 11);
		b ^= tmp;
		std::swap(a, b);
	}
	out[0] = a;
	out[1] = b;
}

template <typename DerivedT, std::endian Endianness>
void magma_cipher<DerivedT, Endianness>::decrypt_block(const word_type* in, word_type* out) noexcept
{
	word_type a = in[1], b = in[0];

	word_type const* pk = key_begin();
	for (int i = 0; i < 8; ++i) {
		word_type tmp = pk[i & 7] + a;
		tmp = t(tmp);
		tmp = left_rotate<word_size>(tmp, 11);
		b ^= tmp;
		std::swap(a, b);
	}
	for (int i = 8; i < 32; ++i) {
		word_type tmp = pk[7 - i & 7] + a;
		tmp = t(tmp);
		tmp = left_rotate<word_size>(tmp, 11);
		b ^= tmp;
		std::swap(a, b);
	}
	out[0] = a;
	out[1] = b;
}

template <typename DerivedT, std::endian Endianness>
inline uint_least32_t magma_cipher<DerivedT, Endianness>::t(uint_least32_t val)
{
	uint_least32_t tmp = sbox[0][val & 0xf];
	for (int i = 1; i < 8; ++i) {
		val >>= 4;
		uint_least32_t bv = sbox[i][val & 0xf];
		bv <<= 4 * i;
		tmp |= bv;
	}
	return tmp;
}

}
