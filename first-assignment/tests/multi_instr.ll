define i32 @test1(i32 noundef %0) {
  %2 = mul i32 %0, 10
  %3 = add i32 %2, 6
  %4 = udiv i32 %2, 10
  ret i32 %4
}

define i32 @test2(i32 noundef %0) {
  %2 = add i32 %0, 10
  %3 = mul i32 %2, 6
  %4 = sub i32 %2, 10
  ret i32 %4
}

define i32 @test3(i32 noundef %0) {
  %2 = mul i32 %0, 10
  %3 = add i32 %2, 6
  %4 = udiv i32 %2, 11
  ret i32 %4
}

define i32 @test4(i32 noundef %0) {
  %2 = add i32 %0, 10
  %3 = mul i32 %2, 6
  %4 = sub i32 %2, 11
  ret i32 %4
}