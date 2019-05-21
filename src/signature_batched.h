#ifndef __SIGNATURE_BATCHED_H__
#define __SIGNATURE_BATCHED_H__

#include "signature.h"

#ifdef DEBUG
#include "bitvector.h"
#endif

/******************************************************************************
 * class BatchedSignature
 * Inherits from Signature class, differentiating the use of the bit vector
 *
 * The base Signature has k hash functions hashing an address into k
 * "Sections" of the bit vector. The resulting hashes index a bit in each
 * section to be asserted.
 *
 * A Batched Signature consists of miu Batches of Signatures, each with k
 * sections. One additional hash function indexes the batch into which an
 * address should be hashed.  The same k hash functions are used in each batch
 * to index the Bloom filter sections.
 *
 * This type of Signature is (hopefully) useful for Signature intersection,
 * as an address is guaranteed to only be present in one batch. Due to the
 * filtering effect of the first hash function, there are fewer elements
 * in each batch to be intersected.
 *****************************************************************************/

class BatchedSignature : public Signature
{
public:
    BatchedSignature(int nBitLength,
                                   int nNumHashFcns,
                                   int nNumBatches,
                                   HashFunction_t hft);
#ifdef DEBUG
    BatchedSignature(const bit_vector * bv, int nNumHashFcns, int nNumBatches);
#endif
    virtual ~BatchedSignature();


    virtual void add(AddrT addr);
    virtual bool isAddrPresent(AddrT addr);
    virtual bool isAnySectionEmpty();
private:
    inline HT getBatch(AddrT addr);

    static HashAbstract<HT, sizeof(AddrT)> * _hfcnFilter;
    const int _nNumBatches;
};
#endif
