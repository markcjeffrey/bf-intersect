// Copied from:
//     http://www.nersc.gov/~scottc/misc/docs/bro.1.0.3/H3_8h-source.html
// $Id: H3.h,v 1.2 2005/09/07 17:34:57 vern Exp $

// H3 hash function family
// C++ template implementation by Ken Keys (kkeys@caida.org)
//
// Usage:
//    #include <h3.h>
//    const H3<T, N> h;
//    T hashval = h(data, size [, offset]);
// (T) is the type to be returned by the hash function; must be an integral
//     type, e.g. uint32_t.
// (N) is the size of the data in bytes (if data is a struct, beware of
//     padding).
// The hash function hashes the (size) bytes of the data pointed to by (data),
//     starting at (offset).  Note: offset affects the hash value, so
//     h(data, size, offset) is not the same as h(data+offset, size, 0).
//     Typically (size) is N and (offset) is 0, but other values can be used to
//     hash a substring of the data.  Hashes of substrings can be bitwise-XOR'ed
//     together to get the same result as hashing the full string.
// Any number of hash functions can be created by creating new instances of H3,
//     with the same or different template parameters.  The hash function is
//     randomly generated using random(); you must call srandom() before the
//     H3 constructor if you wish to seed it.

#ifndef H3_H
#define H3_H

#include <stdlib.h>  // for rand()
#include "abstracthash.h"

#if __GNUC__ < 3
    #define __builtin_expect(expr, val) (expr)
    #define expect(expr, val) (expr)
#else
    #define expect(expr, val) __builtin_expect((long)(expr), val)
#endif
#define __likely(expr) expect((expr), 1)

#ifndef ISPOW2
#define ISPOW2(x) (x && !(x & (x-1)))
#endif



template<class T, int N>
class H3 : public HashAbstract<T,N>
{
    T byte_lookup[N][256];
    const T _nMaxReturnVal;
    const T _mask;
public:
    H3(T nMaxReturnVal);
    ~H3() { /*free(byte_lookup); */}
    virtual T operator()(const void* data, size_t size, size_t offset = 0) const
    {
        const unsigned char *p = static_cast<const unsigned char*>(data);
        T result = 0;

        // loop optmized with Duff's Device
        // Original:
        // do {
        //     result ^= byte_lookup[offset++][*p++];
        // } while (--size > 0 )

        register unsigned n = (size + 7) / 8;
        switch (size % 8) {
        case 0: do { result ^= byte_lookup[offset++][*p++];
        case 7:      result ^= byte_lookup[offset++][*p++];
        case 6:      result ^= byte_lookup[offset++][*p++];
        case 5:      result ^= byte_lookup[offset++][*p++];
        case 4:      result ^= byte_lookup[offset++][*p++];
        case 3:      result ^= byte_lookup[offset++][*p++];
        case 2:      result ^= byte_lookup[offset++][*p++];
        case 1:      result ^= byte_lookup[offset++][*p++];
                } while (--n > 0);
        }

        if (! _mask ) {
            return result % _nMaxReturnVal;
        } else {
            return result & _mask;
        }
    }
};

template<class T, int N>
H3<T,N>::H3(T nMaxReturnVal)
: _nMaxReturnVal(nMaxReturnVal),
  _mask( (ISPOW2(_nMaxReturnVal)) ? (_nMaxReturnVal -1) : (0) )
{

    T bit_lookup[N * 8];

    for (size_t bit = 0; bit < N * 8; bit++) {
        bit_lookup[bit] = 0;
        for (size_t i = 0; i < sizeof(T)/2; i++) {
            // assume random() returns at least 16 random bits
            bit_lookup[bit] = (bit_lookup[bit] << 16) | (rand() & 0xFFFF);
        }
    }

    for (size_t byte = 0; byte < N; byte++) {
        for (unsigned val = 0; val < 256; val++) {
            byte_lookup[byte][val] = 0;
            for (size_t bit = 0; bit < 8; bit++) {
                if (val & (1 << bit))
                    byte_lookup[byte][val] ^= bit_lookup[byte*8+bit];
            }
        }
    }
}

#endif //H3_H
