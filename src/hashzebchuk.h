#ifndef __HASHZEBCHUK_H__
#define __HASHZEBCHUK_H__

#include "pbxhash.h"

// This hash function was inspired by a conversation with Jason Zebchuk
// Contrary to Notary's PBX, which uses an asymmetric, 'every other' selection
// of bits for the XOR, this hashing method XORs the low bits with a mirror
// of the high bits.

class HashPBXZebchukWindow : public HashPBX
{
public:
    HashPBXZebchukWindow(int nSigBitLength, int nNumHashFcns, int nThisInst);
    virtual ~HashPBXZebchukWindow();

    virtual HT operator()(const void* data,
                            size_t size,
                            size_t offset = 0) const;

};

class HashPBXZebchukConverge: public HashPBX
{
public:
    HashPBXZebchukConverge(int nSigBitLength, int nNumHashFcns, int nThisInst);
    virtual ~HashPBXZebchukConverge();

    virtual HT operator()(const void* data,
                            size_t size,
                            size_t offset = 0) const;

};

#endif //__HASHZEBCHUK_H__
