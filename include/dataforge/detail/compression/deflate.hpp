/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include <new>
#include <span>

#include "zlib.h"

namespace dataforge::deflate_detail {

struct state
{
    z_stream strm_;
    uInt buffer_size;
    int ret = Z_OK;

    Bytef* buffer() { return reinterpret_cast<uint_least8_t*>(this) + sizeof(state); }
    
    std::span<const unsigned char> span() const
    {
        return std::span{ reinterpret_cast<const unsigned char*>(this) + sizeof(state), buffer_size };
    }

    std::span<const unsigned char> used_span() const
    {
        return std::span{
            reinterpret_cast<unsigned char const*>(this) + sizeof(state),
            buffer_size - strm_.avail_out
        };
    }

    void reset_buffer()
    {
        strm_.avail_out = buffer_size;
        strm_.next_out = buffer();
    }

private:
    explicit state(uInt bufsz)
        : buffer_size{ bufsz }
    {
        strm_.zalloc = Z_NULL;
        strm_.zfree = Z_NULL;
        strm_.opaque = Z_NULL;
        strm_.avail_in = 0;
        reset_buffer();
    }

    ~state()
    {

    }

public:
    static state* create(uInt buffsize)
    {
        uint_least8_t* place = new uint_least8_t[sizeof(state) + buffsize];
        return new(place) state(buffsize);
    }

    void destroy_deflate()
    {
        deflateEnd(&strm_);
        this->~state();
        delete[] reinterpret_cast<uint_least8_t*>(this);
    }

    void destroy_inflate()
    {
        inflateEnd(&strm_);
        this->~state();
        delete[] reinterpret_cast<uint_least8_t*>(this);
    }
};

}
