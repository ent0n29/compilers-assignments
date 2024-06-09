; ModuleID = 'simpleloop.ll'
source_filename = "simpleloop.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @loop(i32 noundef %0, i32 noundef %1) #0 {
  br label %3

3:                                                ; preds = %7, %2
  %.02 = phi i32 [ 0, %2 ], [ %8, %7 ]
  %.01 = phi i32 [ 100, %2 ], [ %6, %7 ]
  %4 = icmp slt i32 %.02, 10
  br i1 %4, label %5, label %9

5:                                                ; preds = %3
  %6 = add nsw i32 %.01, %0
  br label %7

7:                                                ; preds = %5
  %8 = add nsw i32 %.02, 1
  br label %3, !llvm.loop !6

9:                                                ; preds = %3
  br label %10

10:                                               ; preds = %14, %9
  %.13 = phi i32 [ 0, %9 ], [ %15, %14 ]
  %.0 = phi i32 [ 3, %9 ], [ %13, %14 ]
  %11 = icmp slt i32 %.13, 10
  br i1 %11, label %12, label %16

12:                                               ; preds = %10
  %13 = add nsw i32 %.0, %1
  br label %14

14:                                               ; preds = %12
  %15 = add nsw i32 %.13, 1
  br label %10, !llvm.loop !8

16:                                               ; preds = %10
  br label %17

17:                                               ; preds = %21, %16
  %.2 = phi i32 [ 0, %16 ], [ %22, %21 ]
  %.1 = phi i32 [ 0, %16 ], [ %20, %21 ]
  %18 = icmp slt i32 %.2, 10
  br i1 %18, label %19, label %23

19:                                               ; preds = %17
  %20 = add nsw i32 %.1, %0
  br label %21

21:                                               ; preds = %19
  %22 = add nsw i32 %.2, 1
  br label %17, !llvm.loop !9

23:                                               ; preds = %17
  %24 = add nsw i32 %.1, %.0
  ret i32 %24
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
