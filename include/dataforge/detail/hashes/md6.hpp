/*=============================================================================
    Copyright (c) 2022 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include <cstdint>
#include <span>

#include "../utility/digest_base.hpp"

namespace dataforge::md6_detail {

struct md6_conf_common
{
    /* MD6 compression function constants                                  */

    static constexpr int md6_n = 89;    /* size of compression input block, in words  */
    static constexpr int md6_c = 16;  /* size of compression output, in words       */
    /* a c-word block is also called a "chunk"    */
    static constexpr int md6_max_r = 255;    /* max allowable value for number r of rounds */

    // for n = 89
    static constexpr int t0 = 17;     /* index for linear feedback */
    static constexpr int t1 = 18;     /* index for first input to first and */
    static constexpr int t2 = 21;     /* index for second input to first and */
    static constexpr int t3 = 31;     /* index for first input to second and */
    static constexpr int t4 = 67;     /* index for second input to second and */
    static constexpr int t5 = 89;     /* last tap */
};

template <int WordSizeV> struct md6_word_selector {
    /* MD6 wordsize.
    **
    ** Define md6 wordsize md6_w, in bits.
    ** Note that this is the "word size" relevant to the
    ** definition of md6 (it defines how many bits in an
    ** "md6_word");  it does *not* refer to the word size
    ** on the platform for which this is being compiled.
    */
    static constexpr int md6_w = WordSizeV;

    /* Define "md6_word" appropriately for given value of md6_w.
    ** The term `word' in comments means an `md6_word'.
    */
    //using md6_word = typename boost::uint_t<WordSizeV>::least;
};

template <> struct md6_word_selector<64> : md6_conf_common
{
    static constexpr int md6_w = 64;
    using md6_word = uint_least64_t;
    static const md6_word Q[15];

    static constexpr md6_word S0 = (md6_word)0x0123456789abcdefULL;
    static constexpr md6_word Smask = (md6_word)0x7311c2812425cfa0ULL;
    static void md6_main_compression_loop(md6_word* A, int r) noexcept;
};

template <> struct md6_word_selector<32> : md6_conf_common
{
    static constexpr int md6_w = 32;
    using md6_word = uint_least32_t;
    static const md6_word Q[30];

    static constexpr md6_word S0 = (md6_word)0x01234567UL;
    static constexpr md6_word Smask = (md6_word)0x7311c281UL;
    static void md6_main_compression_loop(md6_word* A, int r) noexcept;
};

template <> struct md6_word_selector<16> : md6_conf_common
{
    static constexpr int md6_w = 16;
    using md6_word = uint_least16_t;
    static const md6_word Q[60];

    static constexpr md6_word S0 = (md6_word)0x01234;
    static constexpr md6_word Smask = (md6_word)0x7311;
    static void md6_main_compression_loop(md6_word* A, int r) noexcept;
};

template <> struct md6_word_selector<8> : md6_conf_common
{
    static constexpr int md6_w = 8;
    using md6_word = uint_least8_t;
    static const md6_word Q[120];

    static constexpr md6_word S0 = (md6_word)0x01;
    static constexpr md6_word Smask = (md6_word)0x73;
    static void md6_main_compression_loop(md6_word* A, int r) noexcept;
};

template <int WordSizeV = 64>
struct md6_conf : md6_word_selector<WordSizeV>
{
    using md6_control_word = uint_least64_t;
    static md6_control_word md6_make_control_word(int r,        /* number rounds */
        int L,      /* parallel passes */
        int z,      /* final node flag */
        uint_least64_t p,         /* padding bits */
        size_t keylen,    /* bytes in key */
        int d           /* digest size */
    ) noexcept;

    using md6_nodeID = uint_least64_t;
    static md6_nodeID md6_make_nodeID(int ell,                   /* level number */
        uint64_t i    /* index (0,1,2,...) within level */
    ) noexcept;

    /* MD6 constants related to standard mode of operation                 */

    /* These five values give lengths of the components of compression     */
    /* input block; they should sum to md6_n.                              */
    static constexpr int md6_q = 15;         /* # Q words in compression block (>=0)       */
    static constexpr int md6_k = 8;        /* # key words per compression block (>=0)    */
    static constexpr int md6_u = (64 / md6_conf::md6_w); /* # words for unique node ID (0 or 64/w)     */
    static constexpr int md6_v = (64 / md6_conf::md6_w); /* # words for control word (0 or 64/w)       */
    static constexpr int md6_b = 64;         /* # data words per compression block (>0)    */

    static constexpr int md6_default_L = 64;    /* large so that MD6 is fully hierarchical */

    static constexpr int md6_max_stack_height = 29;
    /* max_stack_height determines the maximum number of bits that
    ** can be processed by this implementation (with default L) to be:
    **    (b*w) * ((b/c) ** (max_stack_height-3)
    **    = 2 ** 64  for b = 64, w = 64, c = 16, and  max_stack_height = 29
    ** (We lose three off the height since level 0 is unused,
    ** level 1 contains the input data, and C has 0-origin indexing.)
    ** The smallest workable value for md6_max_stack_height is 3.
    ** (To avoid stack overflow for non-default L values,
    ** we should have max_stack_height >= L + 2.)
    ** (One level of storage could be saved by letting st->N[] use
    ** 1-origin indexing, since st->N[0] is now unused.)
    */

    using compression_hook_t = void(typename md6_conf::md6_word* C,
        const typename md6_conf::md6_word* Q,
        typename md6_conf::md6_word* K,
        int ell,
        uint_least64_t i,
        int r,
        int L,
        int z,
        uint_least64_t p,
        size_t keylen,
        int d,
        typename md6_conf::md6_word* N
        );

    static compression_hook_t* compression_hook;
};

template <int WordSizeV>
typename md6_conf<WordSizeV>::compression_hook_t* md6_conf<WordSizeV>::compression_hook = nullptr;

enum class md6_error_code : int
{
    /* SUCCESS:  */
    MD6_SUCCESS = 0,

    /* ERROR CODES: */
    MD6_FAIL = 1,           /* some other problem                     */
    MD6_BADHASHLEN = 2,     /* hashbitlen<1 or >512 bits              */
    MD6_NULLSTATE = 3,      /* null state passed to MD6               */
    MD6_BADKEYLEN = 4,     /* key length is <0 or >512 bits          */
    MD6_STATENOTINIT = 5,   /* state was never initialized            */
    MD6_STACKUNDERFLOW = 6, /* MD6 stack underflows (shouldn't happen)*/
    MD6_STACKOVERFLOW = 7,  /* MD6 stack overflow (message too long)  */
    MD6_NULLDATA = 8,       /* null data pointer                      */
    MD6_NULL_N = 9,         /* compress: N is null                    */
    MD6_NULL_B = 10,        /* standard compress: null B pointer      */
    MD6_BAD_ELL = 11,       /* standard compress: ell not in {0,255}  */
    MD6_BAD_p = 12,         /* standard compress: p<0 or p>b*w        */
    MD6_NULL_K = 13,        /* standard compress: K is null           */
    MD6_NULL_Q = 14,        /* standard compress: Q is null           */
    MD6_NULL_C = 15,        /* standard compress: C is null           */
    MD6_BAD_L = 16,         /* standard compress: L <0 or > 255       */
    /* md6_init: L<0 or L>255                 */
    MD6_BAD_r = 17,         /* compress: r<0 or r>255                 */
    /* md6_init: r<0 or r>255                 */
    MD6_OUT_OF_MEMORY = 18  /* compress: storage allocation failed    */
};

/* MD6 state.
**
** md6_state is the main data structure for the MD6 hash function.
*/

template <typename ConfT>
struct md6_state_general : ConfT
{
    using ConfT::md6_w;
    using ConfT::md6_c;
    using ConfT::md6_k;
    using ConfT::md6_b;
    using ConfT::md6_max_stack_height;

    using md6_word = typename ConfT::md6_word;

    int d;           /* desired hash bit length. 1 <= d <= 512.      */
    int hashbitlen;  /* hashbitlen is the same as d; for NIST API    */

    unsigned char hashval[md6_c * (md6_w / 8)];
    /* e.g. unsigned char hashval[128]                           */
    /* contains hashval after call to md6_final                  */
    /* hashval appears in first floor(d/8) bytes, with           */
    /* remaining (d mod 8) bits (if any) appearing in            */
    /* high-order bit positions of hashval[1+floor(d/8)].        */

    //unsigned char hexhashval[(md6_c * (md6_w / 8)) + 1];
    /* e.g. unsigned char hexhashval[129];                       */
    /* zero-terminated string representing hex value of hashval  */

    int initialized;         /* zero, then one after md6_init called */
    uint_least64_t bits_processed;                /* bits processed so far */
    uint_least64_t compression_calls;    /* compression function calls made*/
    int finalized;          /* zero, then one after md6_final called */

    md6_word K[md6_k];
    /* k-word (8 word) key (aka "salt") for this instance of md6 */
    size_t keylen;
    /* number of bytes in key K. 0<=keylen<=k*(w/8)              */

    int L;
    /* md6 mode specification parameter. 0 <= L <= 255           */
    /* L == 0 means purely sequential (Merkle-Damgaard)          */
    /* L >= 29 means purely tree-based                           */
    /* Default is md6_default_L = 64 (hierarchical)              */

    int r;
    /* Number of rounds. 0 <= r <= 255                           */

    int top;
    /* index of block corresponding to top of stack              */

    md6_word B[md6_max_stack_height][md6_b];
    /* md6_word B[29][64]                                        */
    /* stack of 29 64-word partial blocks waiting to be          */
    /* completed and compressed.                                 */
    /* B[1] is for compressing text data (input);                */
    /* B[ell] corresponds to node at level ell in the tree.      */

    uint_least64_t bits[md6_max_stack_height];
    /* bits[ell] =                                               */
    /*    number of bits already placed in B[ell]                */
    /*    for 1 <= ell < max_stack_height                        */
    /* 0 <= bits[ell] <= b*w                                     */

    uint_least64_t i_for_level[md6_max_stack_height];
    /* i_for_level[ell] =                                        */
    /*    index of the node B[ ell ] on this level (0,1,...)     */
    /* when it is output   */
};

/*
struct md6_impl
{
    static constexpr int digest_length() { return 16; }
    using size_type = uint_least64_t;
    using element_type = uint_least8_t;
    using conf_t = md6_conf<>;
    using md6_state = md6_state_general<conf_t>;

    md6_impl() {}

    void reset();
};
*/

template <typename ConfT>
md6_error_code md6_init(
    md6_state_general<ConfT>* st,  /* state to initialize */
    int d,                          /* hash bit length */
    int rounds, /* rounds. if <=0 use default for the given d */
    std::span<const uint_least8_t> key,
    int l
) noexcept;

template <typename ConfT>
md6_error_code md6_update(md6_state_general<ConfT>* st, void const* data, uint64_t databitlen) noexcept;

template <typename ConfT>
md6_error_code md6_final(md6_state_general<ConfT>* st, unsigned char* hashval) noexcept;

}

#include "md6.ipp"
