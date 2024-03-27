; ModuleID = 'algebraic_id.optimized.bc'
source_filename = "algebraic_id.ll"

define i32 @neutral_sum(i32 noundef %0) {
  %2 = add i32 %0, 0
  %3 = add i32 0, %0
  %4 = mul i32 %0, %0
  ret i32 %4
}

define i32 @neutral_mul(i32 noundef %0) {
  %2 = mul i32 %0, 1
  %3 = shl i32 %0, 1
  %4 = sub i32 %3, %0
  %5 = mul i32 1, %0
  %6 = shl i32 %0, 1
  %7 = sub i32 %6, %0
  %8 = mul i32 %0, %0
  ret i32 %8
}
