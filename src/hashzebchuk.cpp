
#include "hashzebchuk.h"

#include <assert.h>
#include <string.h>

#define PARALLELCONVERGE
#undef PARALLELCONVERGE

//===========================================================================
// Implementation of HashPBXZebchukWindow

HashPBXZebchukWindow::
HashPBXZebchukWindow(int nSigBitLength, int nNumHashFcns, int nThisInst)
: HashPBX(nSigBitLength, nNumHashFcns, nThisInst)
{
}

HashPBXZebchukWindow::~HashPBXZebchukWindow(void) {}


//-----------------------------------------------------------------------
// HashPBXZebchukWindow::operator()
// - Hashes a 32 bit address by xoring a "high" and "low" bit region of the
//   address by mirroring the high region.
// - For > 1 hash fcns, shifts the bit regions up by one in a sliding window
// - Uses NHIGHLOWSEPARATION as the buffer size from the MSB of the address
HT
HashPBXZebchukWindow::operator()(const void* data,
                            size_t size,
                            size_t offset) const
{

    AddrT highbits = 0x0, lowbits = 0x0, oldbits;
    HT retval = 0x0;

    assert(size == sizeof(AddrT)
        //&& (_nThisInst + CACHEBLOCKINDEX + 2*nBitSelectSize) < (sizeof(AddrT) * 8 )
          );

    memcpy(&oldbits, data, size);

    // Load the low and high bits
    int rd = _nThisInst;
    int wr, rdlow, rdhi;
    int nSizeAddrSpace = ((sizeof(AddrT) << 3) -
                            NHIGHLOWSEPARATION - CACHEBLOCKINDEX);

    for (wr = 0; wr < _nBitSelectSize; wr++)
    {
        rdlow = rd % nSizeAddrSpace + CACHEBLOCKINDEX;
        rdhi = (rd + _nBitSelectSize) % nSizeAddrSpace + CACHEBLOCKINDEX;

        if (rdlow - wr > 0) {
            lowbits  |= ( (oldbits & (1 << rdlow)) >> (rdlow - wr) );
        } else {
            lowbits  |= ( (oldbits & (1 << rdlow)) << (wr - rdlow) );
        }

        if (rdhi - (_nBitSelectSize - wr) > 0) {
            highbits |= ( (oldbits & (1 << rdhi)) >> (rdhi - (_nBitSelectSize - 1 - wr)));
        } else {
            highbits |= ( (oldbits & (1 << rdhi)) << ((_nBitSelectSize - 1 - wr) - rdhi));
        }

        rd++;

        #if defined(DEBUG) || defined(OUTPUTSTATICHASH)
            selected_lowbits[_nThisInst][wr] = rdlow;
            selected_hibits[_nThisInst][_nBitSelectSize - 1 - wr] = rdhi;
        #endif
    }

    // Perform the PBX xor
    retval = (HT) (highbits ^ lowbits);

    return retval % (HT) (_nSigBitLength / _nNumHashFcns);
}


//===========================================================================
// Implementation of HashPBXZebchukWindow

HashPBXZebchukConverge::
HashPBXZebchukConverge(int nSigBitLength, int nNumHashFcns, int nThisInst)
: HashPBX(nSigBitLength, nNumHashFcns, nThisInst)
{
}

HashPBXZebchukConverge::~HashPBXZebchukConverge(void) {}


//-----------------------------------------------------------------------
// HashPBXZebchukConverge::operator()
// - Hashes a 32 bit address by xoring a "high" and "low" bit region of the
//   address by mirroring the high region.
// - For > 1 hash fcns, shifts the bit toward one another...they converge
// - Uses NHIGHLOWSEPARATION as the buffer size from the MSB of the address
HT
HashPBXZebchukConverge::operator()(const void* data,
                            size_t size,
                            size_t offset) const
{

    AddrT highbits = 0x0, lowbits = 0x0, oldbits;
    HT retval = 0x0;

    assert(size == sizeof(AddrT)
        //&& (_nThisInst + CACHEBLOCKINDEX + 2*nBitSelectSize) < (sizeof(AddrT) * 8 )
          );

    memcpy(&oldbits, data, size);

    // Load the low and high bits
#ifdef PARALLELCONVERGE
    int rd = _nThisInst;
#else
    int rd = _nThisInst * _nBitSelectSize;
#endif
    int wr, rdlow, rdhi;
    int nSizeAddrSpace = ((sizeof(AddrT) * 8) -
            #ifdef PARALLELCONVERGE
                            (_nNumHashFcns - 1) -
            #endif
                            NHIGHLOWSEPARATION -CACHEBLOCKINDEX); // / 2;

    for (wr = 0; wr < _nBitSelectSize; wr++)
    {
        rdlow = rd % nSizeAddrSpace + CACHEBLOCKINDEX;
        //rdhi = ((sizeof(AddrT) << 3) - 1 - NHIGHLOWSEPARATION - (_nNumHashFcns - 1))
#ifndef PARALLELCONVERGE
        rdhi = ((sizeof(AddrT) << 3) - NHIGHLOWSEPARATION - 1)
                  - (rd % nSizeAddrSpace);
#else
        rdhi =  ( (sizeof(AddrT) << 3) - NHIGHLOWSEPARATION - _nBitSelectSize - (_nNumHashFcns - 1))
                   + rd % nSizeAddrSpace;
#endif


        if (rdlow - wr > 0) {
            lowbits  |= ( (oldbits & (1 << rdlow)) >> (rdlow - wr) );
        } else {
            lowbits  |= ( (oldbits & (1 << rdlow)) << (wr - rdlow) );
        }

#ifdef PARALLELCONVERGE

        if (rdhi - (_nBitSelectSize - wr) > 0) {
            highbits |= ( (oldbits & (1 << rdhi)) >> (rdhi - (_nBitSelectSize - 1 - wr)));
        } else {
            highbits |= ( (oldbits & (1 << rdhi)) << ((_nBitSelectSize - 1 - wr) - rdhi));
        }

        rd++;

        #if defined(DEBUG) || defined(OUTPUTSTATICHASH)
            selected_lowbits[_nThisInst][wr] = rdlow;
            selected_hibits[_nThisInst][_nBitSelectSize - 1 - wr] = rdhi;
        #endif
#else
        if (rdhi - wr > 0) {
            highbits |= ( (oldbits & (1 << rdhi)) >> (rdhi - wr));
        } else {
            highbits |= ( (oldbits & (1 << rdhi)) << (wr - rdhi));
        }

        rd++;

        #if defined(DEBUG) || defined(OUTPUTSTATICHASH)
            selected_lowbits[_nThisInst][wr] = rdlow;
            selected_hibits[_nThisInst][wr] = rdhi;
        #endif
#endif
    }

    // Perform the PBX xor
    retval = (HT) (highbits ^ lowbits);

    return retval % (HT) (_nSigBitLength / _nNumHashFcns);
}
