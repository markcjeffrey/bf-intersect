#ifndef __SIGNATURE_H__
#define __SIGNATURE_H__

#include "bitvector.h"
#include "abstracthash.h"


#define SEED 1000

// Define the hash return value
//#ifndef uint16_t
//#define uint16_t unsigned short
//#endif
//typedef uint16_t HT;

typedef enum HashFunction_enum {
    eHF_H3,
    eHF_PBX,
    eHF_PBXZebchukWin,
    eHF_PBXZebchukConv,
    eHF_PBXFullWin,
    eHF_PBXStatic,
} HashFunction_t;

typedef enum Signature_enum {
    eST_orig,
    eST_AddressTrie,
    eST_LocSet,
    eST_Select
} SignatureType_t;


class Signature
{
public:
    Signature();
    Signature(int nBitLength, int nNumHashFcns);
    Signature(int nBitLength, int nNumHashFcns, HashFunction_t hft);
    ~Signature();

    void copyFrom(Signature * src);
    // store the result of the intersection in this signature
    void intersect(Signature * sig);
    static void intersect(Signature * dst,
                          Signature * src1, Signature * src2);
    bool isEmpty();
    virtual bool isAnySectionEmpty();
    virtual void reset();
    int getBitLength() { return _nBitLength; }
    int getNumHashFcns() { return _nNumHashFcns; }
    void fprintBits(FILE * stream);

    // THE MORE IMPORTANT FUNCTIONS
    virtual void add(AddrT addr, AddrT pc);
    virtual bool isAddrPresent(AddrT addr, AddrT pc);
    virtual void add(AddrT addr);
    virtual bool isAddrPresent(AddrT addr);

protected:
    bit_vector * _bv;
    int _nBitLength;
    static HashAbstract<HT, sizeof(AddrT)> ** _hfcn;
    int _nNumHashFcns;
private:


    inline HT scaleHashResult(HT hashOut, HT hashMax, int desiredMax);
    void allocateHFCN(int hfcnBytes);

    int _nExtraBitLength;
    int _nExtraSigParts;

};

#endif
