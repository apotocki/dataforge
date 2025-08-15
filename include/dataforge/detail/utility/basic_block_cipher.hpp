/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include <memory>
#include <algorithm>
#include <bit>

#include "dataforge/ciphers/defs.hpp"

#include "concepts.hpp"

namespace dataforge {

template <typename AlgoT>
struct cipher_block_byte_span
{
    using type = std::span<const unsigned char>;
};

template <typename AlgoT>
requires (requires{ typename std::integral_constant<size_t, AlgoT::block_wsize()>; })
struct cipher_block_byte_span<AlgoT>
{
    static constexpr size_t size = AlgoT::block_wsize() * AlgoT::word_size / 8;
    using type = std::span<const unsigned char, size>;
};

template <class AlgoFactoryT, size_t InQueueSzMultiplierV = 1, typename AllocatorT = std::allocator<char>>
class basic_block_cipher 
    : public AlgoFactoryT::template apply<basic_block_cipher<AlgoFactoryT, InQueueSzMultiplierV, AllocatorT>>::type
    , AllocatorT
{
    using algo_t = typename AlgoFactoryT::template apply<basic_block_cipher>::type;
    using word_type = typename algo_t::word_type;

    static constexpr bool exact_word_size = sizeof(word_type) * CHAR_BIT == algo_t::word_size;

    template <typename QrkT>
    basic_block_cipher(QrkT const& q, AllocatorT alloc, size_t osz)
        : algo_t{q}, AllocatorT{alloc}, object_size{osz}
        , cipher_mode_{ (uint_least8_t)q.cmt }, pt{ q.pt }
        , bytes_in_buf { 0 }, oblock_ready { 0 }, finalization_stage { 0 }
        , reversed_ctr_flag { 0 }
    {
        algo_t::expand_key(q.key);

        std::fill(iv_begin(), iv_begin() + algo_t::block_wsize(), 0);
        std::fill(iblock_begin(), iblock_begin() + algo_t::block_wsize(), 0);
        size_t cnt = (std::min)(q.iv.size(), algo_t::block_wsize() * algo_t::word_size / 8);
        xe_copy<8, algo_t::word_size>(q.iv.data(), cnt, iv_begin());

        if constexpr (requires { typename QrkT::reversed_ctr_flag; }) {
            reversed_ctr_flag = 1;
            if (cipher_mode() == cipher_mode_type::CTR) {
                algo_t::encrypt_block(iv_begin(), iv_begin());
            }
        }

        std::copy(iv_begin(), iv_begin() + algo_t::block_wsize(), iv_backup_begin());
    }

public:
    template <typename QrkT>
    [[nodiscard]] static basic_block_cipher* create(QrkT const& q, AllocatorT alloc = AllocatorT{})
    {
        algo_t zombie{ q };
        size_t algo_sz;
        if constexpr (requires{ zombie.calculate_size(); }) {
            algo_sz = zombie.calculate_size();
        } else {
            algo_sz = sizeof(basic_block_cipher);
        }
        algo_sz = (algo_sz + sizeof(word_type) - 1) & ~(sizeof(word_type) - 1);
        size_t iv_sz = sizeof(word_type) * zombie.block_wsize();
        size_t inbuff_sz = InQueueSzMultiplierV * sizeof(word_type) * zombie.block_wsize();
        size_t outbuff_sz = sizeof(word_type) * zombie.block_wsize();
        size_t auxbuff_sz = zombie.block_wsize() * algo_t::word_size / 8;
        size_t total_sz = algo_sz + 2 * iv_sz + inbuff_sz + outbuff_sz + auxbuff_sz;
        if (!exact_word_size || zombie.cipher_endianness() != std::endian::native) {
            // we need additional out buffer to compose result bytes
            total_sz += zombie.block_wsize() * algo_t::word_size / 8;
        }
        auto* place = alloc.allocate(total_sz);
        try {
            return new(place) basic_block_cipher{q, std::move(alloc), total_sz };
        } catch (...) {
            alloc.deallocate(place, total_sz);
            throw;
        }
    }

    inline void destroy() noexcept
    {
        size_t osz = object_size;
        AllocatorT alloc = std::move(static_cast<AllocatorT&>(*this));
        this->~basic_block_cipher();
        alloc.deallocate(reinterpret_cast<char*>(this), osz);
    }

    constexpr size_t block_bsize() const { return algo_t::block_wsize() * algo_t::word_size / 8; }

    template <Integral<8> ET, typename BlockProcessorT>
    void push_data(std::span<ET> data, BlockProcessorT const& proc);

    // returns true if needs to refill out buffer
    template <Integral<8> ET, typename BlockProcessorT>
    bool pull_data(std::span<ET>& input, BlockProcessorT const& proc);

    void encrypt_block(const word_type* in);

    template <typename ErrorH, typename ConsumerT>
    void finalize_encryption(ErrorH const& errh, ConsumerT&& cons);
    
    template <typename ErrorH>
    std::span<const unsigned char> pull_finalized_encryption(ErrorH const& errh);
    
    void decrypt_block(const word_type* in, bool store_intermediate = true);

    template <typename ErrorH, typename ConsumerT>
    void finalize_decryption(ErrorH const& errh, ConsumerT&& cons);

    template <typename ErrorH>
    std::span<const unsigned char> pull_finalized_decryption(ErrorH const& errh);

    void reset()
    {
        if (cipher_mode() != cipher_mode_type::ECB) {
            std::copy(iv_backup_begin(), iv_backup_begin() + algo_t::block_wsize(), iv_begin());
        }
        if constexpr (requires { algo_t::reset(); }) {
            algo_t::reset();
        }
        bytes_in_buf = 0;
        oblock_ready = 0;
        finalization_stage = 0;
    }

    using oblock_span_t = typename cipher_block_byte_span<algo_t>::type;

    inline oblock_span_t oblock()
    {
        if constexpr (requires { typename std::integral_constant<std::endian, algo_t::cipher_endianness()>; }) {
            if constexpr (algo_t::cipher_endianness() != std::endian::native || !exact_word_size) {
                return oblock_span_t{ obytes_begin(), block_bsize() };
            } else {
                return oblock_span_t{ reinterpret_cast<const unsigned char*>(oblock_begin()), block_bsize() };
            }
        } else {
            if (algo_t::cipher_endianness() != std::endian::native || !exact_word_size) {
                return oblock_span_t{ obytes_begin(), block_bsize() };
            } else {
                return oblock_span_t{ reinterpret_cast<const unsigned char*>(oblock_begin()), block_bsize() };
            }
        }
    }

    inline cipher_mode_type cipher_mode() const { return (cipher_mode_type)cipher_mode_; }

    inline word_type* iv_begin() noexcept { return reinterpret_cast<word_type*>(obytes_begin()) - algo_t::block_wsize(); }
    inline word_type* iv_backup_begin() noexcept { return reinterpret_cast<word_type*>(obytes_begin()) - 2 * algo_t::block_wsize(); }

    inline word_type* iblock_begin() noexcept { return reinterpret_cast<word_type*>(obytes_begin()) - (2 + InQueueSzMultiplierV) * algo_t::block_wsize(); }
    inline word_type* oblock_begin() noexcept { return reinterpret_cast<word_type*>(obytes_begin()) - (3 + InQueueSzMultiplierV) * algo_t::block_wsize(); }

    inline bool is_oblock_ready() const noexcept { return oblock_ready & 1; }

private:
    inline unsigned char* fields_end() noexcept { return reinterpret_cast<unsigned char*>(this) + object_size; }

    // auxiliary buffer that can store 1 block of bytes
    inline unsigned char* aux_begin() noexcept { return fields_end() - algo_t::block_wsize() * algo_t::word_size / 8; }
    inline word_type* aux_wbegin() noexcept { return reinterpret_cast<word_type*>(aux_begin()); }

    inline unsigned char* obytes_begin() noexcept
    {
        if constexpr (requires { typename std::integral_constant<std::endian, algo_t::cipher_endianness()>; }) {
            static constexpr bool has_buffer = algo_t::cipher_endianness() != std::endian::native || sizeof(word_type) * CHAR_BIT != algo_t::word_size;
            return aux_begin() - (has_buffer ? algo_t::block_wsize() * algo_t::word_size / 8 : 0);
        } else {
            bool has_buffer = algo_t::cipher_endianness() != std::endian::native || sizeof(word_type) * CHAR_BIT != algo_t::word_size;
            return aux_begin() - (has_buffer ? algo_t::block_wsize() * algo_t::word_size / 8 : 0);
        }
    }

    inline void fill_input(size_t offset, int val, size_t cnt);
    
    template <size_t SrcBitC, size_t DestBitC, typename SrcT, std::integral T>
    inline void xe_copy(SrcT const* vsrc, size_t n, T* buffer, size_t boffset = 0);

    template <typename ErrorH>
    std::span<const unsigned char> check_pkcs(ErrorH const& errh);

    void increment_iv();
    void reversed_increment_iv();

    size_t object_size;

    uint_least16_t bytes_in_buf;
    uint_least16_t cipher_mode_ : 4;
    uint_least16_t oblock_ready : 1;
    uint_least16_t finalization_stage : 4;
    uint_least16_t reversed_ctr_flag : 1;
    padding_type pt;
};

template <typename CipherT>
struct shared_algorithm
{
    std::shared_ptr<CipherT> alg_;
    struct algo_destroyer
    {
        void operator()(CipherT* pobj) const
        {
            pobj->destroy();
        }
    };

    template <typename QrkT>
    explicit shared_algorithm(QrkT const& q)
        : alg_{ CipherT::create(q), algo_destroyer{} }
    {}

    CipherT& alg() const { return *alg_; }
};

template <typename ImplT, typename ErrorHandlerT>
class generic_block_encrypter 
    : public ImplT
    , public generic_pusher<ErrorHandlerT>
{
public:
    using input_element_type = unsigned char;
    using output_element_type = unsigned char;

    template <typename SrcQrkT, typename QrkT>
    generic_block_encrypter(SrcQrkT const&, QrkT const& q)
        : ImplT { q }
        , generic_pusher<ErrorHandlerT>{ q }
    { }

    template <CompatibleSpan<char> SpanT, typename ConsumerT>
    inline void push(SpanT ivals, ConsumerT&& cons)
    {
        ImplT::alg().push_data(ivals, [&cons, this](auto * block) {
            if (ImplT::alg().is_oblock_ready()) {
                cons(ImplT::alg().oblock());
            }
            ImplT::alg().encrypt_block(block);
        });
    }

    template <Integral<8> LEIT, typename ConsumerT>
    void push(const LEIT ival, ConsumerT&& cons)
    {
        push(std::span{ &ival, 1 }, std::forward<ConsumerT>(cons));
    }

    template <typename ConsumerT>
    void finish(ConsumerT && cons)
    {
        ImplT::alg().finalize_encryption(*this, std::forward<ConsumerT>(cons));
    }

    template <typename ProviderT>
    std::span<const output_element_type> pull(std::span<const input_element_type>& input, ProviderT p)
    {
        for (;;) {
            if (input.empty()) {
                input = span_cast<const input_element_type>(p());
                if (input.empty()) {
                    return ImplT::alg().pull_finalized_encryption(*this);
                }
            }
        
            if (ImplT::alg().pull_data(input, [this](auto block) { ImplT::alg().encrypt_block(block); })) {
                return ImplT::alg().oblock();
            }
        }
    }

    inline void reset() { ImplT::alg().reset(); }
};

template <typename ImplT, typename ErrorHandlerT>
class generic_block_decrypter
    : public ImplT
    , public generic_pusher<ErrorHandlerT>
{
public:
    using input_element_type = unsigned char;
    using output_element_type = unsigned char;

    template <typename QrkT, typename DestQrkT>
    generic_block_decrypter(QrkT const& q, DestQrkT const&)
        : ImplT{ q }
        , generic_pusher<ErrorHandlerT>{ q }
    { }

    template <CompatibleSpan<char> SpanT, typename ConsumerT>
    inline void push(SpanT ivals, ConsumerT&& cons)
    {
        ImplT::alg().push_data(ivals, [&cons, this](auto block) {
            if (ImplT::alg().is_oblock_ready()) {
                cons(ImplT::alg().oblock());
            }
            ImplT::alg().decrypt_block(block);
        });
    }

    template <Integral<8> LEIT, typename ConsumerT>
    void push(const LEIT ival, ConsumerT&& cons)
    {
        push(std::span{ &ival, 1 }, std::forward<ConsumerT>(cons));
    }

    template <typename ConsumerT>
    void finish(ConsumerT cons)
    {
        ImplT::alg().finalize_decryption(*this, std::forward<ConsumerT>(cons));
    }

    template <typename ProviderT>
    std::span<const output_element_type> pull(std::span<const input_element_type>& input, ProviderT p)
    {
        for (;;) {
            if (input.empty()) {
                input = span_cast<const input_element_type>(p());
                if (input.empty()) {
                    return ImplT::alg().pull_finalized_decryption(*this);
                }
            }

            if (ImplT::alg().pull_data(input, [this](auto block) { ImplT::alg().decrypt_block(block); })) {
                return ImplT::alg().oblock();
            }
        }
    }

    inline void reset() { ImplT::alg().reset(); }
};

}

#include "basic_block_cipher.ipp"
