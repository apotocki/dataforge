/*=============================================================================
    Copyright (c) 2026 Alexander Pototskiy

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
    using element_type = std::byte;

    static constexpr std::byte padding_byte{ 0x80u };
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

    inline void process_block(const void* msg, size_t block_count) noexcept
    {
        const auto* data = reinterpret_cast<const uint8_t*>(msg);
        for (;;) {
            static_cast<DerivedT&>(*this).process_block(data);
            if (--block_count == 0) break;
            data += block_size;
        }
    }

    counter<size_type, BCV> bit_count;

    element_type buffer_[block_size];
};

template <typename DerivedT, size_t BlockSizeV, typename SizeT, size_t BCV>
void digest_base<DerivedT, BlockSizeV, SizeT, BCV>::input(const void* vdata, size_t len)
{
    if (!len) return;

    const std::byte* data = static_cast<const std::byte*>(vdata);

    size_t bytes_in_buf = static_cast<DerivedT&>(*this).bytes_in_buf();

    if (bytes_in_buf) {
        const size_t bytes_to_copy =
            len > block_size - bytes_in_buf ? block_size - bytes_in_buf : len;
        std::memcpy(buffer_ + bytes_in_buf, data, bytes_to_copy);
        assert (bytes_to_copy + bytes_in_buf <= block_size);
        if (bytes_to_copy + bytes_in_buf == block_size && (!DerivedT::allow_full_buffer || bytes_to_copy < len)) {
            static_cast<DerivedT&>(*this).process_block(buffer_, 1);
            len-= bytes_to_copy;
            data += bytes_to_copy;
        } else {
            static_cast<DerivedT&>(*this).count_bytes(bytes_to_copy);
            return;
        }
    }
    
    // now process the data in blocks
    //  is the number of whole blocks in the remaining data, but if allow_full_buffer is true, we can process one more block if it fits in the buffer, so we need to check for that as well
    size_t block_count = len / block_size;
    size_t bytes_to_process = block_count * block_size;
    if constexpr (DerivedT::allow_full_buffer) {
        if (block_count && (len - bytes_to_process == 0)) {
            --block_count;
            bytes_to_process -= block_size;
        }
    }

    if (block_count) {
        //static_cast<DerivedT&>(*this).count_bytes(bytes_to_process);
        static_cast<DerivedT&>(*this).process_block(data, block_count);
        data += bytes_to_process;
        len -= bytes_to_process;
    }

    // copy remaining bytes into buffer
    std::memcpy(buffer_, data, len);
    static_cast<DerivedT&>(*this).count_bytes(data + len - static_cast<const std::byte*>(vdata));
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
        static_cast<DerivedT&>(*this).process_block(buffer_, 1);
        std::memset(buffer_, 0, DerivedT::block_size);
    }

    static_cast<DerivedT&>(*this).store_bit_count(buffer_ + pad_end);

    static_cast<DerivedT&>(*this).process_block(buffer_, 1);
}

}
