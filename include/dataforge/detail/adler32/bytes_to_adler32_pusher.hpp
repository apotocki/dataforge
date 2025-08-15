/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

namespace dataforge {

#define DO1(buf,i)  {s1 += (buf)[i]; s2 += s1;}
#define DO2(buf,i)  DO1(buf,i); DO1(buf,i+1);
#define DO4(buf,i)  DO2(buf,i); DO2(buf,i+2);
#define DO8(buf,i)  DO4(buf,i); DO4(buf,i+4);
#define DO16(buf)   DO8(buf,0); DO8(buf,8);

class bytes_to_adler32_pusher
    : public generic_pusher<void>
{
    static constexpr uint_least32_t base = 65521UL;
    static constexpr uint_least32_t NMAX = 5552;

    uint_least32_t s1{ 1 };
    uint_least32_t s2{ 0 };

public:
    using input_element_type = unsigned char;
    using output_element_type = uint_least32_t;

    template <typename SrcTagT>
    bytes_to_adler32_pusher(SrcTagT const&, adler32_qrk const&)
    {

    }

    template <CompatibleSpan<char> SpanT, typename ConsumerT>
    inline void push(SpanT ivals, ConsumerT&&)
    {
        auto const* buf = ivals.data();
        size_t len = ivals.size();

        /* in case user likes doing a byte at a time, keep it fast */
        if (len == 1) {
            push(buf[0], nullptr);
            return;
        }

        /* in case short lengths are provided, keep it somewhat fast */
        if (len < 16) {
            while (len--) {
                s1 += *buf++;
                s2 += s1;
            }
            return;
        }

        /* do length NMAX blocks -- requires just one modulo operation */
        while (len >= NMAX) {
            len -= NMAX;
            auto n = NMAX / 16;          /* NMAX is divisible by 16 */
            do {
                DO16(buf);          /* 16 sums unrolled */
                buf += 16;
            } while (--n);
        }

        /* do remaining bytes (less than NMAX, still just one modulo) */
        if (len) {                  /* avoid modulos if none remaining */
            while (len >= 16) {
                len -= 16;
                DO16(buf);
                buf += 16;
            }
            while (len--) {
                s1 += *buf++;
                s2 += s1;
            }
        }
    }

    template <std::integral LEIT, typename ConsumerT>
    void push(const LEIT ival, ConsumerT&&)
    {
        s1 += static_cast<uint_least8_t>(ival);
        s2 += s1;
    }

    template <typename ConsumerT>
    void finish(ConsumerT cons)
    {
        s1 %= base;
        s2 %= base;
        cons((s2 << 16) + s1);
        s1 = 1; s2 = 0;
    }
};

template <typename FromEHT>
struct cvt_resolver<int_qrk<8, FromEHT>, adler32_qrk>
{
    using type = bytes_to_adler32_pusher;
};

}
