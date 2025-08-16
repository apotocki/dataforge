/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include <bit>
#include <cstring>
#include <limits>

namespace dataforge {

inline uint16_t reverse_bytes(uint16_t value) noexcept
{
    return (value >> 8) | (value << 8);
}

inline uint32_t reverse_bytes(uint32_t value) noexcept
{
    value = (value >> 16) | (value << 16);
    return ((value & 0xff00ff00) >> 8) | ((value & 0x00ff00ff) << 8);
}

inline uint64_t reverse_bytes(uint64_t value) noexcept
{
    value = (value >> 32) | (value << 32);
    value = ((value & 0xffff0000ffff0000ull) >> 16) | ((value & 0x0000ffff0000ffffull) << 16);
    return ((value & 0xff00ff00ff00ff00ull) >> 8) | ((value & 0x00ff00ff00ff00ffull) << 8);
}

template <typename T>
inline void T_to_be(void* pdest, T const* psrc, int n) noexcept
{
    if constexpr (std::endian::native == std::endian::big) {
        std::memcpy(pdest, psrc, n * sizeof(T));
    } else {
        unsigned char* pd = reinterpret_cast<unsigned char*>(pdest);
        for (int i = 0; i < n; ++i) {
            if constexpr (std::endian::native == std::endian::little) {
                T v = reverse_bytes(*psrc++);
                std::memcpy(pd, &v, sizeof(T));
                pd += sizeof(T);
            } else {
                for (int j = 8 * (sizeof(T) - 1); j >= 0; j -= 8) {
                    *pd++ = static_cast<uint8_t>((*psrc) >> j);
                }
                ++psrc;
            }
        }
    }
}



/*
template <size_t SrcBitC, std::integral SrcT, size_t DestBitC, std::integral DestT, size_t DestCountV>
struct le_integral_caster
{
    mutable DestT buff[DestCountV];

    DestT const* cast(SrcT const* src) const
    {
        if constexpr (SrcBitC < DestBitC) {
            constexpr int k = DestBitC / SrcBitC;
            for (size_t i = 0; i < DestCountV; ++i)
            {
                buff[i] = src[i * k];
                for (size_t j = 1; j < k; ++j)
                    buff[i] |= static_cast<DestT>(src[i * k + j]) << SrcBitC * j;
            }
        } else {
            constexpr int k = SrcBitC / DestBitC;
            for (size_t i = 0; i < DestCountV; ++i)
            {
                buff[i] = static_cast<DestT>(src[i / k] >> DestBitC * (i % k));
            }
        }
        return buff;
    }
};

template <size_t SrcBitC, std::integral SrcT, size_t DestBitC, std::integral DestT, size_t DestCountV>
requires (sizeof(SrcT) * CHAR_BIT == SrcBitC &&
          sizeof(DestT) * CHAR_BIT == DestBitC &&
          std::endian::native == std::endian::little)
struct le_integral_caster<SrcBitC, SrcT, DestBitC, DestT, DestCountV>
{
    DestT const* cast(SrcT const* src) const
    {
        if constexpr (std::alignment_of_v<DestT> > std::alignment_of_v<SrcT>) {
            assert(!(reinterpret_cast<uintptr_t>(src) % std::alignment_of_v<DestT>));
        }
        return reinterpret_cast<DestT const*>(src);
    }
};
*/

// soffset is in src elements
template <size_t SrcBitC, size_t DestBitC, typename SrcT, std::integral T>
inline void le_copy(SrcT const* vsrc, size_t n, T* buffer, size_t soffset = 0)
{
    static_assert(
        std::endian::native == std::endian::big ||
        std::endian::native == std::endian::little,
        "unsupported endianness"
    );
    using src_t = std::conditional_t<std::is_void_v<SrcT>, std::byte, SrcT>;
    if constexpr (std::endian::native == std::endian::little &&
        sizeof(src_t) * CHAR_BIT == SrcBitC &&
        sizeof(T) * CHAR_BIT == DestBitC)
    {
        std::memcpy(reinterpret_cast<src_t*>(buffer) + soffset, vsrc, n * sizeof(src_t));
    } else if constexpr (SrcBitC < DestBitC) {
        T mask = (T(1) << SrcBitC) - 1;
        const auto* src = static_cast<const src_t*>(vsrc);
        constexpr int k = DestBitC / SrcBitC;
        for (size_t i = soffset / k, j = soffset % k; i < (SrcBitC * (n + soffset) + DestBitC - 1) / DestBitC; ++i, j = 0)
        {
            T dest = buffer[i];
            for (auto shift = SrcBitC * j; j < (std::min)(k, (int)(n + soffset - i * k)); ++j, shift += SrcBitC) {
                dest = ((~(mask << shift)) & dest) | ((T(*src++) & mask) << shift);
            }
            buffer[i] = dest;
        }
    } else {
        assert(!soffset); // not implemented case
        const auto* src = static_cast<const src_t*>(vsrc);
        constexpr int k = SrcBitC / DestBitC;
        for (size_t i = 0; i < SrcBitC * n / DestBitC; ++i)
        {
            *buffer++ = static_cast<T>(src[i / k] >> DestBitC * (i % k));
        }
    }
}

template <size_t DestBitC, std::integral T>
inline void le_fill(T* buffer, size_t boffset, int val, size_t cnt)
{
    if constexpr (std::endian::native == std::endian::little && sizeof(T) * CHAR_BIT == DestBitC) {
        std::memset(reinterpret_cast<char*>(buffer) + boffset, val, cnt);
    } else {
        T tval(val & 0xff);
        T mask = T(0xff);
        constexpr int k = DestBitC / 8;
        for (size_t i = boffset / k, j = boffset % k; i < (cnt + boffset + k - 1) / k; ++i, j = 0)
        {
            T dest = buffer[i];
            for (auto shift = 8 * j; j < (std::min)(k, (int)(cnt + boffset - i * k)); ++j, shift += 8) {
                dest = ((~(mask << shift)) & dest) | (tval << shift);
            }
            buffer[i] = dest;
        }
    }
}

template <size_t SrcBitC, size_t DestBitC, std::integral T, typename SrcT>
inline T* le_to_T(std::remove_const_t<T>* buffer, SrcT* vsrc, size_t n)
{
    static_assert(
        std::endian::native == std::endian::big ||
        std::endian::native == std::endian::little,
        "unsupported endianness"
    );

    using src_t = std::conditional_t<std::is_void_v<SrcT>, unsigned char, SrcT>;

    if constexpr (std::endian::native == std::endian::little &&
        sizeof(src_t) * CHAR_BIT == SrcBitC &&
        sizeof(T) * CHAR_BIT == DestBitC)
    {
        if constexpr (!std::is_const_v<SrcT> || std::is_const_v<T>) {
            if (!(reinterpret_cast<uintptr_t>(vsrc) % std::alignment_of_v<T>)) {
                return reinterpret_cast<T*>(vsrc);
            }
        }
    }
    le_copy<SrcBitC, DestBitC>(vsrc, n, buffer);
    return buffer;
}

// soffset is in src elements
template <size_t SrcBitC, size_t DestBitC, typename SrcT, std::integral T>
inline void be_copy(SrcT const* vsrc, size_t n, T* buffer, size_t soffset = 0)
{
    static_assert(
        std::endian::native == std::endian::big ||
        std::endian::native == std::endian::little,
        "unsupported endianness"
    );
    using src_t = std::conditional_t<std::is_void_v<SrcT>, unsigned char, SrcT>;
    if constexpr (std::endian::native == std::endian::big &&
        sizeof(src_t) * CHAR_BIT == SrcBitC &&
        sizeof(T) * CHAR_BIT == DestBitC)
    {
        std::memcpy(reinterpret_cast<src_t*>(buffer) + soffset, vsrc, n * sizeof(src_t));
    } else if constexpr (SrcBitC < DestBitC) {
        T mask = (T(1) << SrcBitC) - 1;
        const auto* src = static_cast<const src_t*>(vsrc);
        constexpr size_t k = DestBitC / SrcBitC;
        for (size_t i = soffset / k, j = soffset % k; i < (SrcBitC * (n + soffset) + DestBitC - 1) / DestBitC; ++i, j = 0)
        {
            T dest = buffer[i];
            for (auto shift = SrcBitC * (k - j - 1); j < (std::min)(k, (size_t)(n + soffset - i * k)); ++j, shift -= SrcBitC) {
                dest = ((~(mask << shift)) & dest) | ((T(*src++) & mask) << shift);
            }
            buffer[i] = dest;
        }
    } else {
        assert(!soffset); // not implemented case
        const auto* src = static_cast<const src_t*>(vsrc);
        constexpr int k = SrcBitC / DestBitC;
        for (size_t i = 0; i < SrcBitC * n / DestBitC; ++i)
        {
            *buffer++ = static_cast<T>(src[i / k] >> DestBitC * (k - i % k - 1));
        }
    }
}

template <size_t SrcBitC, size_t DestBitC, std::integral T, typename SrcT>
inline T* be_to_T(T* buffer, SrcT* vsrc, size_t n)
{
    static_assert(
        std::endian::native == std::endian::big ||
        std::endian::native == std::endian::little,
        "unsupported endianness"
    );

    using src_t = std::conditional_t<std::is_void_v<SrcT>, unsigned char, SrcT>;

    if constexpr (std::endian::native == std::endian::big &&
        sizeof(src_t) * CHAR_BIT == SrcBitC &&
        sizeof(T) * CHAR_BIT == DestBitC)
    {
        if constexpr (!std::is_const_v<SrcT> || std::is_const_v<T>) {
            if (!(reinterpret_cast<uintptr_t>(vsrc) % std::alignment_of_v<T>)) {
                return reinterpret_cast<T*>(vsrc);
            }
        }
    }
    be_copy<SrcBitC, DestBitC>(vsrc, n, buffer);
    return buffer;

#if 0
    if constexpr (std::endian::native == std::endian::big) {
        T* pldest = std::launder(pdest);
        std::memcpy(pldest, vsrc, n);
        return pldest;
    } else {
        const unsigned char* src = reinterpret_cast<const unsigned char*>(vsrc);
        const int size = sizeof(T);

        int i = 0;
        for (; i < n / size; ++i)
        {
            T dst_acc = (T)(*src++) << 8 * (size - 1);
            for (size_t j = 1; j < size; ++j)
                dst_acc |= ((T)(*src++)) << 8 * (size - j - 1);
            pdest[i] = dst_acc;
        }

        assert(n == i * size);
        /*
        T last = 0;
        for (int j = 0; j < n - i * size; ++j) {
            last |= static_cast<T>(src[size - j - 1]) << 8 * j;
        }
        pdest[i] = last;
        */
        return pdest;
        /*
        T* last = pdest + i;
        for (int j = 0, offs = i * size; j < n - offs; ++j) {
            static_cast<unsigned char*>(last)[j] = src[size - j - 1];
        }
        */
    }
#endif
}
template <size_t DestBitC, std::integral T>
inline void be_fill(T* buffer, size_t boffset, int val, size_t cnt)
{
    if constexpr (std::endian::native == std::endian::big && sizeof(T) * CHAR_BIT == DestBitC) {
        std::memset(reinterpret_cast<char*>(buffer) + boffset, val, cnt);
    } else {
        T tval(val & 0xff);
        T mask = T(0xff);
        constexpr size_t k = DestBitC / 8;
        for (size_t i = boffset / k, j = boffset % k; i < (cnt + boffset + k - 1) / k; ++i, j = 0)
        {
            T dest = buffer[i];
            for (size_t shift = 8 * (k - j - 1); j < (std::min)(k, (size_t)(cnt + boffset - i * k)); ++j, shift -= 8) {
                dest = ((~(mask << shift)) & dest) | (tval << shift);
            }
            buffer[i] = dest;
        }
    }
}

template <size_t BitC, typename WordT, typename SomeIntegerT>
inline WordT left_rotate(WordT x, SomeIntegerT bits)
{
    static_assert(BitC <= sizeof(WordT) * CHAR_BIT);
    if constexpr (sizeof(WordT) * CHAR_BIT != BitC) {
        x &= (WordT(1) << BitC) - 1;
    }
    return (x << (bits % BitC)) | (x >> (BitC - (bits % BitC)));
}

template <size_t BitC, typename WordT, typename SomeIntegerT>
inline WordT right_rotate(WordT x, SomeIntegerT bits)
{
    static_assert(BitC <= sizeof(WordT) * CHAR_BIT);
    if constexpr (sizeof(WordT) * CHAR_BIT != BitC) {
        x &= (WordT(1) << BitC) - 1;
    }
    return (x >> (bits % BitC)) | (x << (BitC - (bits % BitC)));
}

template <size_t BitC, std::integral T>
requires(std::numeric_limits<T>::digits >= BitC && BitC <= 64)
inline T reverse_bytes(T value) noexcept
{
    if constexpr (BitC > 32) {
        value = ((value & 0xFF00FF00FF00FF00ull) >> 8) | ((value & 0x00FF00FF00FF00FFull) << 8);
        value = ((value & 0xFFFF0000FFFF0000ull) >> 16) | ((value & 0x0000FFFF0000FFFFull) << 16);
        value = (value >> 32) | (value << 32);
    }
    else if constexpr (BitC > 16) {
        value = ((value & 0xFF00FF00) >> 8) | ((value & 0x00FF00FF) << 8);
        value = (value >> 16) | (value << 16);
    }
    else if constexpr (BitC > 8) {
        value = (value >> 8) | (value << 8);
    }
    return value;
}

inline uint64_t reverse_bits(uint64_t value) noexcept
{
    value = ((value & 0xAAAAAAAAAAAAAAAAull) >> 1) | ((value & 0x5555555555555555ull) << 1);
    value = ((value & 0xCCCCCCCCCCCCCCCCull) >> 2) | ((value & 0x3333333333333333ull) << 2);
    value = ((value & 0xF0F0F0F0F0F0F0F0ull) >> 4) | ((value & 0x0F0F0F0F0F0F0F0Full) << 4);
    value = ((value & 0xFF00FF00FF00FF00ull) >> 8) | ((value & 0x00FF00FF00FF00FFull) << 8);
    value = ((value & 0xFFFF0000FFFF0000ull) >> 16) | ((value & 0x0000FFFF0000FFFFull) << 16);
    value = (value >> 32) | (value << 32);

    return value;
}

inline uint32_t reverse_bits(uint32_t value) noexcept
{
    value = ((value & 0xAAAAAAAA) >> 1) | ((value & 0x55555555) << 1);
    value = ((value & 0xCCCCCCCC) >> 2) | ((value & 0x33333333) << 2);
    value = ((value & 0xF0F0F0F0) >> 4) | ((value & 0x0F0F0F0F) << 4);
    value = ((value & 0xFF00FF00) >> 8) | ((value & 0x00FF00FF) << 8);
    value = (value >> 16) | (value << 16);

    return value;
}

inline uint16_t reverse_bits(uint16_t value) noexcept
{
    value = ((value & 0xAAAA) >> 1) | ((value & 0x5555) << 1);
    value = ((value & 0xCCCC) >> 2) | ((value & 0x3333) << 2);
    value = ((value & 0xF0F0) >> 4) | ((value & 0x0F0F) << 4);
    value = (value >> 8) | (value << 8);

    return value;
}

inline uint8_t reverse_bits(uint8_t value) noexcept
{
    value = ((value & 0xAA) >> 1) | ((value & 0x55) << 1);
    value = ((value & 0xCC) >> 2) | ((value & 0x33) << 2);
    value = ((value & 0xF0) >> 4) | ((value & 0x0F) << 4);
    return value;
}

}
