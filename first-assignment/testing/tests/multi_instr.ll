; Add then sub
define i32 @test1(i32 noundef %0) {
  %2 = add i32 %0, 10
  %3 = mul i32 %2, 6
  %4 = sub i32 %2, 10
  ret i32 %4
}

; Sub then add
define i32 @test2(i32 noundef %0) {
  %2 = sub i32 %0, 10
  %3 = mul i32 %2, 6
  %4 = add i32 %2, 10
  ret i32 %4
}

; No sandwich
define i32 @test3(i32 noundef %0) {
  %2 = sub i32 %0, 10
  %3 = add i32 %2, 10
  ret i32 %3
}

; Bigger sandwich
define i32 @test4(i32 noundef %0) {
  %2 = sub i32 %0, 10
  %3 = mul i32 %2, 6
  %4 = shl i32 %2, 2
  %5 = add i32 %2, 10
  %6 = add i32 %5, 1
  ret i32 %6
}

; Placebo test: different constants
define i32 @test5(i32 noundef %0) {
  %2 = add i32 %0, 10
  %3 = add i32 %2, 6
  %4 = sub i32 %2, 9
  ret i32 %4
}

; Placebo test: sub isn't commutative
define i32 @test6(i32 noundef %0) {
  %2 = add i32 10, %0
  %3 = sub i32 10, %2
  ret i32 %3
}

; Placebo test: sub isn't commutative
define i32 @test7(i32 noundef %0) {
  %2 = sub i32 10, %0
  %3 = add i32 10, %2
  ret i32 %3
}

define i32 @main() {
  %1 = call i32 @test1(i32 101)
  ret i32 %1
}