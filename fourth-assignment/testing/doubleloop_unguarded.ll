; ModuleID = 'doubleloop_unguarded.ll'
source_filename = "doubleloop_unguarded.c"
target datalayout = "e-m:o-i64:64-i128:128-n32:64-S128"
target triple = "arm64-apple-macosx14.0.0"

; Function Attrs: noinline nounwind ssp uwtable(sync)
define i32 @loop(i32 noundef %0, i32 noundef %1) #0 {
  br label %3

3:                                                ; preds = %2, %6
  %.015 = phi i32 [ 1, %2 ], [ %7, %6 ]
  %.024 = phi i32 [ 0, %2 ], [ %5, %6 ]
  %.036 = phi i32 [ 0, %2 ], [ %11, %6 ]
  %4 = mul nsw i32 %0, %.015
  %5 = add nsw i32 %.024, %4
  br label %9

6:                                                ; preds = %9
  %7 = add nsw i32 %.015, 1
  %8 = icmp slt i32 %7, 12
  br i1 %8, label %3, label %12, !llvm.loop !5

9:                                                ; preds = %3
  %10 = mul nsw i32 %1, %.015
  %11 = add nsw i32 %.036, %10
  br label %6

12:                                               ; preds = %6
  %13 = add nsw i32 poison, poison
  ret i32 %13
}

attributes #0 = { noinline nounwind ssp uwtable(sync) "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="apple-m1" "target-features"="+aes,+crc,+dotprod,+fp-armv8,+fp16fml,+fullfp16,+lse,+neon,+ras,+rcpc,+rdm,+sha2,+sha3,+v8.1a,+v8.2a,+v8.3a,+v8.4a,+v8.5a,+v8a,+zcm,+zcz" }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{i32 7, !"frame-pointer", i32 1}
!4 = !{!"clang version 17.0.6 (https://github.com/llvm/llvm-project/ 6009708b4367171ccdbf4b5905cb6a803753fe18)"}
!5 = distinct !{!5, !6}
!6 = !{!"llvm.loop.mustprogress"}
