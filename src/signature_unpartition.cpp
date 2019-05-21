#include "signature_unpartition.h"
#include <assert.h>
#include "h3.h"
#include "bitvector.h"

//============================================================================
// Implementation of UnpartitionedSignature
//============================================================================
//
UnpartitionedSignature::UnpartitionedSignature(int nBitLength,
                                   int nNumHashFcns,
                                   HashFunction_t hft)
: Signature(nBitLength, nNumHashFcns)
{
    assert(_hfcn);
    // Instantiate the hash functions
    //srand(SEED);   // We want all instances of the Signature to have same hash
                   // fcns
    for (int k = 0; k < _nNumHashFcns; k++)
    {
        if (_hfcn[k] != NULL) continue;
        switch (hft)
        {
        case eHF_H3:
            _hfcn[k] = new H3<HT, sizeof(AddrT)>(_nBitLength);
            break;
        default:
            fprintf(stderr,
                "Signature::Signature: invalid HashFunctionType\n");
            exit(-1);
            break;
        }
    }
}


UnpartitionedSignature::~UnpartitionedSignature(){}

void
UnpartitionedSignature::add(AddrT addr)
{
    HT hashRet;
    // Iterate over the hash fcns
    for (int k = 0; k < _nNumHashFcns; k++)
    {
        hashRet = (*_hfcn[k])( &addr, sizeof(AddrT) );

        set_bit(_bv, hashRet, true);
    }
}

bool
UnpartitionedSignature::isAddrPresent(AddrT addr)
{
    HT hashRet;
    bool bConflict = true;

    // Iterate over the hash fcns
    for (int k = 0; k < _nNumHashFcns && bConflict == true; k++)
    {
        hashRet = (*_hfcn[k])( &addr, sizeof(AddrT) );

        bConflict &= get_bit(_bv, hashRet);
    }

    return bConflict;
}


bool
UnpartitionedSignature::isAnySectionEmpty()
{
    return bits_are_false(_bv);
}


