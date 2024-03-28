; ModuleID = '/home/christofer/workspace/GitLLVM/compilers-assignments/first-assignment/testing/tests/advanced_pow2_sr.optimized.bc'
source_filename = "/home/christofer/workspace/GitLLVM/compilers-assignments/first-assignment/testing/tests/advanced_pow2_sr.ll"

define i32 @test1(i32 noundef %0) {
  %2 = shl i32 %0, 3
  %3 = add i32 %2, %0
  ret i32 %3
}

define i32 @test2(i32 noundef %0) {
  %2 = shl i32 %0, 4
  %3 = sub i32 %2, %0
  ret i32 %3
}

define i32 @test3(i32 noundef %0) {
  %2 = shl i32 31, 3
  ret i32 %2
}
