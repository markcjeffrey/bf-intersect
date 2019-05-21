#include "signature_batched.h"
#include <assert.h>
#include "h3.h"

//#define ISPOW2(x) ((x & (x-1))==0 && (x != 0))

/******************************************************************************
 * BatchedSignature Implementation
 *****************************************************************************/

HashAbstract<HT, sizeof(AddrT)> * BatchedSignature::_hfcnFilter = NULL;

BatchedSignature::BatchedSignature(int nBitLength,
                                   int nNumHashFcns,
                                   int nNumBatches,
                                   HashFunction_t hft)
: Signature(nBitLength, nNumHashFcns),
  _nNumBatches(nNumBatches)
{

    // k hash functions index k sections in each Bloom filter
    // There are nNumBatches of Bloom filters
    int nNumSections = _nNumHashFcns * _nNumBatches;

    // Ensure each section will have an equal number of bits
    assert( 0 == _nBitLength % (nNumSections));
    // Ensure each section is indexable
    assert( ISPOW2(_nBitLength / (nNumSections)) );

    //srand(SEED);   // We want all instances of the Signature to have same hash
                   // fcns


    // Instantiate the hash functions
    for (int k = 0; k < _nNumHashFcns; k++)
    {
        if (_hfcn[k] != NULL) continue;
        switch (hft)
        {
        case eHF_H3:
            _hfcn[k] = new H3<HT, sizeof(AddrT)>(_nBitLength / nNumSections);
            break;
       default:
            fprintf(stderr,
                    "BatchedSignature::BatchedSignature: only supports H3 Hash type\n");
            exit(-1);
            break;
        }
    }

    if (NULL == _hfcnFilter)
    {
        switch (hft)
        {
            case eHF_H3:
                //TODO verify that H3 performs mod?
                _hfcnFilter = new H3<HT, sizeof(AddrT)>(_nNumBatches);
                break;
            default:
                fprintf(stderr,
                        "BatchedSignature::BatchedSignature: only supports H3 Hash type\n");
                exit(-1);
                break;
        }
    }


    if (_hfcnFilter == NULL || _hfcn == NULL || _hfcn[0] == NULL)
    {
        fprintf(stderr,
            "BatchedSignature::BatchedSignature: null _hfcn array returned\n");
        exit(-1);
    }


}


BatchedSignature::~BatchedSignature()
{
    if (_hfcnFilter != NULL)
    {
        delete _hfcnFilter;
        _hfcnFilter = NULL;
    }
}


void
BatchedSignature::add(AddrT addr)
{
    const int nSigSectionLen = _nBitLength / (_nNumHashFcns * _nNumBatches);
    HT hashBit;

    // Determine which batch to add the address to
    const HT hashBatch = getBatch(addr);

    // Iterate over the hash functions to determine which bits to set in each
    // section
    for (int k = 0; k < _nNumHashFcns; k++)
    {
        hashBit = (*_hfcn[k])( &addr, sizeof(AddrT) );

        set_bit(_bv,
                hashBit + k * (nSigSectionLen)
                + hashBatch * (_nBitLength / _nNumBatches),
                true);
    }
}


bool
BatchedSignature::isAddrPresent(AddrT addr)
{
    bool bAddrPresent = true;
    HT hashBit;

    const int nBatchLen = _nBitLength / _nNumBatches;
    const int nSigSectionLen = nBatchLen / _nNumHashFcns;

    // Determine the batch to search
    const HT hashBatch = getBatch(addr);

    // Iterate over the hash functions to determine if the bits are set in
    // all sections
    for (int k = 0; k < _nNumHashFcns; k++)
    {
        hashBit = (*_hfcn[k])( &addr, sizeof(AddrT) );

        bAddrPresent &= get_bit(_bv,
                hashBit + k * (nSigSectionLen)
                + hashBatch * (nBatchLen));
    }

    return bAddrPresent;
}


bool
BatchedSignature::isAnySectionEmpty()
{
    const int nBatchLen = _nBitLength / _nNumBatches;
    const int nSectionLen = nBatchLen / _nNumHashFcns;

    // Empty in this context means that all bits in a section are zero
    // If any batch has no non-empty section, then return true
    // (there exists a set intersection)
    bool bNotEmptyBatch;

    // For each batch
    for (int b = 0; b < _nNumBatches; b++)
    {
        // Assume that the batch has no empty section until proven
        // otherwise
        bNotEmptyBatch = true;

        // For each section
        for (int k = 0; (k < _nNumHashFcns) && bNotEmptyBatch; k++)
        {
            // If a non-empty section is found, move to next batch
            if ( bits_are_false_range(_bv,
                        b * nBatchLen + k * nSectionLen,
                        b * nBatchLen +(k+1) * nSectionLen -1) )
            {
                bNotEmptyBatch = false;
            }
        }

        if (bNotEmptyBatch == true)
            return false;
    }


    return true;
}

inline
HT
BatchedSignature::getBatch(AddrT addr)
{
    return (*_hfcnFilter)( &addr, sizeof(AddrT) );
}

#ifdef DEBUG
BatchedSignature::BatchedSignature(const bit_vector * bv, int nNumHashFcns, int nNumBatches)
: Signature(bv->num_bits, nNumHashFcns),
  _nNumBatches(nNumBatches)
{

    _hfcnFilter = NULL;
    copy_bits(_bv, bv);
}
#endif
