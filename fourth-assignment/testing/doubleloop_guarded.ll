; ModuleID = 'doubleloop_guarded.ll'
source_filename = "doubleloop_guarded.c"
target datalayout = "e-m:o-i64:64-i128:128-n32:64-S128"
target triple = "arm64-apple-macosx14.0.0"

; Function Attrs: noinline nounwind ssp uwtable(sync)
define i32 @loop(i32 noundef %0, i32 noundef %1, i32 noundef %2) #0 {
  %4 = icmp slt i32 1, %2
  br i1 %4, label %.lr.ph, label %._crit_edge10

.lr.ph:                                           ; preds = %3
  br label %5

5:                                                ; preds = %.lr.ph, %8
  %.015 = phi i32 [ 1, %.lr.ph ], [ %9, %8 ]
  %.034 = phi i32 [ 0, %.lr.ph ], [ %7, %8 ]
  %.026 = phi i32 [ 0, %.lr.ph ], [ %13, %8 ]
  %6 = mul nsw i32 %0, %.015
  %7 = add nsw i32 %.034, %6
  br label %11

8:                                                ; preds = %11
  %9 = add nsw i32 %.015, 1
  %10 = icmp slt i32 %9, %2
  br i1 %10, label %5, label %._crit_edge10, !llvm.loop !5

11:                                               ; preds = %5
  %12 = mul nsw i32 %1, %.015
  %13 = add nsw i32 %.026, %12
  br label %8

._crit_edge10:                                    ; preds = %3, %8
  br label %14

14:                                               ; preds = %._crit_edge10
  %15 = add nsw i32 0, poison
  ret i32 %15
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
