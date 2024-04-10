define i32 @test_algebraic_id(i32 noundef %0) {
  %2 = mul i32 0, %0
  %3 = add i32 1, %2
  ret i32 %3
}

define i32 @test_sr(i32 noundef %0) {
  %2 = mul i32 30, 10
  %3 = udiv i32 16, %0
  %4 = udiv i32 %2, 15
  ret i32 %4
}

define i32 @test_multi_instr(i32 noundef %0) {
  %2 = sub i32 12, 11
  %3 = add i32 12, 11
  ret i32 %3
}