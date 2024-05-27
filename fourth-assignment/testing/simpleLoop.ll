; ModuleID = 'simpleLoop.bc'
source_filename = "simpleLoop.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @loop(i32 noundef %0, i32 noundef %1) #0 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  store i32 %0, ptr %3, align 4
  store i32 %1, ptr %4, align 4
  store i32 0, ptr %5, align 4
  store i32 100, ptr %6, align 4
  store i32 3, ptr %7, align 4
  store i32 0, ptr %5, align 4
  br label %8

8:                                                ; preds = %15, %2
  %9 = load i32, ptr %5, align 4
  %10 = icmp slt i32 %9, 12
  br i1 %10, label %11, label %18

11:                                               ; preds = %8
  %12 = load i32, ptr %3, align 4
  %13 = load i32, ptr %6, align 4
  %14 = add nsw i32 %13, %12
  store i32 %14, ptr %6, align 4
  br label %15

15:                                               ; preds = %11
  %16 = load i32, ptr %5, align 4
  %17 = add nsw i32 %16, 1
  store i32 %17, ptr %5, align 4
  br label %8, !llvm.loop !6

18:                                               ; preds = %8
  %19 = load i32, ptr %7, align 4
  %20 = add nsw i32 %19, 4
  store i32 %20, ptr %7, align 4
  store i32 0, ptr %5, align 4
  br label %21

21:                                               ; preds = %28, %18
  %22 = load i32, ptr %5, align 4
  %23 = icmp slt i32 %22, 12
  br i1 %23, label %24, label %31

24:                                               ; preds = %21
  %25 = load i32, ptr %4, align 4
  %26 = load i32, ptr %7, align 4
  %27 = add nsw i32 %26, %25
  store i32 %27, ptr %7, align 4
  br label %28

28:                                               ; preds = %24
  %29 = load i32, ptr %5, align 4
  %30 = add nsw i32 %29, 1
  store i32 %30, ptr %5, align 4
  br label %21, !llvm.loop !8

31:                                               ; preds = %21
  store i32 0, ptr %5, align 4
  br label %32

32:                                               ; preds = %39, %31
  %33 = load i32, ptr %5, align 4
  %34 = icmp slt i32 %33, 12
  br i1 %34, label %35, label %42

35:                                               ; preds = %32
  %36 = load i32, ptr %3, align 4
  %37 = load i32, ptr %6, align 4
  %38 = add nsw i32 %37, %36
  store i32 %38, ptr %6, align 4
  br label %39

39:                                               ; preds = %35
  %40 = load i32, ptr %5, align 4
  %41 = add nsw i32 %40, 1
  store i32 %41, ptr %5, align 4
  br label %32, !llvm.loop !9

42:                                               ; preds = %32
  %43 = load i32, ptr %6, align 4
  %44 = load i32, ptr %7, align 4
  %45 = add nsw i32 %43, %44
  ret i32 %45
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
