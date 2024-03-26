; ModuleID = 'multi.optimized.bc'
source_filename = "opt_test.ll"

define i32 @test_function(i32 %b) {
entry:
  %d = add i32 %b, 3
  ret i32 %d
}
