
#include "pbxhash.h"
#include <assert.h>
#include <string.h> // for memcpy
#include <math.h>
#include <stdio.h> // for error messages

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif


//-------------------------------------------------------------
// Implementation

int HashPBX::nInstanceCount = 0;
int nBitSelectSize = 0;
int HashPBX::NHIGHLOWSEPARATION = 0;


#if defined(DEBUG) || defined(OUTPUTSTATICHASH)
int HashPBX::nNumHashFcns_global = 0;
int ** HashPBX::selected_lowbits = NULL;
int ** HashPBX::selected_hibits = NULL;
#endif



HashPBX::HashPBX(int nSigBitLength, int nNumHashFcns, int nThisInst, int nBits)
: _nThisInst( nThisInst ),//(nInstanceCount++) % nNumHashFcns),
  _nNumHashFcns(nNumHashFcns),
  _nSigBitLength(nSigBitLength),
  _nBitSelectSize( (int) (ceil(log2(nSigBitLength / nNumHashFcns))))
{

    int k = ((_nBitSelectSize + 1) >> 1) << 1;

    if (!((CACHEBLOCKINDEX + 2*k + 1 + NHIGHLOWSEPARATION )
           < (int)(sizeof(AddrT) * 8 ))
       )
    {
        fprintf(stderr, "HashPBX::HashPBX() Invalid combination of SigBitLen %d"
                        " and nHashFcns %d\n"
                        "\tsince (%d + %d + 1 + %d < %d) is not satisfied. "
                        "Exit process\n",
                        _nSigBitLength, _nNumHashFcns, CACHEBLOCKINDEX, 2*k,
                        NHIGHLOWSEPARATION, (int) (sizeof(AddrT) * 8 ) );
        exit(0);
    }


    nBitSelectSize = max( max(_nBitSelectSize,nBits), nBitSelectSize);

#if defined(DEBUG) || defined(OUTPUTSTATICHASH)
    //nNumHashFcns_global = _nNumHashFcns;

    if (selected_lowbits == NULL)
    {
        selected_lowbits = (int**)malloc(sizeof(int*)* nNumHashFcns_global);
        for (int i = 0; i < nNumHashFcns_global; i++)
            selected_lowbits[i] = new int[nBitSelectSize];
    }
    if (selected_hibits == NULL)
    {
        selected_hibits = (int**)malloc(sizeof(int*)* nNumHashFcns_global);
        for (int i = 0; i < nNumHashFcns_global; i++)
            selected_hibits[i] = new int[nBitSelectSize];
    }
#endif
}

HashPBX::~HashPBX()
{
#if defined(DEBUG) || defined(OUTPUTSTATICHASH)
    if (selected_lowbits != NULL)
    {
    #ifdef DEBUG
        fprintXorSelectedBits(stdout);
    #endif
    #ifdef OUTPUTSTATICHASH

        // Check if the file already exists
        FILE * fpv = NULL, * fp;
        char overwrite = 'a';

        if (overwrite != 'n')
        {
            fp = fopen(OUTPUTSTATICFILE, "w");
            fpv = fopen(OUTPUTSTATICFILE ".v", "w");

            fprintCfile(fp);
            fprintVfile(fpv);
            fclose(fp);
            fclose(fpv);
        }
    #endif

        for (int i = 0; i < nNumHashFcns_global; i++)
            delete [] selected_lowbits[i];
        free(selected_lowbits);
        selected_lowbits = NULL;
    }
    if (selected_hibits != NULL)
    {
        for (int i = 0; i < nNumHashFcns_global; i++)
            delete [] selected_hibits[i];
        free(selected_hibits);

        selected_hibits = NULL;
    }
#endif
}


HT
HashPBX::operator()(const void* data,
                            size_t size,
                            size_t offset) const
{
    AddrT highbits = 0x0, lowbits = 0x0, oldbits;

    int k = ((_nBitSelectSize + 1) >> 1) << 1;

    HT retval = 0x0;

    assert(size == sizeof(AddrT)
             && (CACHEBLOCKINDEX + 2*k + 1 + NHIGHLOWSEPARATION ) < (sizeof(AddrT) * 8 )
          );

    memcpy(&oldbits, data, size);

    // Load the low and high bits
    int start = CACHEBLOCKINDEX;
    int rd = _nThisInst, wr;

    for (wr = 0; wr < _nBitSelectSize; wr++)
    {
        int modlow = (rd % (k + 1)) + start;
        int modhi = (rd % (k) ) + k + 1 + start + NHIGHLOWSEPARATION;

        if (modlow - wr > 0) {
            lowbits  |= ( (oldbits & (1 << modlow)) >> (modlow - wr) );
        } else {
            lowbits  |= ( (oldbits & (1 << modlow)) << (wr - modlow) );
        }

        // Note (modhi - wr) is always > 0
        highbits |= ( (oldbits & (1 << (modhi))) >> (modhi - wr) );

        rd += _nNumHashFcns;

        #if defined(DEBUG) || defined(OUTPUTSTATICHASH)
            selected_lowbits[_nThisInst][wr] = modlow;
            selected_hibits[_nThisInst][wr] = modhi;
        #endif
    }

    // Perform the PBX xor
    retval = (HT) (highbits ^ lowbits);

    return retval % (HT) (_nSigBitLength / _nNumHashFcns);
}

#ifdef DEBUG
void HashPBX::fprintXorSelectedBits(FILE * stream)
{
    for (int hash = 0; hash < nNumHashFcns_global; hash++)
    {
        fprintf(stream, "HashPBX Xor Pairs\n"
                        "Low[%d]   ", hash
               );
        for (int i = nBitSelectSize - 1; i >= 0; i--)
        {
            fprintf(stream, "%3d ", HashPBX::selected_lowbits[hash][i]);
        }
        fprintf(stream, "\n"
                        "High[%d]  ", hash
               );
        for (int i = nBitSelectSize - 1; i >= 0; i--)
        {
            fprintf(stream, "%3d ", HashPBX::selected_hibits[hash][i]);
        }
        fprintf(stream, "\n");
    }
}
#endif


#ifdef OUTPUTSTATICHASH
void
HashPBX::fprintCfile(FILE * stream)
{
    // int sat_get_size() {
    fprintf(stream,
        "\n"
        "#define B(a,s) ((a>>s)&1)\n"
        "#define XORB(a,b1,b2) (B(a,b1)^B(a,b2))\n"
        "\n"
        "int sbf_get_sig_bit_len()\n"
        "{\n"
        "    return %d;\n"
        "}\n"
        "\n"
        "int sbf_get_num_hashes()\n"
        "{\n"
        "    return %d;\n"
        "}\n\n",
        _nSigBitLength,
        HashPBX::nNumHashFcns_global
    );


    // static int sat_loadsig(unsigned a) {
    fprintf(stream,
        "static void sbf_gethashes(unsigned int a, int * resultHash, int hashlen) \n"
        "{\n"
        //"    if (hashlen != sbf_get_num_hashes()) return;"
    );

    for (int hash = 0; hash < nNumHashFcns_global; hash++)
    {
        // The top of the line, result[0] =
        fprintf(stream, "   resultHash[%d] = ", hash);

        // Add the mask
        fprintf(stream, " ( 0x0 \n");


        for (int bit = 0; bit < nBitSelectSize; bit++)
        {
            fprintf(stream,
                //"       | (B(a,%d) ^ B(a,%d) << %d)\n",
                "       | (XORB(a,%d,%d) << %d)\n",
                HashPBX::selected_lowbits[hash][bit],
                HashPBX::selected_hibits[hash][bit],
                bit
            );
        }
        fprintf(stream, "       );\n\n");

    }



    fprintf(stream, "    return;\n"
                    "}\n");
}

void
HashPBX::fprintVfile(FILE * stream)
{
    fprintf(stream,
    "function [%d:0] MYSIG;\n"
    "input [25:0] a;\n",
    _nSigBitLength - 1
    );

    // wire[7:0] hash0;
    for (int hash = 0; hash < HashPBX::nNumHashFcns_global; hash++)
    {
        fprintf(stream,
        "reg[%d:0] hash%d;\n",
        nBitSelectSize-1,
        hash
        );
    }

    fprintf(stream,
    "begin\n"
    "    MYSIG = %d'b0;\n\n",
    HashPBX::nNumHashFcns_global * (1 << (nBitSelectSize)) - 1
    );

    for (int hash = 0; hash < HashPBX::nNumHashFcns_global; hash++)
    {
        fprintf(stream,
        "    hash%d = {\n", hash
        );

        int bit;
        for (bit = 0; bit < nBitSelectSize -1; bit++)
        {
            fprintf(stream,
                //"       | (B(a,%d) ^ B(a,%d) << %d)\n",
                "       a[%d]^a[%d],\n",
                HashPBX::selected_lowbits[hash][bit],
                HashPBX::selected_hibits[hash][bit]
            );
        }

        fprintf(stream,
        "       a[%d]^a[%d]\n",
                HashPBX::selected_lowbits[hash][bit],
                HashPBX::selected_hibits[hash][bit]
        );

        fprintf(stream,
        "    };\n\n"
        );
    }

    // Determine the mask in case more bits are hashed than need be:
    // Perform log2 of _nSigBitLength/HashPBX::nNumHashFcns_global
    int nMaxReqBits = sizeof(_nSigBitLength) * 8 - 1;
    while ( (( (_nSigBitLength/HashPBX::nNumHashFcns_global) >> nMaxReqBits) & 0x1) == 0)
    {
        nMaxReqBits--;
    }

    for (int hash = 0; hash < HashPBX::nNumHashFcns_global; hash++)
    {
        fprintf(stream,
        "    MYSIG[%d + hash%d[%d:0]] = 1'b1;\n",
        hash * (_nSigBitLength / HashPBX::nNumHashFcns_global),
        hash,
        nMaxReqBits - 1
        );
    }

    fprintf(stream,
    "\nend\n"
    "endfunction\n"
    );

    return;
}

#endif
