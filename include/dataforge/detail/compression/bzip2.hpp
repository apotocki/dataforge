/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include <new>
#include <span>

#include "bzlib.h"

namespace dataforge::bzip2_detail {

struct state
{
    bz_stream strm_;
    unsigned int buffer_size;
    int ret;

    unsigned char* buffer() { return reinterpret_cast<unsigned char *>(this) + sizeof(state); }
    
    std::span<const unsigned char> span() const
    {
        return std::span{ reinterpret_cast<unsigned char const*>(this) + sizeof(state), buffer_size };
    }

    std::span<const unsigned char> used_span() const
    {
        return std::span {
            reinterpret_cast<unsigned char const*>(this) + sizeof(state),
            buffer_size - strm_.avail_out
        };
    }

    void reset_buffer()
    {
        strm_.avail_out = buffer_size;
        strm_.next_out = reinterpret_cast<char*>(buffer());
    }

    int decompress()
    {
        ret = ::BZ2_bzDecompress(&strm_);
        return ret;
    }

private:
    explicit state(unsigned int bufsz)
        : buffer_size{ bufsz }
    {
        strm_.bzalloc = nullptr;
        strm_.bzfree = nullptr;
        strm_.opaque = nullptr;
        strm_.avail_in = 0;
        reset_buffer();
    }

    ~state()
    {

    }

public:
    static state* create(unsigned int buffsize)
    {
        char* place = new char[sizeof(state) + buffsize];
        return new(place) state(buffsize);
    }

    void destroy_compressor()
    {
        BZ2_bzCompressEnd(&strm_);
        this->~state();
        delete[] reinterpret_cast<char*>(this);
    }

    void destroy_extractor()
    {
        BZ2_bzDecompressEnd(&strm_);
        this->~state();
        delete[] reinterpret_cast<char*>(this);
    }
};

}
