/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#include "../utility/data_ops.hpp"

namespace dataforge::magma_detail {

}

namespace dataforge {
    
template <typename DerivedT>
template <typename QrkT>
kuznyechik_cipher<DerivedT>::kuznyechik_cipher(QrkT const& q)
{

}

template <typename DerivedT>
void kuznyechik_cipher<DerivedT>::expand_key(std::span<const unsigned char> key)
{
    
}

template <typename DerivedT>
void kuznyechik_cipher<DerivedT>::encrypt_block(const word_type* in, word_type* out) noexcept
{

}

template <typename DerivedT>
void kuznyechik_cipher<DerivedT>::decrypt_block(const word_type* in, word_type* out) noexcept
{

}

}
