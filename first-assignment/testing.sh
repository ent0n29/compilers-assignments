#!/bin/bash

# path to your directory containing the .ll files
dir="$PATH_TO_YOUR_DIRECTORY/compilers-assignments/first-assignment/tests/"
for file in $dir/*.ll; do
    base=$(basename $file .ll)

    # run passes
    opt -p localopts $file -o $dir/$base.optimized.bc
    llvm-dis $dir/$base.optimized.bc -o $dir/$base\_optimized.ll
done

# remove bytecode
rm $dir/*.bc
