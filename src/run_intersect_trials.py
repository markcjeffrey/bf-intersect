#!/usr/bin/python -u

##############################################################################
# 2011 Jan 17
# Mark Jeffrey
# This script builds and runs the local sig_intersect program, collecting the
# False Positive Rate and outputting the following to stdout:
# <nhash> <length> <fprQoQ> <fprPart> <fprUnPart> <fprBatch 2> <fprBatch 4>...
##############################################################################

from subprocess import Popen, PIPE
import re
import multiprocessing as mp

# Function to run intersect simulation and extract fpr
def simFPR( cArgs ):
    # Run sigintersect C program with the appropriate arguments
    p = Popen( cArgs, stdout=PIPE, stderr=PIPE )

    # Extract the false positive rate (fpr)
    reFPR = "fpr.*([01]\.[0-9]+)"
    mo = re.search(reFPR, p.communicate()[0])
    if mo:
        return mo.group(1)
    else:
        return '-1'



# Cardinalities of sets A and B
A=64
B=64
# Number of trials for each experiment
#numTrials=10000
numTrials=1000000

# Name of C executable
exe="./sigintersect"
# List of # of hash fcns and signature length, and # batches
# All values are power-of-two
lsNumHashFcns = [2**k for k in range(0,3+1)] # 1 to 8
lsSigLen = [2**k for k in range(6, 20+1)] # 64 to 1M
lsNBatches = [2**k for k in range(1, 7+1)] # 2 to 128
#lsNumHashFcns = [2**k for k in range(0,1+1)]
#lsSigLen = [2**k for k in range(8, 10+1)] # 4 to 1M
#lsNBatches = [2**k for k in range(1, 3+1)] # 2 to 128


# Instantiate the pool
pool = mp.Pool()


# Build as necessary
Popen(["make", "clean"], stdout=PIPE, stderr=PIPE).wait()
Popen(["make", "release"], stdout=PIPE, stderr=PIPE).wait()

for k in lsNumHashFcns:
    for m in lsSigLen:
        lsres = []

        # Build the common arguments list
        lsCommonArgs = [exe, "-A"+str(A), "-B"+str(B), "-h"+str(k), 
                        "-l"+str(m), "-t"+str(numTrials)]

        # Queue of Query false positive rate
        lsres.append(pool.apply_async(simFPR, (lsCommonArgs + ["-q"],))) 
        # Partitioned Intersection false positive rate
        lsres.append(pool.apply_async(simFPR, (lsCommonArgs,))) 
        # Unpartitioned Intersection false positive rate
        lsres.append(pool.apply_async(simFPR, (lsCommonArgs + ["-u"],)))
        for b in lsNBatches:
            # Batched Signature Intersection false positive rate
            lsres.append(pool.apply_async(simFPR, 
                          (lsCommonArgs + ["-b"+str(b)],))) 


        # The list of result objects contains the fprs in the order we want,
        # so extract them with "get"
        lsfprs = [obj.get() for obj in lsres]

        # Print this configuration of fprs
        print ' '.join( map(str, [k, m] + lsfprs) )

pool.close()
pool.join()

