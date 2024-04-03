; ModuleID = '/home/enton/programming/compilers-assignments/first-assignment/testing/tests/edge_cases.optimized.bc'
source_filename = "/home/enton/programming/compilers-assignments/first-assignment/testing/tests/edge_cases.ll"

define i32 @test_algebraic_id(i32 noundef %0) {
  %2 = shl i32 %0, 0
  %3 = sub i32 %2, %0
  %4 = add i32 1, %3
  ret i32 %4
}

define i32 @test_sr(i32 noundef %0) {
  %2 = mul i32 30, 10
  %3 = udiv i32 16, %0
  %4 = udiv i32 %2, 15
  ret i32 %4
}

define i32 @test_multi_instr(i32 noundef %0) {
  %2 = add i32 %0, 10
  %3 = sub i32 %2, 5
  %4 = add i32 %3, 15
  %5 = add i32 %0, 10
  ret i32 %5
}
