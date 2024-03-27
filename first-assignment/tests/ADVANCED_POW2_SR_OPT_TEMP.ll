; ModuleID = 'advanced_pow2_srOPT_TEMPORARY.optimized.bc'
source_filename = "advanced_pow2_sr.ll"

define i32 @test1(i32 noundef %0) {
  %2 = mul i32 %0, 15
  %3 = shl i32 %0, 4
  %4 = sub i32 %3, %0
  ret i32 %4
}
