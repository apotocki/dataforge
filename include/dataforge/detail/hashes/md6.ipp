/*=============================================================================
    Copyright (c) 2022 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#include <cstring>
#include <algorithm>
#include <memory>
#include <bit>

namespace dataforge::md6_detail {

/* Default number of rounds                                    */
/* (as a function of digest size d and keylen                  */
inline int md6_default_r(int d, size_t keylen) noexcept
{
	/* Default number of rounds is forty plus floor(d/4) */
	int r = 40 + (d / 4);
	/* unless keylen > 0, in which case it must be >= 80 as well */
	if (keylen > 0)
		r = (std::max)(80, r);
	return r;
}

/* return byte-reversal of md6_word x.
** Written to work for any w, w=8,16,32,64.
*/
template <typename WordT>
WordT md6_byte_reverse(WordT x) noexcept
{
	constexpr WordT mask8 = (WordT)0x00ff00ff00ff00ffULL;
	constexpr WordT mask16 = (WordT)0x0000ffff0000ffffULL;

	if constexpr (sizeof(WordT) == 8) {
		x = (x << 32) | (x >> 32);
	}

	if constexpr (sizeof(WordT) >= 4) {
		x = ((x & mask16) << 16) | ((x & ~mask16) >> 16);
	}

	if constexpr (sizeof(WordT) >= 2) {
		x = ((x & mask8) << 8) | ((x & ~mask8) >> 8);
	}
	return x;
}

/* Byte-reverse words x[0...count-1] if machine is little_endian */
template <typename WordT>
void md6_reverse_little_endian(WordT* x, int count) noexcept
{
	if (std::endian::native == std::endian::little)
		for (int i = 0; i < count; i++)
			x[i] = md6_byte_reverse(x[i]);
}

/* Initialize md6_state
** Input:
**     st         md6_state to be initialized
**     d          desired hash bit length 1 <= d <= w*(c/2)    (<=512 bits)
**     key        key (aka salt) for this hash computation     (byte array)
**                defaults to all-zero key if key==NULL or keylen==0
**     keylen     length of key in bytes; 0 <= keylen <= (k*8) (<=64 bytes)
**     L          md6 mode parameter; 0 <= L <= 255
**                md6.h defines md6_default_L for when you want default
**     r          number of rounds; 0 <= r <= 255
** Output:
**     updates components of state
**     returns one of the following:
**       MD6_SUCCESS
**       MD6_NULLSTATE
**       MD6_BADKEYLEN
**       MD6_BADHASHLEN
*/
template <typename ConfT>
md6_error_code md6_full_init(md6_state_general<ConfT>* st,       /* uninitialized state to use */
	int d,                          /* hash bit length */
	const uint_least8_t* key,        /* key; OK to give NULL */
	size_t keylen,     /* keylength (bytes); OK to give 0 */
	int L,           /* mode; OK to give md6_default_L */
	int r                          /* number of rounds */
) noexcept
{ /* check that md6_full_init input parameters make some sense */
	if (!st) return md6_error_code::MD6_NULLSTATE;
	if (key && ((keylen < 0) || (keylen > ConfT::md6_k * (ConfT::md6_w / 8))))
		return md6_error_code::MD6_BADKEYLEN;
	if (d < 1 || d > 512 || d > ConfT::md6_w * ConfT::md6_c / 2) return md6_error_code::MD6_BADHASHLEN;

	//md6_detect_byte_order();
	memset(st, 0, sizeof(md6_state_general<ConfT>));  /* clear state to zero */
	st->d = d;                       /* save hashbitlen */
	if (key && keylen > 0)   /* if no key given, use memset zeros*/
	{
		memcpy(st->K, key, keylen);    /* else save key (with zeros added) */
		st->keylen = keylen;
		/* handle endian-ness */       /* first byte went into high end */
		md6_reverse_little_endian(st->K, ConfT::md6_k);
	}
	else
		st->keylen = 0;
	if ((L < 0) || (L > 255)) return md6_error_code::MD6_BAD_L;
	st->L = L;
	if ((r < 0) || (r > 255)) return md6_error_code::MD6_BAD_r;
	st->r = r;
	st->initialized = 1;
	st->top = 1;
	/* if SEQ mode for level 1; use IV=0  */
	/* zero bits already there by memset; */
	/* we just need to set st->bits[1]    */
	if (L == 0) st->bits[1] = ConfT::md6_c * ConfT::md6_w;
	//ConfT::compression_hook = nullptr;     /* just to be sure default is "not set" */
	return md6_error_code::MD6_SUCCESS;
}

template <typename ConfT>
md6_error_code md6_init(md6_state_general<ConfT>* st, int d, int rounds, std::span<const uint_least8_t> key, int l) noexcept
{
	return md6_full_init(st,
		d,
		key.data(),
		key.size(),
		l < 0 ? ConfT::md6_default_L : l,
		rounds > 0 ? rounds : md6_default_r(d, key.size())
	);
}

/* Assumes n-word input array N has been fully set up.
** Input:
**   N               input array of n w-bit words (n=89)
**   A               working array of a = rc+n w-bit words
**                   A is OPTIONAL, may be given as NULL
**                   (then md6_compress allocates and uses its own A).
**   r               number of rounds
** Modifies:
**   C               output array of c w-bit words (c=16)
** Returns one of the following:
**   MD6_SUCCESS (0)
**   MD6_NULL_N
**   MD6_NULL_C
**   MD6_BAD_r
**   MD6_OUT_OF_MEMORY
*/
template <typename ConfT>
md6_error_code md6_compress(typename ConfT::md6_word* C,
	typename ConfT::md6_word* N,
	int r,
	typename ConfT::md6_word* A
) noexcept
{
	using md6_word = typename ConfT::md6_word;
	md6_word* A_as_given = A;

	/* check that input is sensible */
	if (!N) return md6_error_code::MD6_NULL_N;
	if (!C) return md6_error_code::MD6_NULL_C;
	if (r<0 || r > ConfT::md6_max_r) return md6_error_code::MD6_BAD_r;

	if (!A) A = reinterpret_cast<md6_word*>(calloc(r * ConfT::md6_c + ConfT::md6_n, sizeof(md6_word)));
	if (!A) return md6_error_code::MD6_OUT_OF_MEMORY;

	std::memcpy(A, N, ConfT::md6_n * sizeof(md6_word));    /* copy N to front of A */

	ConfT::md6_main_compression_loop(A, r);          /* do all the work */

	std::memcpy(C, A + (r - 1) * ConfT::md6_c + ConfT::md6_n, ConfT::md6_c * sizeof(md6_word)); /* output into C */

	if (!A_as_given)           /* zero and free A if nec. */
	{
		memset(A, 0, (r * ConfT::md6_c + ConfT::md6_n) * sizeof(md6_word)); /* contains key info */
		free(A);
	}

	return md6_error_code::MD6_SUCCESS;
}

/* Construct control word V for given inputs.
** Input:
**   r = number of rounds
**   L = mode parameter (maximum tree height)
**   z = 1 iff this is final compression operation
**   p = number of pad bits in a block to be compressed
**   keylen = number of bytes in key
**   d = desired hash output length
**   Does not check inputs for validity.
** Returns:
**   V = constructed control word
*/
template <int WordSizeV>
typename md6_conf<WordSizeV>::md6_control_word md6_conf<WordSizeV>::md6_make_control_word(int r,        /* number rounds */
	int L,      /* parallel passes */
	int z,      /* final node flag */
	uint64_t p,         /* padding bits */
	size_t keylen,    /* bytes in key */
	int d           /* digest size */
) noexcept
{
	md6_control_word V;
	V = ((((md6_control_word)0) << 60) | /* reserved, width  4 bits */
		(((md6_control_word)r) << 48) |           /* width 12 bits */
		(((md6_control_word)L) << 40) |           /* width  8 bits */
		(((md6_control_word)z) << 36) |           /* width  4 bits */
		(((md6_control_word)p) << 20) |           /* width 16 bits */
		(((md6_control_word)keylen) << 12) |     /* width  8 bits */
		(((md6_control_word)d)));                /* width 12 bits */
	return V;
}

/* Make "unique nodeID" U based on level ell and position i
** within level; place it at specified destination.
** Inputs:
**    dest = address of where nodeID U should be placed
**    ell = integer level number, 1 <= ell <= ...
**    i = index within level, i = 0, 1, 2,...
** Returns
**    U = constructed nodeID
*/
template <int WordSizeV>
typename md6_conf<WordSizeV>::md6_nodeID md6_conf<WordSizeV>::md6_make_nodeID(int ell, uint64_t i) noexcept
{
	md6_nodeID U;
	U = ((((md6_nodeID)ell) << 56) |
		((md6_nodeID)i));
	return U;
}

/*Assembling components of compression input.
*/
/* Pack data before compression into n-word array N.
*/
template <typename ConfT>
void md6_pack(typename ConfT::md6_word* N,
	const typename ConfT::md6_word* Q,
	typename ConfT::md6_word* K,
	int ell, uint64_t i,
	int r, int L, int z, uint64_t p, size_t keylen, int d,
	typename ConfT::md6_word* B) noexcept
{
	typename ConfT::md6_nodeID U;
	typename ConfT::md6_control_word V;

	int ni = 0;

	for (int j = 0; j < ConfT::md6_q; j++) N[ni++] = Q[j];       /* Q: Q in words     0--14 */

	for (int j = 0; j < ConfT::md6_k; j++) N[ni++] = K[j];       /* K: key in words  15--22 */

	U = ConfT::md6_make_nodeID(ell, i);             /* U: unique node ID in 23 */
	/* The following also works for variants
	** in which u=0.
	*/
	std::memcpy((unsigned char*)&N[ni],
		&U,
		(std::min)(size_t(ConfT::md6_u * (ConfT::md6_w / 8)), sizeof(typename ConfT::md6_nodeID)));
	ni += ConfT::md6_u;

	V = ConfT::md6_make_control_word(
		r, L, z, p, keylen, d);/* V: control word in   24 */
	/* The following also works for variants
	** in which v=0.
	*/
	std::memcpy((unsigned char*)&N[ni],
		&V,
		(std::min)(size_t(ConfT::md6_v * (ConfT::md6_w / 8)), sizeof(typename ConfT::md6_control_word)));
	ni += ConfT::md6_v;

	std::memcpy(N + ni, B, ConfT::md6_b * sizeof(typename ConfT::md6_word));      /* B: data words    25--88 */
}

/* Standard compress: assemble components and then compress
*/
/* Perform md6 block compression using all the "standard" inputs.
** Input:
**     Q              q-word (q=15) approximation to (sqrt(6)-2)
**     K              k-word key input (k=8)
**     ell            level number
**     i              index within level
**     r              number of rounds in this compression operation
**     L              mode parameter (max tree height)
**     z              1 iff this is the very last compression
**     p              number of padding bits of input in payload B
**     keylen         number of bytes in key
**     d              desired output hash bit length
**     B              b-word (64-word) data input block (with zero padding)
** Modifies:
**     C              c-word output array (c=16)
** Returns one of the following:
**   MD6_SUCCESS (0)   MD6_BAD_p
**   MD6_NULL_B        MD6_BAD_HASHLEN
**   MD6_NULL_C        MD6_NULL_K
**   MD6_BAD_r         MD6_NULL_Q
**   MD6_BAD_ELL       MD6_OUT_OF_MEMORY
*/
template <typename ConfT>
md6_error_code md6_standard_compress(typename ConfT::md6_word* C,
	const typename ConfT::md6_word* Q,
	typename ConfT::md6_word* K,
	int ell, uint64_t i,
	int r, int L, int z, uint64_t p, size_t keylen, int d,
	typename ConfT::md6_word* B
) noexcept
{
	using md6_word = typename ConfT::md6_word;
	md6_word N[ConfT::md6_n];
	std::unique_ptr<md6_word[]> A(new md6_word[5000]);
	//md6_word A[5000];       /* MS VS can't handle variable size here */

	/* check that input values are sensible */
	if (!C) return md6_error_code::MD6_NULL_C;
	if (!B) return md6_error_code::MD6_NULL_B;
	if ((r < 0) || (r > ConfT::md6_max_r)) return md6_error_code::MD6_BAD_r;
	if ((L < 0) || (L > 255)) return md6_error_code::MD6_BAD_L;
	if ((ell < 0) || (ell > 255)) return md6_error_code::MD6_BAD_ELL;
	if ((p < 0) || (p > ConfT::md6_b * ConfT::md6_w)) return md6_error_code::MD6_BAD_p;
	if ((d <= 0) || (d > ConfT::md6_c * ConfT::md6_w / 2)) return md6_error_code::MD6_BADHASHLEN;
	if (!K) return md6_error_code::MD6_NULL_K;
	if (!Q) return md6_error_code::MD6_NULL_Q;

	/* pack components into N for compression */
	md6_pack<ConfT>(N, Q, K, ell, i, r, L, z, p, keylen, d, B);

	/* call compression hook if it is defined. */
	/* -- for testing and debugging.           */
	if (ConfT::compression_hook)
		ConfT::compression_hook(C, Q, K, ell, i, r, L, z, p, keylen, d, B);

	return md6_compress<ConfT>(C, N, r, A.get());
}

/* Append bit string src to the end of bit string dest
** Input:
**     dest         a bit string of destlen bits, starting in dest[0]
**                  if destlen is not a multiple of 8, the high-order
**                  bits are used first
**     src          a bit string of srclen bits, starting in src[0]
**                  if srclen is not a multiple of 8, the high-order
**                  bits are used first
** Modifies:
**     dest         when append_bits returns, dest will be modified to
**                  be a bit-string of length (destlen+srclen).
**                  zeros will fill any unused bit positions in the
**                  last byte.
*/
inline void append_bits(unsigned char* dest, uint64_t destlen,
	const unsigned char* src, uint64_t srclen) noexcept
{
	int i, accumlen;
	uint16_t accum;

	if (srclen == 0) return;

	/* Initialize accum, accumlen, and di */
	accum = 0;    /* accumulates bits waiting to be moved, right-justified */
	accumlen = 0; /* number of bits in accumulator */
	if (destlen % 8 != 0)
	{
		accumlen = destlen % 8;
		accum = dest[destlen / 8];        /* grab partial byte from dest     */
		accum = accum >> (8 - accumlen);  /* right-justify it in accumulator */
	}
	uint64_t di = destlen / 8;        /* index of where next byte will go within dest */

	/* Now process each byte of src */
	uint64_t srcbytes = (srclen + 7) / 8;   /* number of bytes (full or partial) in src */
	for (i = 0; i < srcbytes; i++)
	{ /* shift good bits from src[i] into accum */
		if (i != srcbytes - 1) /* not last byte */
		{
			accum = (accum << 8) ^ src[i];
			accumlen += 8;
		}
		else /* last byte */
		{
			int newbits = ((srclen % 8 == 0) ? 8 : (srclen % 8));
			accum = (accum << newbits) | (src[i] >> (8 - newbits));
			accumlen += newbits;
		}
		/* do as many high-order bits of accum as you can (or need to) */
		while (((i != srcbytes - 1) && (accumlen >= 8)) ||
			((i == srcbytes - 1) && (accumlen > 0)))
		{
			int numbits = (std::min)(8, accumlen);
			unsigned char bits;
			bits = accum >> (accumlen - numbits);    /* right justified */
			bits = bits << (8 - numbits);              /* left justified  */
			bits &= (0xff00 >> numbits);             /* mask            */
			dest[di++] = bits;                       /* save            */
			accumlen -= numbits;
		}
	}
}

/*
Here are some notes on the data structures used (inside state).
* The variable B[] is a stack of length-b (b-64) word records,
  each corresponding to a node in the tree.  B[ell] corresponds
  to a node at level ell.  Specifically, it represents the record which,
  when compressed, will yield the value at that level. (It only
  contains the data payload, not the auxiliary information.)
  Note that B[i] is used to store the *inputs* to the computation at
  level i, not the output for the node at that level.
  Thus, for example, the message input is stored in B[1], not B[0].
* Level 0 is not used.  The message bytes are placed into B[1].
* top is the largest ell for which B[ell] has received data,
  or is equal to 1 in case no data has been received yet at all.
* top is never greater than L+1.  If B[L+1] is
  compressed, the result is put back into B[L+1]  (this is SEQ).
* bits[ell] says how many bits have been placed into
  B[ell].  An invariant maintained is that of the bits in B[ell],
  only the first bits[ell] may be nonzero; the following bits must be zero.
* The B nodes may have somewhat different formats, depending on the level:
  -- Level 1 node contains a variable-length bit-string, and so
	 0 <= bits[1] <= b*w     is all we can say.
  -- Levels 2...top always receive data in c-word chunks (from
	 children), so for them bits[ell] is between 0 and b*w,
	 inclusive, but is also a multiple of cw.  We can think of these
	 nodes as have (b/c) (i.e. 4) "slots" for chunks.
  -- Level L+1 is special, in that the first c words of B are dedicated
	 to the "chaining variable" (or IV, for the first node on the level).
* When the hashing is over, B[top] will contain the
  final hash value, in the first or second (if top = L+1) slot.
*/
/* Compress one block -- compress data at a node (md6_compress_block).
*/

/* compress block at level ell, and put c-word result into C.
** Input:
**     st         current md6 computation state
**     ell        0 <= ell < max_stack_height-1
**     z          z = 1 if this is very last compression; else 0
** Output:
**     C          c-word array to put result in
** Modifies:
**     st->bits[ell]  (zeroed)
**     st->i_for_level[ell] (incremented)
**     st->B[ell] (zeroed)
**     st->compression_calls (incremented)
** Returns one of the following:
**     MD6_SUCCESS
**     MD6_NULLSTATE
**     MD6_STATENOTINIT
**     MD6_STACKUNDERFLOW
**     MD6_STACKOVERFLOW
*/
template <typename ConfT>
md6_error_code md6_compress_block(typename ConfT::md6_word* C,
	md6_state_general<ConfT>* st,
	int ell,
	int z
) noexcept
{
	/* check that input values are sensible */
	if (!st) return md6_error_code::MD6_NULLSTATE;
	if (st->initialized == 0) return md6_error_code::MD6_STATENOTINIT;
	if (ell < 0) return md6_error_code::MD6_STACKUNDERFLOW;
	if (ell >= ConfT::md6_max_stack_height - 1) return md6_error_code::MD6_STACKOVERFLOW;

	st->compression_calls++;

	if (ell == 1) /* leaf; hashing data; reverse bytes if nec. */
	{
		if (ell < (st->L + 1)) /* PAR (tree) node */
			md6_reverse_little_endian(&(st->B[ell][0]), ConfT::md6_b);
		else /* SEQ (sequential) node; don't reverse chaining vars */
			md6_reverse_little_endian(&(st->B[ell][ConfT::md6_c]), ConfT::md6_b - ConfT::md6_c);
	}

	uint64_t p = ConfT::md6_b * ConfT::md6_w - st->bits[ell];          /* number of pad bits */

	md6_error_code err =
		md6_standard_compress<ConfT>(
			C,                                      /* C    */
			ConfT::Q,                               /* Q    */
			st->K,                                  /* K    */
			ell, st->i_for_level[ell],              /* -> U */
			st->r, st->L, z, p, st->keylen, st->d,  /* -> V */
			st->B[ell]                              /* B    */
			);
	if ((int)err) return err;

	st->bits[ell] = 0; /* clear bits used count this level */
	st->i_for_level[ell]++;

	std::memset(&(st->B[ell][0]), 0, ConfT::md6_b * sizeof(typename ConfT::md6_word));     /* clear B[ell] */
	return md6_error_code::MD6_SUCCESS;
}

/*
** Do processing of level ell (and higher, if necessary) blocks.
**
** Input:
**     st         md6 state that has been accumulating message bits
**                and/or intermediate results
**     ell        level number of block to process
**     final      true if this routine called from md6_final
**                     (no more input will come)
**                false if more input will be coming
**                (This is not same notion as "final bit" (i.e. z)
**                 indicating the last compression operation.)
** Output (by side effect on state):
**     Sets st->hashval to final chaining value on final compression.
** Returns one of the following:
**     MD6_SUCCESS
**     MD6_NULLSTATE
**     MD6_STATENOTINIT
*/
template <typename ConfT>
md6_error_code md6_process(md6_state_general<ConfT>* st,
	int ell,
	int final) noexcept
{
	int z, next_level;
	typename ConfT::md6_word C[ConfT::md6_c];

	/* check that input values are sensible */
	if (!st) return md6_error_code::MD6_NULLSTATE;
	if (!st->initialized) return md6_error_code::MD6_STATENOTINIT;

	if (!final) /* not final -- more input will be coming */
	{ /* if not final and block on this level not full, nothing to do */
		if (st->bits[ell] < ConfT::md6_b * ConfT::md6_w)
			return md6_error_code::MD6_SUCCESS;
		/* else fall through to compress this full block,
		**       since more input will be coming
		*/
	}
	else /* final -- no more input will be coming */
	{
		if (ell == st->top)
		{
			if (ell == (st->L + 1)) /* SEQ node */
			{
				if (st->bits[ell] == ConfT::md6_c * ConfT::md6_w && st->i_for_level[ell] > 0)
					return md6_error_code::MD6_SUCCESS;
				/* else (bits>cw or i==0, so fall thru to compress */
			}
			else /* st->top == ell <= st->L so we are at top tree node */
			{
				if (ell > 1 && st->bits[ell] == ConfT::md6_c * ConfT::md6_w)
					return md6_error_code::MD6_SUCCESS;
				/* else (ell==1 or bits>cw, so fall thru to compress */
			}
		}
		/* else (here ell < st->top so fall through to compress */
	}

	/* compress block at this level; result goes into C */
	/* first set z to 1 iff this is the very last compression */
	z = 0; if (final && (ell == st->top)) z = 1;
	if (md6_error_code err = md6_compress_block<ConfT>(C, st, ell, z); (int)err)
		return err;
	if (z == 1) /* save final chaining value in st->hashval */
	{
		memcpy(st->hashval, C, ConfT::md6_c * (ConfT::md6_w / 8));
		return md6_error_code::MD6_SUCCESS;
	}

	/* where should result go? To "next level" */
	next_level = (std::min)(ell + 1, st->L + 1);
	/* Start sequential mode with IV=0 at that level if necessary
	** (All that is needed is to set bits[next_level] to c*w,
	** since the bits themselves are already zeroed, either
	** initially, or at the end of md6_compress_block.)
	*/
	if (next_level == st->L + 1
		&& st->i_for_level[next_level] == 0
		&& st->bits[next_level] == 0)
		st->bits[next_level] = ConfT::md6_c * ConfT::md6_w;
	/* now copy C onto next level */
	memcpy((char*)st->B[next_level] + st->bits[next_level] / 8,
		C,
		ConfT::md6_c * (ConfT::md6_w / 8));
	st->bits[next_level] += ConfT::md6_c * ConfT::md6_w;
	if (next_level > st->top) st->top = next_level;

	return md6_process(st, next_level, final);
}

template <typename ConfT>
md6_error_code md6_update(md6_state_general<ConfT>* st, void const* vdata, uint64_t databitlen) noexcept
{
	unsigned char const* data = reinterpret_cast<unsigned char const*>(vdata);
	/* check that input values are sensible */
	if (!st) return md6_error_code::MD6_NULLSTATE;
	if (st->initialized == 0) return md6_error_code::MD6_STATENOTINIT;
	if (!data) return md6_error_code::MD6_NULLDATA;

	uint64_t j = 0; /* j = number of bits processed so far with this update */
	while (j < databitlen)
	{ /* handle input string in portions (portion_size in bits)
	  ** portion_size may be zero (level 1 data block might be full,
	  ** having size b*w bits) */
		uint64_t portion_size = (std::min)(databitlen - j,
			(uint64_t)(ConfT::md6_b * ConfT::md6_w - (st->bits[1])));

		if ((portion_size % 8 == 0) &&
			(st->bits[1] % 8 == 0) &&
			(j % 8 == 0))
		{ /* use mempy to handle easy, but most common, case */
			std::memcpy((char*)st->B[1] + st->bits[1] / 8,
				&(data[j / 8]),
				(size_t)(portion_size / 8));
		}
		else /* handle messy case where shifting is needed */
		{
			append_bits((unsigned char*)st->B[1], /* dest */
				st->bits[1],   /* dest current bit size */
				&(data[j / 8]),  /* src */
				portion_size); /* src size in bits  */
		}
		j += portion_size;
		st->bits[1] += portion_size;
		st->bits_processed += portion_size;

		/* compress level-1 block if it is now full
	   but we're not done yet */
		if (st->bits[1] == ConfT::md6_b * ConfT::md6_w && j < databitlen)
		{
			if (md6_error_code err = md6_process(st,
				1,    /* ell */
				0     /* final */
			); (int)err)
				return err;
		}
	} /* end of loop body handling input portion */
	return md6_error_code::MD6_SUCCESS;
}

template <typename ConfT>
void trim_hashval(md6_state_general<ConfT>* st) noexcept
{ /* trim hashval to desired length d bits by taking only last d bits */
  /* note that high-order bit of a byte is considered its *first* bit */
	int full_or_partial_bytes = (st->d + 7) / 8;
	int bits = st->d % 8;                 /* bits in partial byte */
	int i;

	/* move relevant bytes to the front */
	for (i = 0; i < full_or_partial_bytes; i++)
		st->hashval[i] = st->hashval[ConfT::md6_c * (ConfT::md6_w / 8) - full_or_partial_bytes + i];

	/* zero out following bytes */
	for (i = full_or_partial_bytes; i < ConfT::md6_c * (ConfT::md6_w / 8); i++)
		st->hashval[i] = 0;

	/* shift result left by (8-bits) bit positions, per byte, if needed */
	if (bits > 0)
	{
		for (i = 0; i < full_or_partial_bytes; i++)
		{
			st->hashval[i] = (st->hashval[i] << (8 - bits));
			if ((i + 1) < ConfT::md6_c * (ConfT::md6_w / 8))
				st->hashval[i] |= (st->hashval[i + 1] >> bits);
		}
	}
}

/* Do final processing to produce md6 hash value
** Input:
**     st              md6 state that has been accumulating message bits
**                     and/or intermediate results
** Output (by side effect on state):
**     hashval         If this is non-NULL, final hash value copied here.
**                     (NULL means don't copy.)  In any case, the hash
**                     value remains in st->hashval.
**     st->hashval     this is a 64-byte array; the first st->d
**                     bits of which will be the desired hash value
**                     (with high-order bits of a byte used first), and
**                     remaining bits set to zero (same as hashval)
**     st->hexhashval  this is a 129-byte array which contains the
**                     zero-terminated hexadecimal version of the hash
** Returns one of the following:
**     MD6_SUCCESS
**     MD6_NULLSTATE
**     MD6_STATENOTINIT
*/
template <typename ConfT>
md6_error_code md6_final(md6_state_general<ConfT>* st, unsigned char* hashval) noexcept
{
	int ell;

	/* check that input values are sensible */
	if (!st) return md6_error_code::MD6_NULLSTATE;
	if (!st->initialized) return md6_error_code::MD6_STATENOTINIT;

	/* md6_final was previously called */
	if (st->finalized == 1) return md6_error_code::MD6_SUCCESS;

	/* force any processing that needs doing */
	if (st->top == 1) ell = 1;
	else for (ell = 1; ell <= st->top; ell++)
		if (st->bits[ell] > 0) break;
	/* process starting at level ell, up to root */
	md6_error_code err = md6_process(st, ell, 1);
	if ((int)err) return err;

	/* md6_process has saved final chaining value in st->hashval */

	md6_reverse_little_endian((typename ConfT::md6_word*)st->hashval, ConfT::md6_c);

	/* 4/15/09: Following two lines were previously out of order, which
	**          caused errors depending on whether caller took hash output
	**          from  st->hashval (which was correct) or from
	**                hashval parameter (which was incorrect, since it
	**                                   missed getting "trimmed".)
	*/
	trim_hashval(st);
	if (hashval) memcpy(hashval, st->hashval, (st->d + 7) / 8);

	//md6_compute_hex_hashval(st);

	st->finalized = 1;
	return md6_error_code::MD6_SUCCESS;
}

inline const uint64_t md6_word_selector<64>::Q[15] = {
  0x7311c2812425cfa0ULL,
  0x6432286434aac8e7ULL,
  0xb60450e9ef68b7c1ULL,
  0xe8fb23908d9f06f1ULL,
  0xdd2e76cba691e5bfULL,
  0x0cd0d63b2c30bc41ULL,
  0x1f8ccf6823058f8aULL,
  0x54e5ed5b88e3775dULL,
  0x4ad12aae0a6d6031ULL,
  0x3e7f16bb88222e0dULL,
  0x8af8671d3fb50c2cULL,
  0x995ad1178bd25c31ULL,
  0xc878c1dd04c4b633ULL,
  0x3b72066c7a1552acULL,
  0x0d6f3522631effcbULL
};

inline const uint32_t md6_word_selector<32>::Q[30] = {
  0x7311c281UL, 0x2425cfa0UL,
  0x64322864UL, 0x34aac8e7UL,
  0xb60450e9UL, 0xef68b7c1UL,
  0xe8fb2390UL, 0x8d9f06f1UL,
  0xdd2e76cbUL, 0xa691e5bfUL,
  0x0cd0d63bUL, 0x2c30bc41UL,
  0x1f8ccf68UL, 0x23058f8aUL,
  0x54e5ed5bUL, 0x88e3775dUL,
  0x4ad12aaeUL, 0x0a6d6031UL,
  0x3e7f16bbUL, 0x88222e0dUL,
  0x8af8671dUL, 0x3fb50c2cUL,
  0x995ad117UL, 0x8bd25c31UL,
  0xc878c1ddUL, 0x04c4b633UL,
  0x3b72066cUL, 0x7a1552acUL,
  0x0d6f3522UL, 0x631effcbUL
};

inline const uint16_t md6_word_selector<16>::Q[60] = {
	0x7311, 0xc281, 0x2425, 0xcfa0,
	0x6432, 0x2864, 0x34aa, 0xc8e7,
	0xb604, 0x50e9, 0xef68, 0xb7c1,
	0xe8fb, 0x2390, 0x8d9f, 0x06f1,
	0xdd2e, 0x76cb, 0xa691, 0xe5bf,
	0x0cd0, 0xd63b, 0x2c30, 0xbc41,
	0x1f8c, 0xcf68, 0x2305, 0x8f8a,
	0x54e5, 0xed5b, 0x88e3, 0x775d,
	0x4ad1, 0x2aae, 0x0a6d, 0x6031,
	0x3e7f, 0x16bb, 0x8822, 0x2e0d,
	0x8af8, 0x671d, 0x3fb5, 0x0c2c,
	0x995a, 0xd117, 0x8bd2, 0x5c31,
	0xc878, 0xc1dd, 0x04c4, 0xb633,
	0x3b72, 0x066c, 0x7a15, 0x52ac,
	0x0d6f, 0x3522, 0x631e, 0xffcb
};

inline const uint8_t md6_word_selector<8>::Q[120] = {
	0x73, 0x11, 0xc2, 0x81, 0x24, 0x25, 0xcf, 0xa0,
	0x64, 0x32, 0x28, 0x64, 0x34, 0xaa, 0xc8, 0xe7,
	0xb6, 0x04, 0x50, 0xe9, 0xef, 0x68, 0xb7, 0xc1,
	0xe8, 0xfb, 0x23, 0x90, 0x8d, 0x9f, 0x06, 0xf1,
	0xdd, 0x2e, 0x76, 0xcb, 0xa6, 0x91, 0xe5, 0xbf,
	0x0c, 0xd0, 0xd6, 0x3b, 0x2c, 0x30, 0xbc, 0x41,
	0x1f, 0x8c, 0xcf, 0x68, 0x23, 0x05, 0x8f, 0x8a,
	0x54, 0xe5, 0xed, 0x5b, 0x88, 0xe3, 0x77, 0x5d,
	0x4a, 0xd1, 0x2a, 0xae, 0x0a, 0x6d, 0x60, 0x31,
	0x3e, 0x7f, 0x16, 0xbb, 0x88, 0x22, 0x2e, 0x0d,
	0x8a, 0xf8, 0x67, 0x1d, 0x3f, 0xb5, 0x0c, 0x2c,
	0x99, 0x5a, 0xd1, 0x17, 0x8b, 0xd2, 0x5c, 0x31,
	0xc8, 0x78, 0xc1, 0xdd, 0x04, 0xc4, 0xb6, 0x33,
	0x3b, 0x72, 0x06, 0x6c, 0x7a, 0x15, 0x52, 0xac,
	0x0d, 0x6f, 0x35, 0x22, 0x63, 0x1e, 0xff, 0xcb
};

/* ***************************************************************** */
#define loop_body(rs,ls,step)                                       \
      x = S;                                /* feedback constant     */ \
      x ^= A[i+step-t5];                    /* end-around feedback   */ \
      x ^= A[i+step-t0];                    /* linear feedback       */ \
      x ^= ( A[i+step-t1] & A[i+step-t2] ); /* first quadratic term  */ \
      x ^= ( A[i+step-t3] & A[i+step-t4] ); /* second quadratic term */ \
      x ^= (x >> rs);                       /* right-shift           */ \
      A[i+step] = x ^ (x << ls);            /* left-shift            */   
/* ***************************************************************** */

/* Main compression loop.
**
*/

/*
** Perform the md6 "main compression loop" on the array A.
** This is where most of the computation occurs; it is the "heart"
** of the md6 compression algorithm.
** Input:
**     A                  input array of length t+n already set up
**                        with input in the first n words.
**     r                  number of rounds to run (178); each is c steps
** Modifies:
**     A                  A[n..r*c+n-1] filled in.
*/
inline void md6_word_selector<64>::md6_main_compression_loop(md6_word* A, int r) noexcept
{
	md6_word S = S0, x;
	for (int j = 0, i = md6_n; j < r * md6_c; j += md6_c)
	{
		loop_body(10, 11, 0) //RL00
		loop_body(5, 24, 1)  //RL01
		loop_body(13, 9, 2)  //RL02
		loop_body(10, 16, 3) //RL03
		loop_body(11, 15, 4) //RL04
		loop_body(12, 9, 5) //RL05
		loop_body(2, 27, 6) //RL06
		loop_body(7, 15, 7) //RL07
		loop_body(14, 6, 8) //RL08
		loop_body(15, 2, 9) //RL09
		loop_body(7, 29, 10) //RL10
		loop_body(13, 8, 11) //RL11
		loop_body(11, 15, 12) //RL12
		loop_body(7, 5, 13) //RL13
		loop_body(6, 31, 14) //RL14
		loop_body(12, 9, 15) //RL15

		/* Advance round constant S to the next round constant. */
		S = (S << 1) ^ (S >> (md6_w - 1)) ^ (S & Smask);
		i += 16;
	}
}

inline void md6_word_selector<32>::md6_main_compression_loop(md6_word* A, int r) noexcept
{
	md6_word S = S0, x;
	for (int j = 0, i = md6_n; j < r * md6_c; j += md6_c)
	{
		loop_body(5, 4, 0)
		loop_body(3, 7, 1)
		loop_body(6, 7, 2)
		loop_body(5, 9, 3)
		loop_body(4, 13, 4)
		loop_body(6, 8, 5)
		loop_body(7, 4, 6)
		loop_body(3, 14, 7)
		loop_body(5, 7, 8)
		loop_body(6, 4, 9)
		loop_body(5, 8, 10)
		loop_body(5, 11, 11)
		loop_body(4, 5, 12)
		loop_body(6, 8, 13)
		loop_body(7, 2, 14)
		loop_body(5, 11, 15)

		/* Advance round constant S to the next round constant. */
		S = (S << 1) ^ (S >> (md6_w - 1)) ^ (S & Smask);
		i += 16;
	}
}

inline void md6_word_selector<16>::md6_main_compression_loop(md6_word* A, int r) noexcept
{
	md6_word S = S0, x;
	for (int j = 0, i = md6_n; j < r * md6_c; j += md6_c)
	{
		loop_body(5, 6, 0)
		loop_body(4, 7, 1)
		loop_body(3, 2, 2)
		loop_body(5, 4, 3)
		loop_body(7, 2, 4)
		loop_body(5, 6, 5)
		loop_body(5, 3, 6)
		loop_body(2, 7, 7)
		loop_body(4, 5, 8)
		loop_body(3, 7, 9)
		loop_body(4, 6, 10)
		loop_body(3, 5, 11)
		loop_body(4, 5, 12)
		loop_body(7, 6, 13)
		loop_body(7, 4, 14)
		loop_body(2, 3, 15)

		/* Advance round constant S to the next round constant. */
		S = (S << 1) ^ (S >> (md6_w - 1)) ^ (S & Smask);
		i += 16;
	}
}

inline void md6_word_selector<8>::md6_main_compression_loop(md6_word* A, int r) noexcept
{
	md6_word x, S;
	int i, j;

	/*
	** main computation loop for md6 compression
	*/
	S = S0;
	for (j = 0, i = md6_n; j < r * md6_c; j += md6_c)
	{
		loop_body(3, 2, 0)
		loop_body(3, 4, 1)
		loop_body(3, 2, 2)
		loop_body(4, 3, 3)
		loop_body(3, 2, 4)
		loop_body(3, 2, 5)
		loop_body(3, 2, 6)
		loop_body(3, 4, 7)
		loop_body(2, 3, 8)
		loop_body(2, 3, 9)
		loop_body(3, 2, 10)
		loop_body(2, 3, 11)
		loop_body(2, 3, 12)
		loop_body(3, 4, 13)
		loop_body(2, 3, 14)
		loop_body(3, 4, 15)

		/* Advance round constant S to the next round constant. */
		S = (S << 1) ^ (S >> (md6_w - 1)) ^ (S & Smask);
		i += 16;
	}
}

#undef loop_body

}
