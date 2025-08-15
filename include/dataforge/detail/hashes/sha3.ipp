/*=============================================================================
    Copyright (c) 2022 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#include <algorithm>

namespace dataforge::sha3_detail {

inline const uint_least64_t keccak_ctx_defs<24>::xor_masks[24] = {
    0x0000000000000001ULL, 0x0000000000008082ULL, 0x800000000000808aULL,
    0x8000000080008000ULL, 0x000000000000808bULL, 0x0000000080000001ULL,
    0x8000000080008081ULL, 0x8000000000008009ULL, 0x000000000000008aULL,
    0x0000000000000088ULL, 0x0000000080008009ULL, 0x000000008000000aULL,
    0x000000008000808bULL, 0x800000000000008bULL, 0x8000000000008089ULL,
    0x8000000000008003ULL, 0x8000000000008002ULL, 0x8000000000000080ULL,
    0x000000000000800aULL, 0x800000008000000aULL, 0x8000000080008081ULL,
    0x8000000000008080ULL, 0x0000000080000001ULL, 0x8000000080008008ULL
};

template <size_t Rounds>
keccak_ctx<Rounds>::keccak_ctx(size_t bitsize, size_t d, uint8_t pad)
    : bits_{ bitsize }
    , d_{ d }
    , block_size_{ 200 - 2 * (bitsize / 8) }
    , pad_{ pad }
{
    reset();
}

template <size_t Rounds>
void keccak_ctx<Rounds>::reset()
{
    std::fill(hash_, hash_ + state_size, 0);
    num_bytes_ = 0;
    buffer_size_ = 0;
}

inline unsigned int mod5(unsigned int x) noexcept
{
    if (x < 5)
        return x;
    return x - 5;
}

template <size_t Rounds>
void keccak_ctx<Rounds>::process_block(const void* data)
{
    const uint_least64_t* data64 = reinterpret_cast<const uint_least64_t*>(data);
    // mix data into state
    for (unsigned int i = 0; i < block_size_ / 8; i++) {
        if constexpr (std::endian::native == std::endian::little && sizeof(uint_least8_t) * 8 == sizeof(uint_least64_t)) {
            hash_[i] ^= data64[i];
        } else if constexpr (std::endian::native == std::endian::big && sizeof(uint_least8_t) * 8 == sizeof(uint_least64_t)) {
            uint_least64_t swapped = reverse_bytes(data64[i]);
            hash_[i] ^= swapped;
        } else {
            static_assert(dependent_false<std::integral_constant<size_t, Rounds>>, "unsupported endianness");
        }
    }
    // re-compute state
    for (unsigned int round = 0; round < rounds; round++)
    {
        // Theta
        uint_least64_t coefficients[5];
        for (unsigned int i = 0; i < 5; i++)
            coefficients[i] = hash_[i] ^ hash_[i + 5] ^ hash_[i + 10] ^ hash_[i + 15] ^ hash_[i + 20];

        for (unsigned int i = 0; i < 5; i++)
        {
            uint_least64_t one = coefficients[mod5(i + 4)] ^ left_rotate<64>(coefficients[mod5(i + 1)], 1);
            hash_[i] ^= one;
            hash_[i + 5] ^= one;
            hash_[i + 10] ^= one;
            hash_[i + 15] ^= one;
            hash_[i + 20] ^= one;
        }

        // temporary
        uint_least64_t one;

        // Rho Pi
        uint_least64_t last = hash_[1];
        one = hash_[10]; hash_[10] = left_rotate<64>(last, 1); last = one;
        one = hash_[7]; hash_[7] = left_rotate<64>(last, 3); last = one;
        one = hash_[11]; hash_[11] = left_rotate<64>(last, 6); last = one;
        one = hash_[17]; hash_[17] = left_rotate<64>(last, 10); last = one;
        one = hash_[18]; hash_[18] = left_rotate<64>(last, 15); last = one;
        one = hash_[3]; hash_[3] = left_rotate<64>(last, 21); last = one;
        one = hash_[5]; hash_[5] = left_rotate<64>(last, 28); last = one;
        one = hash_[16]; hash_[16] = left_rotate<64>(last, 36); last = one;
        one = hash_[8]; hash_[8] = left_rotate<64>(last, 45); last = one;
        one = hash_[21]; hash_[21] = left_rotate<64>(last, 55); last = one;
        one = hash_[24]; hash_[24] = left_rotate<64>(last, 2); last = one;
        one = hash_[4]; hash_[4] = left_rotate<64>(last, 14); last = one;
        one = hash_[15]; hash_[15] = left_rotate<64>(last, 27); last = one;
        one = hash_[23]; hash_[23] = left_rotate<64>(last, 41); last = one;
        one = hash_[19]; hash_[19] = left_rotate<64>(last, 56); last = one;
        one = hash_[13]; hash_[13] = left_rotate<64>(last, 8); last = one;
        one = hash_[12]; hash_[12] = left_rotate<64>(last, 25); last = one;
        one = hash_[2]; hash_[2] = left_rotate<64>(last, 43); last = one;
        one = hash_[20]; hash_[20] = left_rotate<64>(last, 62); last = one;
        one = hash_[14]; hash_[14] = left_rotate<64>(last, 18); last = one;
        one = hash_[22]; hash_[22] = left_rotate<64>(last, 39); last = one;
        one = hash_[9]; hash_[9] = left_rotate<64>(last, 61); last = one;
        one = hash_[6]; hash_[6] = left_rotate<64>(last, 20); last = one;
        hash_[1] = left_rotate<64>(last, 44);

        // Chi
        for (unsigned int j = 0; j < state_size; j += 5)
        {
            // temporaries
            uint_least64_t one = hash_[j];
            uint_least64_t two = hash_[j + 1];

            hash_[j] ^= hash_[j + 2] & ~two;
            hash_[j + 1] ^= hash_[j + 3] & ~hash_[j + 2];
            hash_[j + 2] ^= hash_[j + 4] & ~hash_[j + 3];
            hash_[j + 3] ^= one & ~hash_[j + 4];
            hash_[j + 4] ^= two & ~one;
        }

        // Iota
        hash_[0] ^= xor_masks[round];
    }
}

template <size_t Rounds>
void keccak_ctx<Rounds>::input(const void* vdata, size_t len)
{
    const uint_least8_t* current = reinterpret_cast<const uint_least8_t*>(vdata);

    // copy data to buffer
    if (buffer_size_ > 0)
    {
        while (len > 0 && buffer_size_ < block_size_)
        {
            buffer_[buffer_size_++] = *current++;
            --len;
        }
    }

    // full buffer
    if (buffer_size_ == block_size_)
    {
        process_block(buffer_);
        num_bytes_ += block_size_;
        buffer_size_ = 0;
    }

    // no more data ?
    if (!len) return;

    // process full blocks
    while (len >= block_size_)
    {
        process_block(current);
        current += block_size_;
        num_bytes_ += block_size_;
        len -= block_size_;
    }

    // keep remaining bytes in buffer
    while (len > 0)
    {
        buffer_[buffer_size_++] = *current++;
        --len;
    }
}

template <size_t Rounds>
void keccak_ctx<Rounds>::finalize()
{
    // process remaining bytes
    // add padding
    size_t offset = buffer_size_;
    // add a "1" byte
    buffer_[offset++] = pad_;
    // fill with zeros
    while (offset < block_size_)
        buffer_[offset++] = 0;

    // and add a single set bit
    buffer_[offset - 1] |= 0x80;

    process_block(buffer_);
}

}
