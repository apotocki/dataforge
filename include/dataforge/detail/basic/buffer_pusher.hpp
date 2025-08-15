/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include <memory>
#include <algorithm>

namespace dataforge {

template <typename ET>
class buffer_pusher : public generic_pusher<void>
{
    std::shared_ptr<ET> buffer;
    size_t buffer_size;
    size_t current_size{ 0 };

public:
    using input_element_type = ET;
    using output_element_type = ET;

    template <typename SrcT>
    buffer_pusher(SrcT const& quark, buffer_qrk<ET> const& gt)
        : buffer_size{ gt.buffer_sz }
    {
        buffer.reset(new ET[buffer_size], [](ET* p) { delete[] p; });
    }

    template <CompatibleSpan<ET> SpanT, typename ConsumerT>
    void push(SpanT ispan, ConsumerT cons)
    {
        auto ivals = std::span{ ispan.data(), ispan.size() }; // to ensure subspan availability
        while (!ivals.empty()) {
            size_t b2wr = (std::min)(buffer_size, current_size + ivals.size()) - current_size;
            std::copy(ivals.begin(), ivals.begin() + b2wr, buffer.get() + current_size);
            current_size += b2wr;
            if (current_size == buffer_size) {
                cons(std::span<const ET>{buffer.get(), buffer_size});
                current_size = 0;
            }
            ivals = ivals.subspan(b2wr);
        }
    }

    template <std::integral LEIT, typename ConsumerT>
    void push(const LEIT ival, ConsumerT cons)
    {
        buffer.get()[current_size++] = ival;
        if (current_size == buffer_size) {
            cons(std::span<const ET>{buffer.get(), buffer_size});
            current_size = 0;
        }
    }

    template <typename ConsumerT>
    inline void flush(ConsumerT cons)
    {
        if (current_size) {
            cons(std::span<const ET>{buffer.get(), current_size});
            current_size = 0;
        }
    }

    template <typename ConsumerT>
    inline void finish(ConsumerT && cons)
    {
        flush(std::forward<ConsumerT>(cons));
    }

    inline void reset() noexcept { current_size = 0; }
};

template <typename SrcT, typename IT>
struct cvt_resolver<SrcT, buffer_qrk<IT>>
{
    using type = buffer_pusher<IT>;
};

}
