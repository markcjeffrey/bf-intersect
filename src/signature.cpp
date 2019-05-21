
#include "signature.h"
#include "h3.h"
#include "pbxhash.h"
#include "hashzebchuk.h"
#include "hashpbxfullwin.h"
//#include "hashstatic.h"

#include "in_parameters.h"

#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <string.h> // For memset


#ifndef min
#define min(a,b) ((a < b)?(a):(b))
#endif


///////////////////////////////////////////////////////////////////////////
/// Implementation of Signature
///////////////////////////////////////////////////////////////////////////

HashAbstract<HT, sizeof(AddrT)> ** Signature::_hfcn = NULL;

Signature::Signature()
: _bv(NULL),
  _nBitLength(0),
  _nNumHashFcns(0),
  _nExtraBitLength(0),
  _nExtraSigParts(0)
{
    _hfcn = NULL;
}

Signature::Signature(int nBitLength, int nNumHashFcns)
: _bv(new_bit_vector(nBitLength)),
  _nBitLength(nBitLength),
  _nNumHashFcns(nNumHashFcns)
{
    reset();

    // For the constructor with no specified hash type, assume simple,
    // power-of-two behaviour
    _nExtraSigParts = 0;
    _nExtraBitLength = 0;

    allocateHFCN((_nNumHashFcns + _nExtraSigParts) *
            sizeof(HashAbstract<HT, sizeof(AddrT)>*));

}

Signature::Signature(int nBitLength, int nNumHashFcns, HashFunction_t hft)
: _bv(new_bit_vector(nBitLength))
{
    reset();

//    if (false == cmdopt_global.bModNonPower2Sigs)
    if (true)
    {
        _nBitLength = 1 << ((int) log2(nBitLength));
    } else {
        _nBitLength = nBitLength;
    }

    _nExtraBitLength = nBitLength - _nBitLength;
    _nNumHashFcns = nNumHashFcns;



    if ((nBitLength/nNumHashFcns) % 9 == 0 && _nExtraBitLength > 0) {
        _nExtraSigParts = 1;
    } else {
        _nExtraSigParts = 0;
    }

    #if defined(DEBUG) || defined(OUTPUTSTATICHASH)
        HashPBX::nNumHashFcns_global = _nNumHashFcns + _nExtraSigParts;
     #endif


    assert(   //log2(_nExtraBitLength) % 1 == 0 &&
           _nBitLength % _nNumHashFcns == 0
           );


    allocateHFCN((_nNumHashFcns + _nExtraSigParts) *
            sizeof(HashAbstract<HT, sizeof(AddrT)>*));


    // Instantiate the hash functions
    // -------------------------------------
    //srand(SEED);   // We want all instances of the Signature to have same hash
                   // fcns

    int k;
    for (k = 0; (k < _nNumHashFcns); k++)
    {
        if (_hfcn[k] != NULL) continue;
        switch (hft)
        {
        case eHF_H3:
            _hfcn[k] = new H3<HT, sizeof(AddrT)>(_nBitLength / _nNumHashFcns);
            break;
        case eHF_PBX:
            _hfcn[k] = new HashPBX(_nBitLength, _nNumHashFcns, k);
            break;
        case eHF_PBXZebchukWin:
            _hfcn[k] = new HashPBXZebchukWindow(_nBitLength, _nNumHashFcns, k);
            break;
        case eHF_PBXZebchukConv:
            _hfcn[k] = new HashPBXZebchukConverge(_nBitLength, _nNumHashFcns, k);
            break;
        case eHF_PBXFullWin:
            _hfcn[k] = new HashPBXFullWindow(_nBitLength,
                                             _nNumHashFcns,
                                             k,
                                             _nExtraSigParts,
                                             _nExtraBitLength);
            break;
//        case eHF_PBXStatic:
//            _hfcn[k] = new HashStatic(_nBitLength, _nNumHashFcns, k);
//            break;
        default:
            fprintf(stderr,
                "Signature::Signature: invalid HashFunctionType\n");
            exit(-1);
            break;
        }
    }

    for ( ; (k < (_nNumHashFcns + _nExtraSigParts)); k++ )
    {
        if (_hfcn[k] != NULL) continue;
        switch (hft)
        {
        case eHF_H3:
            _hfcn[k] = new H3<HT, sizeof(AddrT)>(_nExtraBitLength / _nExtraSigParts);
            break;
        case eHF_PBX:
            _hfcn[k] = new HashPBX(_nExtraBitLength, _nExtraSigParts, k);
            break;
        case eHF_PBXZebchukWin:
            _hfcn[k] = new HashPBXZebchukWindow(_nExtraBitLength, _nExtraSigParts, k);
            break;
        case eHF_PBXZebchukConv:
            _hfcn[k] = new HashPBXZebchukConverge(_nExtraBitLength, _nExtraSigParts, k);
            break;
        case eHF_PBXFullWin:
            _hfcn[k] = new HashPBXFullWindow(_nBitLength,
                                             _nNumHashFcns,
                                             k,
                                             _nExtraSigParts,
                                             _nExtraBitLength);
            break;
//        case eHF_PBXStatic:
//            _hfcn[k] = new HashStatic(_nExtraBitLength, _nExtraSigParts, k);
//            break;
        default:
            fprintf(stderr,
                "Signature::Signature: invalid HashFunctionType\n");
            exit(-1);
            break;
        }
    }

    if (_hfcn == NULL || _hfcn[0] == NULL)
    {
        fprintf(stderr,
            "SignatureH3::SignatureH3: null _hfcn array returned\n");
        exit(-1);
    }
}

Signature::~Signature()
{
    if (_bv != NULL)
    {
        free_bit_vector(_bv);
        _bv = NULL;
    }

    if (_hfcn != NULL)
    {
        for (int k = 0; k < (_nNumHashFcns + _nExtraSigParts); k++)
        {
            if (NULL != _hfcn[k]) delete _hfcn[k];
            _hfcn[k] = NULL;
        }

        free( _hfcn );
        _hfcn = NULL;
    }
}

void
Signature::copyFrom(Signature * src)
{
    assert(src != NULL && src->_bv != NULL);

    if (_bv == NULL || _bv->num_bits != src->_bv->num_bits)
    {
        free_bit_vector(_bv);
        _bv = new_bit_vector(src->_bv->num_bits);
    }

    copy_bits(_bv, (const bit_vector *) src->_bv);
}


// store the result of the intersection in this signature
void
Signature::intersect(Signature * sig)
{
    and_bits(_bv, (const bit_vector *) sig->_bv);
}

void
Signature::intersect(Signature * dst,
                            Signature * src1,
                            Signature * src2)
{
    dst->copyFrom(src1);
    dst->intersect(src2);
}


bool
Signature::isEmpty()
{
    return bits_are_false(_bv);
}


bool
Signature::isAnySectionEmpty()
{
    int nSectionLen = _nBitLength / _nNumHashFcns;
    //assert( nSegID * nSigSectionLen < _nBitLength );
    for (int k = 0; k < _nNumHashFcns; k++)
    {
        if ( bits_are_false_range(_bv,
                                   k * nSectionLen,
                                   (k+1) * nSectionLen -1) )
        {
            return true;
        }
    }
    return false;
}

void
Signature::reset()
{
    set_all_bits(_bv, false);
}


void
Signature::add(AddrT addr, AddrT pc)
{
  add(addr);
}


void
Signature::add(AddrT addr)
{
    int nSigSectionLen = _nBitLength / _nNumHashFcns;
    int k;
    HT hashRet;
    // Iterate over the hash fcns
    for (k = 0; k < _nNumHashFcns; k++)
    {
        hashRet = (*_hfcn[k])( &addr, sizeof(AddrT) );

        set_bit(_bv, hashRet + k * (nSigSectionLen), true);
    }

    nSigSectionLen = (_nExtraSigParts == 0)?
                        0 : (_nExtraBitLength / _nExtraSigParts);
    for ( ; k < (_nNumHashFcns + _nExtraSigParts); k++)
    {
        hashRet = (*_hfcn[k])( &addr, sizeof(AddrT) );

        hashRet = hashRet % nSigSectionLen;

        set_bit(_bv,
                hashRet + (k-_nNumHashFcns)*nSigSectionLen + _nBitLength, true);
    }
}

bool Signature::isAddrPresent(AddrT addr, AddrT pc)
{
  return isAddrPresent(addr);
}

bool
Signature::isAddrPresent(AddrT addr)
{
    bool bConflict = true;

    int nSigSectionLen = _nBitLength / _nNumHashFcns;
    int k;
    HT hashRet;

    // Iterate over the hash fcns
    for (k = 0; k < _nNumHashFcns && bConflict == true; k++)
    {
        hashRet = (*_hfcn[k])( &addr, sizeof(AddrT) );

        bConflict &= get_bit(_bv, hashRet + k * (nSigSectionLen));
    }

    nSigSectionLen = (_nExtraSigParts == 0)?
                        0 : (_nExtraBitLength / _nExtraSigParts);
    for ( ; k < (_nNumHashFcns + _nExtraSigParts) && bConflict == true; k++)
    {
        hashRet = (*_hfcn[k])( &addr, sizeof(AddrT) );

        hashRet = hashRet % nSigSectionLen;

        bConflict &= get_bit(_bv, hashRet
                           + (k-_nNumHashFcns) * nSigSectionLen + _nBitLength);
    }


    return bConflict;
}


HT
Signature::scaleHashResult(HT hashOut, HT hashMax, int desiredMax)
{
#if 0
    HT tmp = (HT)(0.5f + ((float) desiredMax/hashMax) * hashOut);
    if (tmp < desiredMax)
        return tmp;
    else
        return desiredMax;
#else
    return min(desiredMax,
            (int)(0.5f + (((int)desiredMax * (int)hashOut) / (float)hashMax)));

#endif
}

void
Signature::allocateHFCN(int hfcnBytes)
{
    if (NULL == _hfcn) {
        _hfcn = (HashAbstract<HT, sizeof(AddrT)>**) malloc( hfcnBytes );

        // Set all pointers to NULL
        memset(_hfcn, NULL, hfcnBytes);
    }
}

void
Signature::fprintBits(FILE * stream)
{
    if (stream != NULL)
    {
        fprint_allbits(stream, _bv, " ");
    }
}
