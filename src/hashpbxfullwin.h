#ifndef __HASH_PBX_FULL_WIN_H__
#define __HASH_PBX_FULL_WIN_H__

#include "pbxhash.h"

class HashPBXFullWindow : public HashPBX
{
public:
    HashPBXFullWindow(int nSigBitLength,
                      int nNumHashFcns,
                      int nThisInst,
                      int nExtraSigParts,
                      int nExtraBitLength);
    virtual ~HashPBXFullWindow();

    virtual HT operator()(const void* data,
                            size_t size,
                            size_t offset = 0) const;

 private:
    int _nExtraSigParts;
    int _nExtraBitLength;

};

#endif // __HASH_PBX_FULL_WIN_H__
