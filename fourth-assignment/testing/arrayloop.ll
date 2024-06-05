; ModuleID = 'arrayloop.ll'
source_filename = "arrayloop.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define dso_local void @twoloop(ptr noundef %0, ptr noundef %1, ptr noundef %2, ptr noundef %3, i32 noundef %4) #0 {
  br label %6

6:                                                ; preds = %18, %5
  %.0 = phi i32 [ 3, %5 ], [ %19, %18 ]
  %7 = icmp slt i32 %.0, %4
  br i1 %7, label %8, label %20

8:                                                ; preds = %6
  %9 = sext i32 %.0 to i64
  %10 = getelementptr inbounds i32, ptr %1, i64 %9
  %11 = load i32, ptr %10, align 4
  %12 = sext i32 %.0 to i64
  %13 = getelementptr inbounds i32, ptr %2, i64 %12
  %14 = load i32, ptr %13, align 4
  %15 = add nsw i32 %11, %14
  %16 = sext i32 %.0 to i64
  %17 = getelementptr inbounds i32, ptr %0, i64 %16
  store i32 %15, ptr %17, align 4
  br label %18

18:                                               ; preds = %8
  %19 = add nsw i32 %.0, 1
  br label %6, !llvm.loop !6

20:                                               ; preds = %6
  br label %21

21:                                               ; preds = %33, %20
  %.1 = phi i32 [ 0, %20 ], [ %34, %33 ]
  %22 = icmp slt i32 %.1, %4
  br i1 %22, label %23, label %35

23:                                               ; preds = %21
  %24 = sext i32 %.1 to i64
  %25 = getelementptr inbounds i32, ptr %0, i64 %24
  %26 = load i32, ptr %25, align 4
  %27 = sext i32 %.1 to i64
  %28 = getelementptr inbounds i32, ptr %0, i64 %27
  %29 = load i32, ptr %28, align 4
  %30 = mul nsw i32 %26, %29
  %31 = sext i32 %.1 to i64
  %32 = getelementptr inbounds i32, ptr %3, i64 %31
  store i32 %30, ptr %32, align 4
  br label %33

33:                                               ; preds = %23
  %34 = add nsw i32 %.1, 1
  br label %21, !llvm.loop !8

35:                                               ; preds = %21
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
