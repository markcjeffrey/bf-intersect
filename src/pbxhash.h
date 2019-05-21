#ifndef __PBXHASH_H__
#define __PBXHASH_H__

#include "abstracthash.h"

#define CACHEBLOCKINDEX 2
#define IGNORETOPBITS 6
#define OUTPUTSTATICFILE "staticbfh.c"

// For DEBUGGING
#if defined(DEBUG) || defined(OUTPUTSTATICHASH)
#include <stdio.h>
#endif


class HashPBX : public HashAbstract<HT, sizeof(AddrT)>
{
public:
    HashPBX(int nSigBitLength, int nNumHashFcns, int nThisInst, int nBits = -1);
    virtual ~HashPBX();
    virtual HT operator()(const void* data,
                            size_t size,
                            size_t offset = 0) const;

    static int NHIGHLOWSEPARATION;
    static int nInstanceCount;

protected:
    const int _nThisInst;
    const int _nNumHashFcns;
    const int _nSigBitLength;
    const int _nBitSelectSize;

public:

// For debugging or creating static hash files
#ifdef DEBUG
    static void fprintXorSelectedBits(FILE * stream);
#endif
#ifdef OUTPUTSTATICHASH
    void fprintCfile(FILE * stream);
    void fprintVfile(FILE * stream);
#endif

#if defined(DEBUG) || defined(OUTPUTSTATICHASH)
    static int ** selected_lowbits;
    static int ** selected_hibits;
    static int nNumHashFcns_global;
#endif
};


#endif // __PBXHASH_H__
