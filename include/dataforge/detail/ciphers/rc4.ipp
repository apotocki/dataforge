/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include <new>
#include <algorithm>

namespace dataforge::rc4_detail {

inline rc4_impl* rc4_impl::create(std::span<const unsigned char> key, uint_least8_t n, size_t skip_bits)
{
	size_t buffsz = ((size_t)1) << n;
	size_t totalsz = sizeof(rc4_impl) + sizeof(uint_least16_t) * buffsz;
	void* place = new char[totalsz];
	rc4_impl* result = new(place) rc4_impl(key, n, skip_bits);
	return result;
}

inline void rc4_impl::destroy()
{
	this->~rc4_impl();
	delete[] reinterpret_cast<char*>(this);
}

inline rc4_impl::rc4_impl(std::span<const unsigned char> key, uint_least8_t n, size_t skip_bits) noexcept
	: key_{ key }, n_{ (uint_least8_t)n }, s_sz_{ ((uint_least32_t)1) << n }, skip_bits_ { skip_bits }
{
	reset();
}

inline void rc4_impl::reset() noexcept
{
	size_t skip_bits = skip_bits_;
	size_t mask = s_sz_ - 1;
	uint_least16_t* sblock = sblock_begin();
	size_t j = 0;
	for (size_t i = 0; i < s_sz_; ++i) {
		sblock[i] = (uint16_t)i;
	}
	for (size_t i = 0; i < s_sz_; ++i) {
		j = (uint16_t)((j + sblock[i] + key_[i % key_.size()]) & mask);
		std::swap(sblock[i], sblock[j]);
	}
	i_ = j_ = 0;
	buf = 0;
	bits_in_buf = 0;

	while (skip_bits) {
		i_ = (i_ + 1) & mask;
		j_ = (j_ + sblock[i_]) & mask;
		std::swap(sblock[i_], sblock[j_]);
		uint_least16_t t = (sblock[i_] + sblock[j_]) & mask;
		buf |= ((uint_least32_t)sblock[t]) << bits_in_buf;
		bits_in_buf += n_;
		size_t bc = (std::min)((size_t)bits_in_buf, skip_bits);
		skip_bits -= bc;
		bits_in_buf -= (uint8_t)bc;
		buf >>= bc;
	}
}

}
