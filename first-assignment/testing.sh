#!/bin/bash

# path to your directory containing the .ll files
my_path=$(realpath $0)
my_dir=$(dirname $my_path)
dir="$my_dir/testing"
declare -i exit_code=0

for file in $dir/tests/*.ll; do

    base=$(basename $file .ll)
    echo "----- Optimizing $base.ll -----"

    # run passes
    /home/christofer/workspace/LLVM_17/BUILD/bin/opt -p localopts $file -S -o $dir/optimized/$base\_optimized.ll

    echo "Optimization ended, checking the results"
    diff <(tail +4 $dir/optimized/$base\_optimized.ll) <(cat $dir/expected/$base\_expected.ll)

    do_differ=$?
    exit_code+=$do_differ

    if [[ $do_differ -eq "0" ]];
      then echo -e "------ PASSED $base.ll ------\n";
      else echo -e "------ FAILED $base.ll ------\n"
    fi
    
done

exit $exit_code
