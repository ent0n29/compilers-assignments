#!/bin/bash

# path to your directory containing the .ll files
DIR="./testing"
declare -i EXIT_CODE=0

for file in $DIR/tests/*.ll; do

    base=$(basename $file .ll)
    echo -e "\n\nStarting the optimization of $base.ll"

    # run passes
    opt -p localopts $file -o $DIR/tests/$base.optimized.bc
    llvm-dis $DIR/tests/$base.optimized.bc -o $DIR/optimized/$base\_optimized.ll

    echo -e "\nOptimization ended, checking the results"
    diff <(tail +4 $DIR/optimized/$base\_optimized.ll) <(cat $DIR/expected/$base\_expected.ll)

    DO_DIFFER=$?
    EXIT_CODE+=$DO_DIFFER
    
    if [[ $DO_DIFFER -eq "0" ]]; then echo "Correct results"; fi
done

# remove bytecode
rm $DIR/tests/*.bc
exit $EXIT_CODE
