/* =============================================================================
 *
 * sigintersect.cpp
 *
 * ===========================================================================*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <set>
#include <math.h>
#include "in_parameters.h"
#include "signature.h"
#include "signature_batched.h"
#include "signature_unpartition.h"

using namespace std;


/* =============================================================================
 * Typedefs
 * =============================================================================
 */
typedef set<unsigned int> addrSet_t;

/* =============================================================================
 * Global Parameters
 * =============================================================================
 */
unsigned long global_params[256];

/* =============================================================================
 * main
 * =============================================================================
 */
int
main(int argc, const char ** argv)
{
    // Initialization
    // ------------------------------------------
    parseArgs(argc, (char** const)argv);
    unsigned long sizeA = global_params[PARAM_SIZE_A];
    unsigned long sizeB = global_params[PARAM_SIZE_B];
    unsigned long sigLen = global_params[PARAM_LEN_SIG];
    unsigned long numHash = global_params[PARAM_NUM_HASH];
    unsigned long numBatches = global_params[PARAM_NUM_BATCH];
    //float delta = pow(10.0f, global_params[PARAM_POW_DELTA]);
    unsigned long desiredNumTrials = global_params[PARAM_NUM_TRIAL];
    bool doQueries = global_params[PARAM_DO_QUERIES];
    bool doUnpartition = global_params[PARAM_UNPARTITION];

    srand(time(NULL));

    // The Precise sets and Signatures
    addrSet_t setA, setB;
    Signature *sigA, *sigB;
    if (numBatches == 1) {
        if (doUnpartition){
            sigA = new UnpartitionedSignature(sigLen, numHash, eHF_H3);
            sigB = new UnpartitionedSignature(sigLen, numHash, eHF_H3);
        } else {
            sigA = new Signature(sigLen, numHash, eHF_H3);
            sigB = new Signature(sigLen, numHash, eHF_H3);
        }
    } else {
        sigA = new BatchedSignature(sigLen, numHash, numBatches, eHF_H3);
        sigB = new BatchedSignature(sigLen, numHash, numBatches, eHF_H3);
    }

    // False positive "rate"
    double fPrevFalsePos = 0, fCurFalsePos = 0;
    unsigned int nNumFalsePos = 0;
    unsigned int nNumTrials = 0;


    do {

        // Build a single set with |A|+|B| unique addresses
        // ------------------------------------------
        while ( setA.size() < (sizeA + sizeB) )
        {
            unsigned int tmp = random();
        //    printf("%d ", tmp);
            setA.insert(tmp);
        }
        //printf("\n");

        // Move the first |B| elements of A to set B
        addrSet_t::iterator it;
        for ( it = setA.begin(); (it != setA.end()) && (setB.size() < sizeB); it++ )
        {
            setB.insert(*it);
        }
        // Remove the elements of B from A so that they do not intersect
        setA.erase(setA.begin(), it);

        // Insert the elements of setA setB to signatures
        // ------------------------------------------
        assert(sizeA == setA.size() && sizeB == setB.size() );
        for ( it = setA.begin(); it != setA.end(); it++ ) {
            sigA->add((AddrT) *it);
        }
        if (!doQueries)
        {
            for ( it = setB.begin(); it != setB.end(); it++ ) {
                sigB->add((AddrT) *it);
            }
        }

        // Intersect the two sets.  Either by signature intersection
        // or by querying each element of B into A.
        // The result should be empty
        // since the sets were constructed with no intersection
#ifdef DEBUG
        if (desiredNumTrials == 1) {
            sigA->fprintBits(stdout); printf("\n");
            sigB->fprintBits(stdout); printf("\n");
        }
#endif
        // Query elements of B into A
        if (doQueries)
        {
            for ( it = setB.begin(); it != setB.end(); it++ )
            {
                if (true == sigA->isAddrPresent((AddrT)*it, 0))
                {
                    nNumFalsePos++;
                    break;
                }
            }

        }
        // Intersect the two Signatures
        else
        {
            sigA->intersect(sigB);
            if ( false == sigA->isAnySectionEmpty() )
            {
                nNumFalsePos++;
            }
        }
#ifdef DEBUG
        if (desiredNumTrials == 1) {
            sigA->fprintBits(stdout); printf("\n");
        }
#endif //DEBUG

        nNumTrials++;
        sigA->reset();
        sigB->reset();
        setA.clear();
        setB.clear();

        fPrevFalsePos = fCurFalsePos;
        fCurFalsePos = ( (double) nNumFalsePos / nNumTrials );

//    } while ( abs(fCurFalsePos - fPrevFalsePos) > delta );
    } while ( nNumTrials < desiredNumTrials );

    printf("==============Summary=======================\n");
    printf("fpr                              %.12f\n", fCurFalsePos);
    printf("num FP                             %9d\n", nNumFalsePos);
    printf("num Trials                         %9d\n", nNumTrials);
    printf("|A|                                %9ld\n", sizeA);
    printf("|B|                                %9ld\n", sizeB);


    if (sigA) delete sigA;
    if (sigB) delete sigB;

    return (0);
}



/* =============================================================================
 *
 * End of sigintersect.cpp
 *
 * =============================================================================
 */
