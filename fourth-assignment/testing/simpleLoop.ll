; ModuleID = 'simpleLoop.ll'
source_filename = "simpleLoop.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @loop(i32 noundef %0, i32 noundef %1) #0 {
  br label %3

3:                                                ; preds = %7, %2
  %.02 = phi i32 [ 0, %2 ], [ %8, %7 ]
  %.01 = phi i32 [ 100, %2 ], [ %6, %7 ]
  %4 = icmp slt i32 %.02, 12
  br i1 %4, label %5, label %9

5:                                                ; preds = %3
  %6 = add nsw i32 %.01, %0
  br label %7

7:                                                ; preds = %5
  %8 = add nsw i32 %.02, 1
  br label %3, !llvm.loop !6

9:                                                ; preds = %3
  %10 = add nsw i32 3, 4
  br label %11

11:                                               ; preds = %15, %9
  %.13 = phi i32 [ 0, %9 ], [ %16, %15 ]
  %.0 = phi i32 [ %10, %9 ], [ %14, %15 ]
  %12 = icmp slt i32 %.13, 12
  br i1 %12, label %13, label %17

13:                                               ; preds = %11
  %14 = add nsw i32 %.0, %1
  br label %15

15:                                               ; preds = %13
  %16 = add nsw i32 %.13, 1
  br label %11, !llvm.loop !8

17:                                               ; preds = %11
  br label %18

18:                                               ; preds = %22, %17
  %.2 = phi i32 [ 0, %17 ], [ %23, %22 ]
  %.1 = phi i32 [ %.01, %17 ], [ %21, %22 ]
  %19 = icmp slt i32 %.2, 12
  br i1 %19, label %20, label %24

20:                                               ; preds = %18
  %21 = add nsw i32 %.1, %0
  br label %22

22:                                               ; preds = %20
  %23 = add nsw i32 %.2, 1
  br label %18, !llvm.loop !9

24:                                               ; preds = %18
  %25 = add nsw i32 %.1, %.0
  ret i32 %25
}

attributes #0 = { noinline nounwind uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"Ubuntu clang version 18.1.3 (1)"}
!6 = distinct !{!6, !7}
!7 = !{!"llvm.loop.mustprogress"}
!8 = distinct !{!8, !7}
!9 = distinct !{!9, !7}
