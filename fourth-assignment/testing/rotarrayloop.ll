; ModuleID = 'arrayloop.ll'
source_filename = "arrayloop.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define dso_local void @twoloop(ptr noundef %0, ptr noundef %1, ptr noundef %2, ptr noundef %3, i32 noundef %4) #0 {
  %6 = icmp slt i32 3, %4
  br i1 %6, label %.lr.ph, label %20

.lr.ph:                                           ; preds = %5
  br label %7

7:                                                ; preds = %.lr.ph, %17
  %.01 = phi i32 [ 3, %.lr.ph ], [ %18, %17 ]
  %8 = sext i32 %.01 to i64
  %9 = getelementptr inbounds i32, ptr %1, i64 %8
  %10 = load i32, ptr %9, align 4
  %11 = sext i32 %.01 to i64
  %12 = getelementptr inbounds i32, ptr %2, i64 %11
  %13 = load i32, ptr %12, align 4
  %14 = add nsw i32 %10, %13
  %15 = sext i32 %.01 to i64
  %16 = getelementptr inbounds i32, ptr %0, i64 %15
  store i32 %14, ptr %16, align 4
  br label %17

17:                                               ; preds = %7
  %18 = add nsw i32 %.01, 1
  %19 = icmp slt i32 %18, %4
  br i1 %19, label %7, label %._crit_edge, !llvm.loop !6

._crit_edge:                                      ; preds = %17
  br label %20

20:                                               ; preds = %._crit_edge, %5
  %21 = icmp slt i32 0, %4
  br i1 %21, label %.lr.ph4, label %35

.lr.ph4:                                          ; preds = %20
  br label %22

22:                                               ; preds = %.lr.ph4, %32
  %.12 = phi i32 [ 0, %.lr.ph4 ], [ %33, %32 ]
  %23 = sext i32 %.12 to i64
  %24 = getelementptr inbounds i32, ptr %0, i64 %23
  %25 = load i32, ptr %24, align 4
  %26 = sext i32 %.12 to i64
  %27 = getelementptr inbounds i32, ptr %0, i64 %26
  %28 = load i32, ptr %27, align 4
  %29 = mul nsw i32 %25, %28
  %30 = sext i32 %.12 to i64
  %31 = getelementptr inbounds i32, ptr %3, i64 %30
  store i32 %29, ptr %31, align 4
  br label %32

32:                                               ; preds = %22
  %33 = add nsw i32 %.12, 1
  %34 = icmp slt i32 %33, %4
  br i1 %34, label %22, label %._crit_edge5, !llvm.loop !8

._crit_edge5:                                     ; preds = %32
  br label %35

35:                                               ; preds = %._crit_edge5, %20
  ret void
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
