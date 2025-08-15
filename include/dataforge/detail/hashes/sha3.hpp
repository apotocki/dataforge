/*=============================================================================
    Copyright (c) 2022 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include "../utility/digest_base.hpp"

namespace dataforge {

enum class sha3_type : int {
    sha3_224, sha3_256, sha3_384, sha3_512, shake_128, shake_256
};

namespace sha3_detail {

template <size_t Rounds> struct keccak_ctx_defs;

template <>
struct keccak_ctx_defs<24>
{
    static constexpr size_t state_size = 1600 / (8 * 8);
    static constexpr size_t max_block_size = 200; // - 2 * (min bit size / 8);
    static constexpr size_t rounds = 24;

    static const uint_least64_t xor_masks[rounds];
};

template <size_t Rounds>
struct keccak_ctx : keccak_ctx_defs<Rounds>
{
    using size_type = size_t;

    using defs_t = keccak_ctx_defs<Rounds>;
    using defs_t::state_size;
    using defs_t::max_block_size;
    using defs_t::rounds;
    using defs_t::xor_masks;

    static constexpr std::endian digest_endianness() { return std::endian::little; }
    inline size_t digest_length() const noexcept { return (d_ + 7) / 8; }
    using digest_word_type = uint_least64_t;
    static constexpr size_t digest_word_bit_count = 64;
    
    inline std::span<digest_word_type, state_size> digest_span() { return hash_; }

    keccak_ctx(size_t bitsize, size_t d, uint_least8_t pad);

    void input(const void* vdata, size_t len);
    void finalize();
    void reset();

protected:
    void process_block(const void* msg);
    void process_buffer();

protected:
    size_t bits_;
    size_t d_;
    size_t block_size_;

    uint_least64_t hash_[state_size];

    /// size of processed data in bytes
    uint_least64_t num_bytes_;

    /// valid bytes in m_buffer
    size_t buffer_size_;

    /// bytes not processed yet
    uint_least8_t buffer_[max_block_size];

    uint_least8_t pad_;
};

struct sha3_impl : keccak_ctx<24>
{
    sha3_impl(size_t bits, size_t d, uint_least8_t pad)
        : keccak_ctx{ bits , d, pad }
    {}

    template <typename InitQuarkT>
    explicit sha3_impl(InitQuarkT const& dt)
        : keccak_ctx{ dt.bits(), dt.dlength, dt.pad() }
    {}
};

}}

#include "sha3.ipp"
