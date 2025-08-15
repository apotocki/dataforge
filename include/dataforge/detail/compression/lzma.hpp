/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include <new>
#include <span>

#include <lzma.h>

namespace dataforge::lzma_detail {

struct state
{
    lzma_stream strm_ = LZMA_STREAM_INIT;
    unsigned int buffer_size;
    lzma_ret ret = LZMA_OK;

    uint8_t* buffer() { return reinterpret_cast<uint8_t*>(this) + sizeof(state); }
    
    std::span<const unsigned char> span() const
    {
        return { reinterpret_cast<unsigned char const*>(this) + sizeof(state), buffer_size };
    }

    std::span<const unsigned char> used_span() const
    {
        return std::span{
            reinterpret_cast<char unsigned const*>(this) + sizeof(state),
            buffer_size - strm_.avail_out
        };
    }
    
    void reset_buffer()
    {
        strm_.avail_out = buffer_size;
        strm_.next_out = buffer();
    }

private:
    explicit state(unsigned int bufsz)
        : buffer_size{ bufsz }
    {
        strm_.avail_in = 0;
        reset_buffer();
    }

    ~state() = default;

public:
    static state* create(unsigned int buffsize)
    {
        char* place = new char[sizeof(state) + buffsize];
        return new(place) state(buffsize);
    }

    void destroy()
    {
        lzma_end(&strm_);
        this->~state();
        delete[] reinterpret_cast<char*>(this);
    }
};

}
