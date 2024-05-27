; ModuleID = 'loop.ll'
source_filename = "loop.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@g = dso_local global i32 4, align 4

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @loop(ptr noundef %0, ptr noundef %1, ptr noundef %2, ptr noundef %3) #0 {
  br label %5

5:                                                ; preds = %33, %4
  %.01 = phi i32 [ 0, %4 ], [ %34, %33 ]
  %6 = icmp slt i32 %.01, 10
  br i1 %6, label %7, label %35

7:                                                ; preds = %5
  br label %8

8:                                                ; preds = %30, %7
  %.0 = phi i32 [ 0, %7 ], [ %31, %30 ]
  %9 = icmp slt i32 %.0, 10
  br i1 %9, label %10, label %32

10:                                               ; preds = %8
  %11 = sext i32 %.01 to i64
  %12 = getelementptr inbounds ptr, ptr %1, i64 %11
  %13 = load ptr, ptr %12, align 8
  %14 = sext i32 %.0 to i64
  %15 = getelementptr inbounds i32, ptr %13, i64 %14
  %16 = load i32, ptr %15, align 4
  %17 = sdiv i32 1, %16
  %18 = sext i32 %.01 to i64
  %19 = getelementptr inbounds ptr, ptr %2, i64 %18
  %20 = load ptr, ptr %19, align 8
  %21 = sext i32 %.0 to i64
  %22 = getelementptr inbounds i32, ptr %20, i64 %21
  %23 = load i32, ptr %22, align 4
  %24 = mul nsw i32 %17, %23
  %25 = sext i32 %.01 to i64
  %26 = getelementptr inbounds ptr, ptr %0, i64 %25
  %27 = load ptr, ptr %26, align 8
  %28 = sext i32 %.0 to i64
  %29 = getelementptr inbounds i32, ptr %27, i64 %28
  store i32 %24, ptr %29, align 4
  br label %30

30:                                               ; preds = %10
  %31 = add nsw i32 %.0, 1
  br label %8, !llvm.loop !6

32:                                               ; preds = %8
  br label %33

33:                                               ; preds = %32
  %34 = add nsw i32 %.01, 1
  br label %5, !llvm.loop !8

35:                                               ; preds = %5
  br label %36

36:                                               ; preds = %63, %35
  %.12 = phi i32 [ 0, %35 ], [ %64, %63 ]
  %37 = icmp slt i32 %.12, 10
  br i1 %37, label %38, label %65

38:                                               ; preds = %36
  br label %39

39:                                               ; preds = %60, %38
  %.1 = phi i32 [ 0, %38 ], [ %61, %60 ]
  %40 = icmp slt i32 %.1, 10
  br i1 %40, label %41, label %62

41:                                               ; preds = %39
  %42 = sext i32 %.12 to i64
  %43 = getelementptr inbounds ptr, ptr %0, i64 %42
  %44 = load ptr, ptr %43, align 8
  %45 = sext i32 %.1 to i64
  %46 = getelementptr inbounds i32, ptr %44, i64 %45
  %47 = load i32, ptr %46, align 4
  %48 = sext i32 %.12 to i64
  %49 = getelementptr inbounds ptr, ptr %2, i64 %48
  %50 = load ptr, ptr %49, align 8
  %51 = sext i32 %.1 to i64
  %52 = getelementptr inbounds i32, ptr %50, i64 %51
  %53 = load i32, ptr %52, align 4
  %54 = add nsw i32 %47, %53
  %55 = sext i32 %.12 to i64
  %56 = getelementptr inbounds ptr, ptr %3, i64 %55
  %57 = load ptr, ptr %56, align 8
  %58 = sext i32 %.1 to i64
  %59 = getelementptr inbounds i32, ptr %57, i64 %58
  store i32 %54, ptr %59, align 4
  br label %60

60:                                               ; preds = %41
  %61 = add nsw i32 %.1, 1
  br label %39, !llvm.loop !9

62:                                               ; preds = %39
  br label %63

63:                                               ; preds = %62
  %64 = add nsw i32 %.12, 1
  br label %36, !llvm.loop !10

65:                                               ; preds = %36
  %66 = load i32, ptr @g, align 4
  %67 = add nsw i32 0, %66
  ret i32 %67
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
!10 = distinct !{!10, !7}
