/* =============================================================================
 * in_parameters.cpp
 * =============================================================================
 */

#include "in_parameters.h"
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#define ISPOW2(v) (v && !(v & (v - 1)))
// The above is from "bit hacks"
// http://graphics.stanford.edu/~seander/bithacks.html#DetermineIfPowerOf2

/* =============================================================================
 * parseArgs
 * =============================================================================
 */
void
parseArgs (long argc, char* const argv[])
{
    long i;
    long opt;

    // Initialize default arguments
    global_params[PARAM_SIZE_A] = PARAM_DEFAULT_SIZE_A;
    global_params[PARAM_SIZE_B] = PARAM_DEFAULT_SIZE_B;
    global_params[PARAM_LEN_SIG] = PARAM_DEFAULT_LEN_SIG;
    global_params[PARAM_NUM_HASH] = PARAM_DEFAULT_NUM_HASH;
    global_params[PARAM_POW_DELTA] = PARAM_DEFAULT_POW_DELTA;
    global_params[PARAM_NUM_TRIAL] = PARAM_DEFAULT_NUM_TRIAL;
    global_params[PARAM_DO_QUERIES] = PARAM_DEFAULT_DO_QUERIES;
    global_params[PARAM_NUM_BATCH] = PARAM_DEFAULT_NUM_BATCH;
    global_params[PARAM_UNPARTITION] = PARAM_DEFAULT_UNPARTITION;

    opterr = 0;

    while ((opt = getopt(argc, argv, "A:B:h:l:t:qb:u")) != -1) {
        switch (opt) {
            case 'A':
            case 'B':
            case 'd':
            case 'h':
            case 'l':
            case 't':
            case 'b':
                global_params[(unsigned char)opt] = atol(optarg);
                break;
            case 'q':
            case 'u':
                global_params[(unsigned char)opt] = 1;
                break;
            case '?':
            default:
                opterr++;
                break;
        }
    }

    if (
         (global_params[PARAM_LEN_SIG] % global_params[PARAM_NUM_HASH] != 0)
         || (global_params[PARAM_NUM_BATCH] <= 0)
         || !(ISPOW2(global_params[PARAM_LEN_SIG]
                 / (global_params[PARAM_NUM_HASH] * global_params[PARAM_NUM_BATCH])))

       )
    {
        opterr++;
    }

    for (i = optind; i < argc; i++) {
        fprintf(stderr, "Non-option argument: %s\n", argv[i]);
        opterr++;
    }

    if (opterr) {
        displayUsage(argv[0]);
        exit(-1);
    }
}

/* =============================================================================
 * displayUsage
 * =============================================================================
 */
void
displayUsage (const char* appName)
{
    printf("Usage: %s [options]\n", appName);
    puts("\nOptions:                              (defaults)\n");
    printf("    A <UINT>   cardinality of set [A] (%i)\n", PARAM_DEFAULT_SIZE_A);
    printf("    B <UINT>   cardinality of set [B] (%i)\n", PARAM_DEFAULT_SIZE_B);
//    printf("    d <UINT>   power of 10 of [d]elta (%i)\n", PARAM_DEFAULT_POW_DELTA);
    printf("    b <UINT>   number of [b]atch sigs (%i)\n", PARAM_DEFAULT_NUM_BATCH);
    printf("    h <UINT>   number of [h]ash fcns  (%i)\n", PARAM_DEFAULT_NUM_HASH);
    printf("    l <UINT>   [l]ength of Signature  (%i)\n", PARAM_DEFAULT_LEN_SIG);
    printf("    q          perform batch [q]ueries    \n");
    printf("    t <UINT>   number of [t]rials     (%i)\n", PARAM_DEFAULT_NUM_TRIAL);
    printf("    u          use unpartitioned filter   \n");
    printf("Quotient of l by h must be a power of 2\n");
}

