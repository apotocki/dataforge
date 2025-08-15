/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

namespace dataforge {

template <class AlgoFactoryT, size_t InQueueSzMultiplierV, typename AllocatorT>
void basic_block_cipher<AlgoFactoryT, InQueueSzMultiplierV, AllocatorT>::fill_input(size_t offset, int val, size_t cnt)
{
    if (algo_t::cipher_endianness() == std::endian::little) {
        le_fill<algo_t::word_size>(iblock_begin(), offset, val, cnt);
    } else {
        be_fill<algo_t::word_size>(iblock_begin(), offset, val, cnt);
    }
}

template <class AlgoFactoryT, size_t InQueueSzMultiplierV, typename AllocatorT>
template <size_t SrcBitC, size_t DestBitC, typename SrcT, std::integral T>
void basic_block_cipher<AlgoFactoryT, InQueueSzMultiplierV, AllocatorT>::xe_copy(SrcT const* vsrc, size_t n, T* buffer, size_t boffset)
{
    if (algo_t::cipher_endianness() == std::endian::little) {
        le_copy<SrcBitC, DestBitC>(vsrc, n, buffer, boffset);
    } else {
        be_copy<SrcBitC, DestBitC>(vsrc, n, buffer, boffset);
    }
}

template <class AlgoFactoryT, size_t InQueueSzMultiplierV, typename AllocatorT>
template <typename ErrorH>
inline std::span<const unsigned char> basic_block_cipher<AlgoFactoryT, InQueueSzMultiplierV, AllocatorT>::check_pkcs(ErrorH const& errh)
{
    auto sp = std::span<const unsigned char>(oblock());
    auto padval = sp.back();
    size_t idx = 1;
    for (; idx < padval && idx < sp.size(); ++idx) {
        if (sp[sp.size() - idx - 1] != padval) break;
    }
    if (idx == padval && idx) {
        return sp.subspan(0, sp.size() - idx);
    } else {
        errh.on_error("wrong padding", sp, errh);
        return {};
    }
}

template <class AlgoFactoryT, size_t InQueueSzMultiplierV, typename AllocatorT>
void basic_block_cipher<AlgoFactoryT, InQueueSzMultiplierV, AllocatorT>::increment_iv()
{
    for (size_t i = algo_t::block_wsize(); i > 0; --i) {
        if constexpr (sizeof(word_type) * CHAR_BIT == algo_t::word_size) {
            if constexpr (std::endian::native == std::endian::little && sizeof(word_type) > 1) {
                word_type ival = reverse_bytes<algo_t::word_size>(iv_begin()[i - 1]);
                ++ival;
                iv_begin()[i - 1] = reverse_bytes<algo_t::word_size>(ival);
                if (ival) break;
            } else {
                if (++(iv_begin()[i - 1])) break;
            }
        } else { // sizeof(word_type) * CHAR_BIT > algo_t::word_size
            auto ival = iv_begin()[i - 1];
            if constexpr (std::endian::native == std::endian::little) {
                ival = reverse_bytes<algo_t::word_size>(ival);
            }
            word_type nextval = (ival + 1) & ((word_type(1) << algo_t::word_size) - 1);
            if constexpr (std::endian::native == std::endian::little) {
                nextval = reverse_bytes<algo_t::word_size>(nextval);
            }
            iv_begin()[i - 1] = nextval;
            if (nextval) break;
        }
    }
}

template <class AlgoFactoryT, size_t InQueueSzMultiplierV, typename AllocatorT>
void basic_block_cipher<AlgoFactoryT, InQueueSzMultiplierV, AllocatorT>::reversed_increment_iv()
{
    for (size_t i = 0; i < algo_t::block_wsize(); ++i) {
        if constexpr (sizeof(word_type) * CHAR_BIT == algo_t::word_size) {
            if constexpr (std::endian::native == std::endian::little && sizeof(word_type) > 1) {
                if (++(iv_begin()[i])) break;
            } else {
                word_type ival = reverse_bytes<algo_t::word_size>(iv_begin()[i]);
                ++ival;
                iv_begin()[i] = reverse_bytes<algo_t::word_size>(ival);
                if (ival) break;
            }
        } else { // sizeof(word_type) * CHAR_BIT > algo_t::word_size
            auto ival = iv_begin()[i];
            if constexpr (std::endian::native == std::endian::big) {
                ival = reverse_bytes<algo_t::word_size>(ival);
            }
            word_type nextval = (ival + 1) & ((word_type(1) << algo_t::word_size) - 1);
            if constexpr (std::endian::native == std::endian::big) {
                nextval = reverse_bytes<algo_t::word_size>(nextval);
            }
            iv_begin()[i] = nextval;
            if (nextval) break;
        }
    }
}

template <class AlgoFactoryT, size_t InQueueSzMultiplierV, typename AllocatorT>
template <Integral<8> ET, typename BlockProcessorT>
void basic_block_cipher<AlgoFactoryT, InQueueSzMultiplierV, AllocatorT>::push_data(std::span<ET> data, BlockProcessorT const& proc)
{
    const size_t bsz = block_bsize();
    if (bytes_in_buf) {
        const auto bytes_to_copy = static_cast<uint_least16_t>((std::min)(bsz - bytes_in_buf, data.size()));
        xe_copy<8, algo_t::word_size>(data.data(), bytes_to_copy, iblock_begin(), bytes_in_buf);
        
        if (bytes_in_buf + bytes_to_copy == bsz) {
            proc(iblock_begin());
            data = data.subspan(bytes_to_copy);
        } else {
            bytes_in_buf += bytes_to_copy;
            return;
        }
    }

    for (; data.size() >= bsz; data = data.subspan(bsz)) {
        if (algo_t::cipher_endianness() == std::endian::native && sizeof(word_type) * CHAR_BIT == algo_t::word_size && 
            !(reinterpret_cast<uintptr_t>(data.data()) % std::alignment_of_v<word_type>))
        {
            proc(reinterpret_cast<const word_type*>(data.data()));
        } else {
            xe_copy<8, algo_t::word_size>(data.data(), bsz, iblock_begin());
            proc(iblock_begin());
        }
    }

    bytes_in_buf = static_cast<uint_least16_t>(data.size());
    if (bytes_in_buf) {
        xe_copy<8, algo_t::word_size>(data.data(), bytes_in_buf, iblock_begin());
    }
}

// returns true if needs to refill out buffer
template <class AlgoFactoryT, size_t InQueueSzMultiplierV, typename AllocatorT>
template <Integral<8> ET, typename BlockProcessorT>
bool basic_block_cipher<AlgoFactoryT, InQueueSzMultiplierV, AllocatorT>::pull_data(std::span<ET>& input, BlockProcessorT const& proc)
{
    const size_t bsz = block_bsize();
    if (bytes_in_buf == bsz) {
        if (oblock_ready) {
            oblock_ready = 0;
            return true;
        }
        proc(iblock_begin());
        bytes_in_buf = 0;
    }
    if (bytes_in_buf) {
        const auto bytes_to_copy = static_cast<uint_least16_t>((std::min)(bsz - bytes_in_buf, input.size()));
        xe_copy<8, algo_t::word_size>(input.data(), bytes_to_copy, iblock_begin(), bytes_in_buf);
        
        input = input.subspan(bytes_to_copy);
        bytes_in_buf += bytes_to_copy;
        if (bytes_in_buf < bsz) return false;
        if (oblock_ready) {
            oblock_ready = 0;
            return true;
        }
        proc(iblock_begin());
        bytes_in_buf = 0;
    }
    if (input.size() >= bsz) {
        if (oblock_ready) {
            oblock_ready = 0;
            return true;
        }
        const ET* indata = input.data();
        if (algo_t::cipher_endianness() == std::endian::native && sizeof(word_type) * CHAR_BIT == algo_t::word_size &&
            !(reinterpret_cast<uintptr_t>(indata) % std::alignment_of_v<word_type>))
        {
            proc(reinterpret_cast<const word_type*>(indata));
        } else {
            xe_copy<8, algo_t::word_size>(indata, bsz, iblock_begin());
            proc(iblock_begin());
        }
        input = input.subspan(bsz);
    } else if (!input.empty()) { // 0 < input.size() < bsz
        xe_copy<8, algo_t::word_size>(input.data(), input.size(), iblock_begin());
        bytes_in_buf = static_cast<uint_least8_t>(input.size());
        input = {};
    }
    return false;
}

template <class AlgoFactoryT, size_t InQueueSzMultiplierV, typename AllocatorT>
void basic_block_cipher<AlgoFactoryT, InQueueSzMultiplierV, AllocatorT>::encrypt_block(const word_type* in)
{
    switch (cipher_mode())
    {
    case cipher_mode_type::ECB:
        algo_t::encrypt_block(in, oblock_begin());
        break;
    case cipher_mode_type::CBC:
        for (word_type* iv_vec_it = iv_begin(), *iv_vec_eit = iv_vec_it + algo_t::block_wsize(); iv_vec_it != iv_vec_eit; ++iv_vec_it, ++in)
            *iv_vec_it ^= *in;
        algo_t::encrypt_block(iv_begin(), oblock_begin());
        std::copy(oblock_begin(), oblock_begin() + algo_t::block_wsize(), iv_begin());
        break;
    case cipher_mode_type::CFB:
        algo_t::encrypt_block(iv_begin(), oblock_begin());
        for (word_type* out_it = oblock_begin(), *out_eit = out_it + algo_t::block_wsize(); out_it != out_eit; ++out_it, ++in)
            *out_it ^= *in;
        std::copy(oblock_begin(), oblock_begin() + algo_t::block_wsize(), iv_begin());
        break;
    case cipher_mode_type::OFB:
        algo_t::encrypt_block(iv_begin(), oblock_begin());
        std::copy(oblock_begin(), oblock_begin() + algo_t::block_wsize(), iv_begin());
        for (word_type* out_it = oblock_begin(), *out_eit = out_it + algo_t::block_wsize(); out_it != out_eit; ++out_it, ++in)
            *out_it ^= *in;
        break;
    case cipher_mode_type::CTR:
        if (reversed_ctr_flag) {
            reversed_increment_iv();
        }
        algo_t::encrypt_block(iv_begin(), oblock_begin());
        for (word_type* out_it = oblock_begin(), *out_eit = out_it + algo_t::block_wsize(); out_it != out_eit; ++out_it, ++in)
            *out_it ^= *in;
        if (!reversed_ctr_flag) {
            increment_iv();
        }
        break;
    case cipher_mode_type::PCBC:
        for (word_type* iv_vec_it = iv_begin(), *iv_vec_eit = iv_vec_it + algo_t::block_wsize(); iv_vec_it != iv_vec_eit; ++iv_vec_it, ++in)
            *iv_vec_it ^= *in;
        algo_t::encrypt_block(iv_begin(), oblock_begin());
        in -= algo_t::block_wsize();
        for (word_type* iv_it = iv_begin(), *out_it = oblock_begin(), *iv_eit = iv_it + algo_t::block_wsize(); iv_it != iv_eit; ++iv_it, ++out_it, ++in)
            *iv_it = *in ^ *out_it;
        break;
    default:
        throw std::runtime_error("encrypt_block is not implemented");
    }

    if (!exact_word_size || algo_t::cipher_endianness() != std::endian::native) {
        xe_copy<algo_t::word_size, 8>(oblock_begin(), algo_t::block_wsize(), obytes_begin());
    }
    oblock_ready = 1;
}

template <class AlgoFactoryT, size_t InQueueSzMultiplierV, typename AllocatorT>
template <typename ErrorH, typename ConsumerT>
void basic_block_cipher<AlgoFactoryT, InQueueSzMultiplierV, AllocatorT>::finalize_encryption(ErrorH const& errh, ConsumerT&& cons)
{
    const size_t bsz = block_bsize();
    size_t offset = 0;
    if (bytes_in_buf >= bsz) {
        if (oblock_ready) {
            cons(oblock());
        }
        for (;;) {
            encrypt_block(iblock_begin() + offset);
            bytes_in_buf -= static_cast<uint_least16_t>(bsz);
            offset += algo_t::block_wsize();
            if (bytes_in_buf < bsz) break;
            cons(oblock());
        }
    }

    if (auto psz = (bsz - bytes_in_buf) % bsz; psz || pt == padding_type::pkcs) {
        if (pt == padding_type::none) {
            switch (cipher_mode())
            {
            case cipher_mode_type::CFB:
            case cipher_mode_type::OFB:
            case cipher_mode_type::CTR:
                if (oblock_ready) {
                    cons(oblock());
                }
                //fill_input(offset * algo_t::word_size / 8 + bsz - psz, 0, psz);
                encrypt_block(iblock_begin() + offset);
                cons(oblock().subspan(0, bytes_in_buf));
                break;
            default:
                if (!oblock_ready) {
                    errh.on_error("Not enough input data, at least one filled block is needed.", errh);
                    return;
                }
                std::memcpy(aux_begin(), oblock().data(), bsz - psz);
                if (cipher_mode_type::ECB == cipher_mode()) {
                    xe_copy<8, algo_t::word_size>(oblock().data() + bsz - psz, psz, iblock_begin() + offset, bsz - psz);
                } else {
                    fill_input(offset * algo_t::word_size / 8 + bsz - psz, 0, psz);
                }
                encrypt_block(iblock_begin() + offset);
                cons(oblock());
                cons(std::span{aux_begin(), bsz - psz});
            }
            
            oblock_ready = 0;
        } else {
            if (oblock_ready) {
                cons(oblock());
            }
            if (pt == padding_type::zero) {
                fill_input(offset * algo_t::word_size / 8 + bsz - psz, 0, psz);
            }
            else if (pt == padding_type::pkcs) {
                if (!psz) psz = bsz;
                fill_input(offset * algo_t::word_size / 8 + bsz - psz, (int)psz, psz);
            }
            encrypt_block(iblock_begin());
        }
    }
    
    if (oblock_ready) {
        cons(oblock());
    }

    reset();
}

template <class AlgoFactoryT, size_t InQueueSzMultiplierV, typename AllocatorT>
template <typename ErrorH>
std::span<const unsigned char> basic_block_cipher<AlgoFactoryT, InQueueSzMultiplierV, AllocatorT>::pull_finalized_encryption(ErrorH const& errh)
{
    const size_t bsz = block_bsize();
    while (bytes_in_buf >= bsz) {
        if (oblock_ready) {
            oblock_ready = 0;
            return oblock();
        }
        encrypt_block(iblock_begin());
        bytes_in_buf -= static_cast<uint_least16_t>(bsz);
        if (!bytes_in_buf) break;
        std::memmove(iblock_begin(), iblock_begin() + algo_t::block_wsize(), (bytes_in_buf + bsz - 1) & ~(bsz - 1));
    }

    switch (finalization_stage) {
    case 0:
        if (auto psz = (bsz - bytes_in_buf) % bsz; psz || pt == padding_type::pkcs) {
            if (pt == padding_type::none) {
                switch (cipher_mode())
                {
                case cipher_mode_type::CFB:
                case cipher_mode_type::OFB:
                case cipher_mode_type::CTR:
                    //fill_input(bytes_in_buf, 0, psz);
                    if (oblock_ready) {
                        finalization_stage = 2;
                        return oblock();
                    }
                    finalization_stage = 15;
                    encrypt_block(iblock_begin());
                    return oblock().subspan(0, bytes_in_buf);
                default:
                    if (!oblock_ready) {
                        errh.on_error("Not enough input data, at least one filled block is needed.", errh);
                        finalization_stage = 15;
                        return {};
                    }
                    std::memcpy(aux_begin(), oblock().data(), bytes_in_buf);
                    if (cipher_mode_type::ECB == cipher_mode()) {
                        xe_copy<8, algo_t::word_size>(oblock().data() + bytes_in_buf, psz, iblock_begin(), bytes_in_buf);
                    } else {
                        fill_input(bytes_in_buf, 0, psz);
                    }
                    encrypt_block(iblock_begin());
                    finalization_stage = 1;
                    return oblock();
                }
            }
            
            if (pt == padding_type::zero) {
                fill_input(bytes_in_buf, 0, psz);
            } else if (pt == padding_type::pkcs) {
                if (!psz) psz = bsz;
                fill_input(bytes_in_buf, (int)psz, psz);
            }
            if (oblock_ready) {
                finalization_stage = 3;
                return oblock();
            }
            encrypt_block(iblock_begin());
        }
        if (oblock_ready) {
            finalization_stage = 15;
            return oblock();
        }
        break;
    case 1:
        finalization_stage = 15;
        return std::span{aux_begin(), bytes_in_buf};
    case 2:
        finalization_stage = 15;
        encrypt_block(iblock_begin());
        return oblock().subspan(0, bytes_in_buf);
    case 3:
        encrypt_block(iblock_begin());
        finalization_stage = 15;
        return oblock();
    default: // finalization_stage == 15
        break;
    }
    return {};
}

template <class AlgoFactoryT, size_t InQueueSzMultiplierV, typename AllocatorT>
void basic_block_cipher<AlgoFactoryT, InQueueSzMultiplierV, AllocatorT>::decrypt_block(const word_type* in, bool store_intermediate)
{
    switch (cipher_mode())
    {
    case cipher_mode_type::ECB:
        algo_t::decrypt_block(in, oblock_begin());
        break;
    case cipher_mode_type::CBC:
        algo_t::decrypt_block(in, oblock_begin());
        if (padding_type::none == pt && store_intermediate) {
            std::copy(iv_begin(), iv_begin() + algo_t::block_wsize(), aux_wbegin());
        }
        for (word_type* iv_it = iv_begin(), *iv_eit = iv_it + algo_t::block_wsize(), *out_it = oblock_begin(); iv_it != iv_eit; ++iv_it, ++out_it)
            *out_it ^= *iv_it;
        std::copy(in, in + algo_t::block_wsize(), iv_begin());
        break;
    case cipher_mode_type::CFB:
        algo_t::encrypt_block(iv_begin(), oblock_begin());
        for (word_type* out_it = oblock_begin(), *out_eit = out_it + algo_t::block_wsize(); out_it != out_eit; ++out_it, ++in)
            *out_it ^= *in;
        std::copy(in - algo_t::block_wsize(), in, iv_begin());
        break;
    case cipher_mode_type::OFB:
    case cipher_mode_type::CTR:
        return encrypt_block(in);
    case cipher_mode_type::PCBC:
        algo_t::decrypt_block(in, oblock_begin());
        for (word_type* iv_it = iv_begin(), *out_it = oblock_begin(), *iv_eit = iv_it + algo_t::block_wsize(); iv_it != iv_eit; ++iv_it, ++out_it, ++in) {
            *out_it ^= *iv_it; *iv_it = *in ^ *out_it;
        }
        break;
    default:
        throw std::runtime_error("decrypt_block is not implemented");
    }

    if (!exact_word_size || algo_t::cipher_endianness() != std::endian::native) {
        xe_copy<algo_t::word_size, 8>(oblock_begin(), algo_t::block_wsize(), obytes_begin());
    }
    oblock_ready = 1;
}

template <class AlgoFactoryT, size_t InQueueSzMultiplierV, typename AllocatorT>
template <typename ErrorH, typename ConsumerT>
void basic_block_cipher<AlgoFactoryT, InQueueSzMultiplierV, AllocatorT>::finalize_decryption(ErrorH const& errh, ConsumerT&& cons)
{
    const size_t bsz = block_bsize();
    size_t offset = 0;
    if (bytes_in_buf >= bsz) {
        if (oblock_ready) {
            cons(oblock());
        }
        for (;;) {
            decrypt_block(iblock_begin() + offset);
            bytes_in_buf -= static_cast<uint_least16_t>(bsz);
            offset += algo_t::block_wsize();
            if (bytes_in_buf < bsz) break;
            cons(oblock());
        }
    }

    if (bytes_in_buf && pt != padding_type::none) {
        if (oblock_ready) {
            cons(oblock());
            oblock_ready = 0;
        }
    }

    if (!oblock_ready) {
        if (pt == padding_type::pkcs || (bytes_in_buf && (cipher_mode_ & 1) == 0)) {
            errh.on_error("insufficient input data", errh);
        }
        if (!bytes_in_buf) return;
    }

    switch (pt) {
    case padding_type::none:
        if (bytes_in_buf) {
            switch (cipher_mode())
            {
            case cipher_mode_type::CBC:
                // here: aux_wbegin() is Y(n-2)
                // here: iv_begin() is Y(n-1), we don't need it, copy Y(n-2) into it
                std::copy(aux_wbegin(), aux_wbegin() + algo_t::block_wsize(), iv_begin());
                fill_input(offset * algo_t::word_size / 8 + bytes_in_buf, 0, bsz - bytes_in_buf);
                for (word_type* iv_it = iv_begin(), *in_it = iblock_begin() + offset, *out_it = oblock_begin(), *out_eit = out_it + algo_t::block_wsize(); out_it != out_eit; ++iv_it, ++out_it, ++in_it)
                    *out_it ^= *iv_it ^ *in_it;
                if (!exact_word_size || algo_t::cipher_endianness() != std::endian::native) {
                    xe_copy<algo_t::word_size, 8>(oblock_begin(), algo_t::block_wsize(), obytes_begin());
                }
                // here: oblock is (X(n) || r)
                //std::memcpy(aux_begin(), oblock().data(), bytes_in_buf);
                //xe_copy<8, algo_t::word_size>(oblock().data() + bytes_in_buf, bsz - bytes_in_buf, iblock_begin() + offset, bytes_in_buf);
                [[fallthrough]];
            case cipher_mode_type::ECB:
                std::memcpy(aux_begin(), oblock().data(), bytes_in_buf);
                xe_copy<8, algo_t::word_size>(oblock().data() + bytes_in_buf, bsz - bytes_in_buf, iblock_begin() + offset, bytes_in_buf);
                decrypt_block(iblock_begin() + offset, false);
                cons(oblock());
                cons(std::span{aux_begin(), bytes_in_buf});
                break;
            default:
            //case cipher_mode_type::CFB:
            //case cipher_mode_type::OFB:
            //case cipher_mode_type::CTR:
                if (oblock_ready) {
                    cons(oblock());
                }
                decrypt_block(iblock_begin() + offset, false);
                cons(oblock().subspan(0, bytes_in_buf));
                break;
            }
            
            break;
        }
        [[fallthrough]];
    case padding_type::zero:
        cons(oblock());
        break;
    case padding_type::pkcs:
        cons(check_pkcs(errh));
        break;
    default:
        errh.on_error("internal error: unexpected padding type value");
    }
    reset();
}

template <class AlgoFactoryT, size_t InQueueSzMultiplierV, typename AllocatorT>
template <typename ErrorH>
std::span<const unsigned char> basic_block_cipher<AlgoFactoryT, InQueueSzMultiplierV, AllocatorT>::pull_finalized_decryption(ErrorH const& errh)
{
    const size_t bsz = block_bsize();
    while (bytes_in_buf >= bsz) {
        if (oblock_ready) {
            oblock_ready = 0;
            return oblock();
        }
        decrypt_block(iblock_begin());
        bytes_in_buf -= static_cast<uint_least16_t>(bsz);
        if (!bytes_in_buf) break;
        std::memmove(iblock_begin(), iblock_begin() + algo_t::block_wsize(), (bytes_in_buf + bsz - 1) & ~(bsz - 1));
    }

    if (bytes_in_buf && pt != padding_type::none) {
        if (oblock_ready) {
            oblock_ready = 0;
            return oblock();
        }
    }
    if (!oblock_ready && !finalization_stage) {
        // we have to have at least 1 oblock of mode is pkcs
        // we have to have at least 1 oblock if input is not empty for ECB, CBC, PCBC 
        if (pt == padding_type::pkcs || (bytes_in_buf && (cipher_mode_ & 1) == 0)) {
            errh.on_error("insufficient input data", errh);
        }
        // if input empty then output is also empty
        if (!bytes_in_buf) return {};
    }

    switch (finalization_stage) {
    case 0:
        switch (pt) {
        case padding_type::none:
            if (bytes_in_buf) {
                switch (cipher_mode())
                {
                case cipher_mode_type::CBC:
                    std::copy(aux_wbegin(), aux_wbegin() + algo_t::block_wsize(), iv_begin());
                    fill_input(bytes_in_buf, 0, bsz - bytes_in_buf);
                    for (word_type* iv_it = iv_begin(), *in_it = iblock_begin(), *out_it = oblock_begin(), *out_eit = out_it + algo_t::block_wsize(); out_it != out_eit; ++iv_it, ++out_it, ++in_it)
                        *out_it ^= *iv_it ^ *in_it;
                    if (!exact_word_size || algo_t::cipher_endianness() != std::endian::native) {
                        xe_copy<algo_t::word_size, 8>(oblock_begin(), algo_t::block_wsize(), obytes_begin());
                    }
                    //std::memcpy(aux_begin(), oblock().data(), bytes_in_buf);
                    //xe_copy<8, algo_t::word_size>(oblock().data() + bytes_in_buf, bsz - bytes_in_buf, iblock_begin(), bytes_in_buf);
                    [[fallthrough]];
                case cipher_mode_type::ECB:
                    std::memcpy(aux_begin(), oblock().data(), bytes_in_buf);
                    xe_copy<8, algo_t::word_size>(oblock().data() + bytes_in_buf, bsz - bytes_in_buf, iblock_begin(), bytes_in_buf);
                    decrypt_block(iblock_begin(), false);
                    finalization_stage = 1;
                    return oblock();
                default:
                    if (oblock_ready) {
                        finalization_stage = 2;
                        return oblock();
                    }
                    finalization_stage = 15;
                    decrypt_block(iblock_begin(), false);
                    return oblock().subspan(0, bytes_in_buf);
                }
            }
            [[fallthrough]];
        case padding_type::zero:
            oblock_ready = 0;
            finalization_stage = 15;
            return oblock();
        case padding_type::pkcs:
            oblock_ready = 0;
            finalization_stage = 15;
            return check_pkcs(errh);
        default:
            errh.on_error("internal error: unexpected padding type value");
            break;
        }
        [[fallthrough]];
    case 1:
        finalization_stage = 15;
        return std::span{aux_begin(), bytes_in_buf};
    case 2:
        finalization_stage = 15;
        decrypt_block(iblock_begin(), false);
        return oblock().subspan(0, bytes_in_buf);
    default: // finalization_stage == 15
        return {};
    }
}

}
