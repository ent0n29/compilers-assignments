define i32 @neutral_sum(i32 noundef %0) {
  %2 = mul i32 %0, %0
  ret i32 %2
}

define i32 @neutral_mul(i32 noundef %0) {
  %2 = mul i32 %0, %0
  ret i32 %2
}
