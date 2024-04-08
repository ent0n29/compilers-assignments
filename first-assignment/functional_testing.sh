#!/bin/bash

# path to your directory containing the .ll files
my_path=$(realpath $0)
my_dir=$(dirname $my_path)
dir="$my_dir/testing"
declare -i exit_code=0

for file in $dir/functional_tests/*.ll; do

    base=$(basename $file .ll)
    optimized=$dir/optimized/$base\_optimized.ll

    echo "----- Optimizing $base.ll -----"

    # run passes
    opt -p localopts $file -S -o $optimized

    echo "Optimization ended, comparing stdout"
    diff <(lli $optimized) <(lli $file)

    do_differ=$?
    exit_code+=$do_differ

    if [[ $do_differ -eq "0" ]];
      then echo -e "------ PASSED $base.ll ------\n";
      else echo -e "------ FAILED $base.ll ------\n"
    fi
    
done

exit $exit_code
