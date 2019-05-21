/* =============================================================================
 * in_parameters.h
 * =============================================================================
 */

#ifndef __IN_PARAMETERS_H__
#define __IN_PARAMETERS_H__

enum param_types {
    // Cardinality of sets A and B
    PARAM_SIZE_A = (unsigned char)'A',
    PARAM_SIZE_B = (unsigned char)'B',
    // Length of the Signature (in bits)
    PARAM_LEN_SIG = (unsigned char) 'l',
    PARAM_NUM_HASH = (unsigned char) 'h',
    PARAM_POW_DELTA = (unsigned char) 'd',
    // Number of set intersection trials
    PARAM_NUM_TRIAL = (unsigned char) 't',
    // Perform set intersection by individually querying each
    // element of B into BF(A)
    PARAM_DO_QUERIES = (unsigned char) 'q',
    // Perform set intersection using the "Batched Signature"
    // Integer following this parameter indicates the # of batches
    // (1 batch implies use of regular Signature)
    PARAM_NUM_BATCH = (unsigned char) 'b',
    // Use Unpartitioned (Standard) Bloom filters
    PARAM_UNPARTITION = (unsigned char) 'u',
};

enum param_defaults {
    PARAM_DEFAULT_SIZE_A = 1 << 6,
    PARAM_DEFAULT_SIZE_B = 1 << 6,
    PARAM_DEFAULT_LEN_SIG = 1 << 10,
    PARAM_DEFAULT_NUM_HASH = 1 << 2,
    PARAM_DEFAULT_POW_DELTA = 5,
    PARAM_DEFAULT_NUM_TRIAL = 1000,
    PARAM_DEFAULT_DO_QUERIES = 0,
    PARAM_DEFAULT_NUM_BATCH = 1,
    PARAM_DEFAULT_UNPARTITION = 0,
};

void displayUsage (const char* appName);
void parseArgs (long argc, char* const argv[]);

extern unsigned long global_params[256];

#endif
