/*=============================================================================
    Copyright (c) 2022 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include <cstdint>
#include <cassert>

#include "counter.hpp"

namespace dataforge {

template <typename DerivedT, size_t BlockSizeV, typename SizeT = uint_least64_t, size_t BCV = 2>
struct digest_base
{
    static constexpr size_t block_size = BlockSizeV;

    using size_type = SizeT;
    using element_type = uint_least8_t;

    static constexpr uint8_t padding_byte = 0x80u;
    static constexpr bool allow_full_buffer = false;

    void finalize();

    unsigned int bytes_in_buf() const noexcept
    {
        return bit_count[0] / 8 % BlockSizeV;
    }

    unsigned int bits_in_buf() const noexcept
    {
        return bit_count[0] % BlockSizeV;
    }

    void store_bit_count(void* dst) const noexcept
    {
        bit_count.store_as_little_endian(dst, 1);
    }

    void add_to_bit_count(size_type x) noexcept
    {
        bit_count.add(x);
    }

    void count_bytes(size_t x)
    {
        if (size_t bcnt = x * 8; (bcnt >> 3) == x) {
            bit_count.add(bcnt);
        } else if (size_t bcnt = x * 4; (bcnt >> 2) == x) {
            bit_count.add(bcnt);
            bit_count.add(bcnt);
        } else if (size_t bcnt = x * 2; (bcnt >> 2) == x) {
            bit_count.add(bcnt);
            bit_count.add(bcnt);
            bit_count.add(bcnt);
            bit_count.add(bcnt);
        } else {
            for (int i = 0; i < 8; ++i) {
                bit_count.add(x);
            }
        }
    }

    void input(const void* vdata, size_t len);

    counter<size_type, BCV> bit_count;

    element_type buffer_[block_size];
};

template <typename DerivedT, size_t BlockSizeV, typename SizeT, size_t BCV>
void digest_base<DerivedT, BlockSizeV, SizeT, BCV>::input(const void* vdata, size_t len)
{
    if (!len) return;

    const uint_least8_t* data = static_cast<const uint_least8_t*>(vdata);

    size_t index = 0; // index into data array
    size_t bytes_in_buf = static_cast<DerivedT&>(*this).bytes_in_buf();

    if (bytes_in_buf) {
        const size_t bytes_to_copy =
            len > block_size - bytes_in_buf ? block_size - bytes_in_buf : len;
        std::memcpy(buffer_ + bytes_in_buf, data, bytes_to_copy);
        assert (bytes_to_copy + bytes_in_buf <= block_size);
        static_cast<DerivedT&>(*this).count_bytes(bytes_to_copy);
        if (bytes_to_copy + bytes_in_buf == block_size && (!DerivedT::allow_full_buffer || bytes_to_copy < len)) {
            static_cast<DerivedT&>(*this).process_block(buffer_);
            index = bytes_to_copy;
        } else {
            return;
        }
    }
    
    // now process the data in blocks
    for (; len - index >= block_size + (DerivedT::allow_full_buffer?1:0); index += block_size) {
        static_cast<DerivedT&>(*this).count_bytes(block_size);
        static_cast<DerivedT&>(*this).process_block(data + index);
    }

    // copy remaining bytes into buffer
    const size_t remaining_bytes = len - index;
    std::memcpy(buffer_, data + index, remaining_bytes);
    static_cast<DerivedT&>(*this).count_bytes(remaining_bytes);
}

template <typename DerivedT, size_t BlockSizeV, typename SizeT, size_t BCV>
void digest_base<DerivedT, BlockSizeV, SizeT, BCV>::finalize()
{
    const unsigned int bytes_in_buf = static_cast<DerivedT&>(*this).bytes_in_buf();
    buffer_[bytes_in_buf] = static_cast<DerivedT&>(*this).padding_byte;
    const unsigned int pad_end = block_size - DerivedT::input_length_size;
    std::memset(buffer_ + bytes_in_buf + 1, 0, DerivedT::block_size - bytes_in_buf - 1);
    if (bytes_in_buf >= pad_end)
    {
        static_cast<DerivedT&>(*this).process_block(buffer_);
        std::memset(buffer_, 0, DerivedT::block_size);
    }

    static_cast<DerivedT&>(*this).store_bit_count(buffer_ + pad_end);

    static_cast<DerivedT&>(*this).process_block(buffer_);
}

}
