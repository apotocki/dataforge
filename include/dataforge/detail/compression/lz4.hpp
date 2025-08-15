/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include <new>
#include <span>
#include <cassert>

#include <lz4frame.h>

namespace dataforge::lz4_detail {

struct state
{
    union {
        LZ4F_compressionContext_t ctx_;
        LZ4F_decompressionContext_t ectx_;
    };

    LZ4F_preferences_t prefs_;
    size_t buffer_size;
    size_t avail_out{ 0 };
    uint8_t compressing_ : 1;
    uint8_t processing_finished_ : 1;
    uint8_t context_ready_ : 1;

    //unsigned char* in() { return reinterpret_cast<unsigned char*>(this) + sizeof(state); }
    //unsigned char* next_in() { return in() + in_size - free_in; }
    unsigned char* out() { return reinterpret_cast<unsigned char*>(this) + sizeof(state); }
    unsigned char* next_out() { return out() + avail_out; }
    inline size_t free_size() const { return buffer_size - avail_out; }

    size_t compress_begin()
    {
        return LZ4F_compressBegin(ctx_, out(), buffer_size, &prefs_);
    }

    template <CompatibleSpan<char> SpanT>
    size_t compress_update(SpanT data)
    {
        return LZ4F_compressUpdate(ctx_, next_out(), buffer_size - avail_out, data.data(), data.size(), nullptr);
    }

    size_t compress_end()
    {
        return LZ4F_compressEnd(ctx_, next_out(), buffer_size - avail_out, nullptr);
    }

    template <CompatibleSpan<char> SpanT>
    size_t extract(SpanT & data)
    {
        size_t dst_size = free_size();
        size_t src_size = data.size();
        size_t r = LZ4F_decompress(ectx_, next_out(), &dst_size, data.data(), &src_size, nullptr);
        avail_out = dst_size;
        data = data.subspan(src_size);
        return r;
    }

    std::span<const unsigned char> out_span()
    {
        return std::span{ out(), avail_out };
    }

    std::span<const unsigned char> detach_out_span()
    {
        size_t spsz = avail_out;
        avail_out = 0;
        return std::span{ out(), spsz };
    }

    template <typename ErrorH>
    void reset(ErrorH const& errh)
    {
        avail_out = 0;
        if (!compressing_) {
            LZ4F_resetDecompressionContext(ectx_);
            processing_finished_ = 0;
        } else {
            if (!context_ready_) {
                if (ctx_) LZ4F_freeCompressionContext(ctx_);
                size_t ret = LZ4F_createCompressionContext(&ctx_, LZ4F_VERSION);
                if (LZ4F_isError(ret)) {
                    errh("lz4 compressor initialization error", ret);
                }
            }
            processing_finished_ = 0;
        }
    }

private:
    explicit state(size_t bsz, LZ4F_preferences_t const& prefs, bool compressing)
        : buffer_size{ bsz }, prefs_{ prefs }, compressing_{ compressing }
        , processing_finished_{0}, context_ready_{0}
    {
        if (compressing) { ctx_ = nullptr; } else { ectx_ = nullptr; }
    }

    ~state()
    {

    }

public:
    template <typename QrkT>
    static state* create(QrkT const& qrk, bool compressing)
    {
        LZ4F_preferences_t prefs = {
            { 
                qrk.blockSizeID,
                qrk.block_linked ? LZ4F_blockLinked : LZ4F_blockIndependent,
                qrk.checksum ? LZ4F_contentChecksumEnabled : LZ4F_noContentChecksum,
                LZ4F_frame,
                0 /* unknown content size */, 0 /* no dictID */ 
                , qrk.block_checksum ? LZ4F_blockChecksumEnabled : LZ4F_noBlockChecksum
            },
            qrk.compression_level, /* compression level; 0 == default */
            0,   /* autoflush */
            0,   /* favor decompression speed */
            { 0, 0, 0 },  /* reserved, must be set to 0 */
        };
        
        size_t bsz = qrk.buffer_size;
        if (bsz == (std::numeric_limits<size_t>::max)()) {
            if (compressing) {
                bsz = LZ4F_compressBound(0, &prefs);
                bsz += bsz >> 1;
                bsz &= ~1023;
            } else {
                bsz = 65536;
            }
        } else if (compressing) {
            size_t sz = LZ4F_compressBound(0, &prefs);
            if (bsz < sz) {
                throw std::runtime_error("lz4 buffer is too small to compress data");
            }
        }
        //size_t outbuf_capacity = LZ4F_HEADER_SIZE_MAX + LZ4F_compressBound(qrk.src_buff_sz, &prefs);
        char* place = new char[sizeof(state) + bsz];
        return new(place) state(bsz, prefs, compressing);
    }

    void destroy_compressor()
    {
        if (ctx_) LZ4F_freeCompressionContext(ctx_);
        this->~state();
        delete[] reinterpret_cast<char*>(this);
    }

    void destroy_extractor()
    {
        if (ctx_) LZ4F_freeDecompressionContext(ectx_);
        this->~state();
        delete[] reinterpret_cast<char*>(this);
    }
};

}
