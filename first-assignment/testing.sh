#!/bin/bash

# path to your directory containing the .ll files
dir="/home/christofer/workspace/GitLLVM/compilers-assignments/first-assignment/tests"
for file in $dir/*.ll; do
    if [[ $file =~ _(optimized|expected).ll$ ]]; then continue; fi
    
    base=$(basename $file .ll)
    echo -e "\n\nStarting the optimization of $base.ll"

    # run passes
    /home/christofer/workspace/LLVM_17/BUILD/bin/opt -p localopts $file -o $dir/$base.optimized.bc
    /home/christofer/workspace/LLVM_17/BUILD/bin/llvm-dis $dir/$base.optimized.bc -o $dir/$base\_optimized.ll

    echo -e "\nOptimization ended, checking the results"
    diff <(tail +4 $dir/$base\_optimized.ll) <(cat $dir/$base\_expected.ll)
    if [[ $? -eq "0" ]]; then echo "Correct results"; fi
done

# remove bytecode
rm $dir/*.bc
