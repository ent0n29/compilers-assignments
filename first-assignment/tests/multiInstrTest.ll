; ModuleID = 'OptimizationTest'
source_filename = "opt_test.ll"

define i32 @test_function(i32 %b) {
entry:
  ; Increment %b by 1
  %a = add i32 %b, 1
  
  ; Immediately decrement %a by 1, resulting in the original value %b
  %c = sub i32 %a, 1
  
  ; Use %c in a subsequent operation to ensure it's not optimized away as dead code
  %d = add i32 %c, 3
  
  ret i32 %d
}
