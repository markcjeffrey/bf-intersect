#include "hashpbxfullwin.h"

#include <assert.h>
#include <string.h>


//===========================================================================
// Implementation of HashPBXFullWindow

HashPBXFullWindow::
HashPBXFullWindow(int nSigBitLength,
                      int nNumHashFcns,
                      int nThisInst,
                      int nExtraSigParts,
                      int nExtraBitLength)
:

#if defined(DEBUG) || defined(OUTPUTSTATICHASH)
  HashPBX(nSigBitLength, nNumHashFcns, nThisInst,
            ((sizeof(AddrT) << 3) - NHIGHLOWSEPARATION
             - (nExtraSigParts-1)- CACHEBLOCKINDEX) / 2),
#else
  HashPBX(nSigBitLength, nNumHashFcns, nThisInst),
#endif
  _nExtraSigParts(nExtraSigParts),
  _nExtraBitLength(nExtraBitLength)
{
}

HashPBXFullWindow::~HashPBXFullWindow(void) {}


//-----------------------------------------------------------------------
// HashPBXFullWindow::operator()
// - Hashes a 32 bit address by xoring a "high" and "low" bit region of the
//   address by mirroring the high region.
// - For > 1 hash fcns, shifts the bit regions up by one in a sliding window
// - Uses NHIGHLOWSEPARATION as the buffer size from the MSB of the address
HT
HashPBXFullWindow::operator()(const void* data,
                            size_t size,
                            size_t offset) const
{

    AddrT highbits = 0x0, lowbits = 0x0, oldbits;
    HT retval = 0x0;

    assert(size == sizeof(AddrT)
        //&& (_nThisInst + CACHEBLOCKINDEX + 2*nBitSelectSize) < (sizeof(AddrT) * 8 )
          //&& ( NHIGHLOWSEPARATION + (_nNumHashFcns-1) + CACHEBLOCKINDEX >  ( _nThisInst) )
          );

    memcpy(&oldbits, data, size);

    // Load the low and high bits
    int rd = _nThisInst;
    int wr, rdlow, rdhi;
    int nSizeAddrSpace;

    if (true)
    {
        nSizeAddrSpace = ((sizeof(AddrT) << 3) -
                            NHIGHLOWSEPARATION - (_nNumHashFcns + _nExtraSigParts-1)- CACHEBLOCKINDEX) / 2;
    } else if (_nThisInst < _nNumHashFcns) {
        nSizeAddrSpace = ((sizeof(AddrT) << 3) -
                            NHIGHLOWSEPARATION - (_nNumHashFcns -1)- CACHEBLOCKINDEX) / 2;
    } else {
        nSizeAddrSpace = ((sizeof(AddrT) << 3) -
                            NHIGHLOWSEPARATION - (_nExtraSigParts - 1)- CACHEBLOCKINDEX) / 2;
    }

    for (wr = 0; wr < nSizeAddrSpace; wr++)
    {
        rdlow = rd + CACHEBLOCKINDEX;
        rdhi = (rd + nSizeAddrSpace)  + CACHEBLOCKINDEX;

        if (rdlow - wr > 0) {
            lowbits  |= ( (oldbits & (1 << rdlow)) >> (rdlow - wr) );
        } else {
            lowbits  |= ( (oldbits & (1 << rdlow)) << (wr - rdlow) );
        }

        if (rdhi - (nSizeAddrSpace - 1 - wr) > 0) {
            highbits |= ( (oldbits & (1 << rdhi)) >> (rdhi - (nSizeAddrSpace - 1 - wr)));
        } else {
            highbits |= ( (oldbits & (1 << rdhi)) << ((nSizeAddrSpace - 1 - wr) - rdhi));
        }

        rd++;

        #if defined(DEBUG) || defined(OUTPUTSTATICHASH)
            selected_lowbits[_nThisInst][wr] = rdlow;
            selected_hibits[_nThisInst][nSizeAddrSpace - 1 - wr] = rdhi;
        #endif
    }

    // Perform the PBX xor
    retval = (HT) (highbits ^ lowbits);

    if (_nThisInst < _nNumHashFcns) {
        retval = retval % (HT) (_nSigBitLength / _nNumHashFcns);
    } else {
        retval = retval % (HT) (_nExtraBitLength / _nExtraSigParts);
    }


    return retval;
}
