; ModuleID = 'test.c'
source_filename = "test.c"
target datalayout = "e-m:o-i64:64-i128:128-n32:64-S128"
target triple = "arm64-apple-macosx14.0.0"

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

; Function Attrs: noinline nounwind optnone ssp uwtable(sync)
define i32 @test1(i32 noundef %0) #0 {
  %2 = add nsw i32 %0, 10
  %3 = sub nsw i32 %2, 10
  ret i32 %3
}

; Function Attrs: noinline nounwind optnone ssp uwtable(sync)
define i32 @test2(i32 noundef %0) #0 {
  %2 = sub nsw i32 %0, 10
  %3 = add nsw i32 %2, 10
  ret i32 %3
}

; Function Attrs: noinline nounwind optnone ssp uwtable(sync)
define i32 @test3(i32 noundef %0) #0 {
  %2 = add nsw i32 10, %0
  %3 = sub nsw i32 %2, 10
  ret i32 %3
}

; Function Attrs: noinline nounwind optnone ssp uwtable(sync)
define i32 @test4(i32 noundef %0) #0 {
  %2 = sub nsw i32 10, %0
  %3 = add nsw i32 %2, 10
  ret i32 %3
}

; Function Attrs: noinline nounwind optnone ssp uwtable(sync)
define i32 @test5(i32 noundef %0) #0 {
  %2 = add nsw i32 %0, 10
  %3 = sub nsw i32 10, %2
  ret i32 %3
}

; Function Attrs: noinline nounwind optnone ssp uwtable(sync)
define i32 @test6(i32 noundef %0) #0 {
  %2 = sub nsw i32 %0, 10
  %3 = add nsw i32 10, %2
  ret i32 %3
}

; Function Attrs: noinline nounwind optnone ssp uwtable(sync)
define i32 @test7(i32 noundef %0) #0 {
  %2 = add nsw i32 10, %0
  %3 = sub nsw i32 10, %2
  ret i32 %3
}

; Function Attrs: noinline nounwind optnone ssp uwtable(sync)
define i32 @test8(i32 noundef %0) #0 {
  %2 = sub nsw i32 10, %0
  %3 = add nsw i32 10, %2
  ret i32 %3
}

; Function Attrs: noinline nounwind optnone ssp uwtable(sync)
define i32 @main() #0 {
  %1 = call i32 @test1(i32 noundef 101)
  %2 = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %1)
  %3 = call i32 @test2(i32 noundef 101)
  %4 = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %3)
  %5 = call i32 @test3(i32 noundef 101)
  %6 = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %5)
  %7 = call i32 @test4(i32 noundef 101)
  %8 = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %7)
  %9 = call i32 @test5(i32 noundef 101)
  %10 = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %9)
  %11 = call i32 @test6(i32 noundef 101)
  %12 = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %11)
  %13 = call i32 @test7(i32 noundef 101)
  %14 = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %13)
  %15 = call i32 @test8(i32 noundef 101)
  %16 = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %15)
  ret i32 0
}

declare i32 @printf(ptr noundef, ...) #1

attributes #0 = { noinline nounwind optnone ssp uwtable(sync) "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="apple-m1" "target-features"="+aes,+crc,+dotprod,+fp-armv8,+fp16fml,+fullfp16,+lse,+neon,+ras,+rcpc,+rdm,+sha2,+sha3,+v8.1a,+v8.2a,+v8.3a,+v8.4a,+v8.5a,+v8a,+zcm,+zcz" }
attributes #1 = { "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="apple-m1" "target-features"="+aes,+crc,+dotprod,+fp-armv8,+fp16fml,+fullfp16,+lse,+neon,+ras,+rcpc,+rdm,+sha2,+sha3,+v8.1a,+v8.2a,+v8.3a,+v8.4a,+v8.5a,+v8a,+zcm,+zcz" }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{i32 7, !"frame-pointer", i32 1}
!4 = !{!"clang version 17.0.6 (https://github.com/llvm/llvm-project/ 6009708b4367171ccdbf4b5905cb6a803753fe18)"}
