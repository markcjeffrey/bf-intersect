#!/bin/bash

# 2010 September 9
# Mark Jeffrey
# This script builds and runs the local sig_intersect program, collecting the
# False Positive Rate (which is of most interest) and outputting to a .out file

A=64
B=64
numTrials=1000000
numSamples=1
#numBatches=4
exe="sigintersect"

aryNBATCHES=(1 2 4 8 16 32)
aryNHASHFCNS=(1 2 4 8)
arySIGLEN=(16 32 64 128 256 512 \
           1024 2048 4096 8192 \
           16384 32768 65536)

# Build as necessary
make clean > /dev/null 2>&1
make release > /dev/null 2>&1

for nhashfcns in ${aryNHASHFCNS[@]} 
do
    for siglen in ${arySIGLEN[@]} 
    do
        for numbatches in ${aryNBATCHES[@]} 
        do

        # Batched Signature Intersection false positive rate
        fprbatchxsect=`./${exe} -A${A} \
        -B${B} -h${nhashfcns} -l${siglen} \
        -t${numTrials} -b${numbatches} \
        | sed -n 's/fpr.*\([01]\.[0-9]*\)/\1/p'`

        if [ "${?}" == "0" ]; then
            if [ -n "${fprbatchxsect}" ]; then
                echo $nhashfcns $siglen $numbatches $fprbatchxsect
            fi
        fi
        
        done

    done

done
