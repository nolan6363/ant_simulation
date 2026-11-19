#!/bin/bash

STEPS=${1:-0}

./bin/main_seq $STEPS 0 ref.bin > /dev/null

for t in 1 2 3 4; do
    OMP_NUM_THREADS=$t ./bin/main $STEPS 0 par.bin > /dev/null
    if cmp -s ref.bin par.bin; then
        echo "$t threads : OK"
    else
        echo "$t threads : DIFFERENT"
    fi
done

rm -f ref.bin par.bin
