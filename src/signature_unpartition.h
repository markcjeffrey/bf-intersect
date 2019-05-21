#ifndef __SIGNATURE_UNPARTITION_H__
#define __SIGNATURE_UNPARTITION_H__

#include "signature.h"

#ifdef DEBUG
#include "bitvector.h"
#endif

/******************************************************************************
 * class UnpartitionedSignature
 * Inherits from Signature class, differentiating the use of the bit vector
 *****************************************************************************/

class UnpartitionedSignature : public Signature
{
public:
    UnpartitionedSignature(int nBitLength,
                                   int nNumHashFcns,
                                   HashFunction_t hft);
    virtual ~UnpartitionedSignature();


    virtual void add(AddrT addr);
    virtual bool isAddrPresent(AddrT addr);
    virtual bool isAnySectionEmpty();
private:

};
#endif
