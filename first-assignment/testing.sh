#!/bin/bash

# path to your directory containing the .ll files
dir="./testing"
declare -i exit_code=0

for file in $dir/tests/*.ll; do

    base=$(basename $file .ll)
    echo -e "\n\nStarting the optimization of $base.ll"

    # run passes
    opt -p localopts $file -S -o $dir/optimized/$base\_optimized.ll

    echo -e "\nOptimization ended, checking the results"
    diff <(tail +4 $dir/optimized/$base\_optimized.ll) <(cat $dir/expected/$base\_expected.ll)

    do_differ=$?
    exit_code+=$do_differ

    if [[ $do_differ -eq "0" ]]; then echo "Correct results"; fi
done

exit $exit_code
