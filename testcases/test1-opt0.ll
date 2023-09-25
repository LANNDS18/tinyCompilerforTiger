; ModuleID = 'temp.bc'
; clang++ -S -emit-llvm temp.bc  -o test1-opt0.ll
source_filename = "The Module"
target datalayout = "e-m:o-i64:64-i128:128-n32:64-S128"
target triple = "arm64-apple-macosx13.0.0"

%person = type { i8*, i64 }

@0 = private unnamed_addr constant [2 x i8] c"a\00", align 1
@1 = private unnamed_addr constant [2 x i8] c" \00", align 1
@2 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@3 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@4 = private unnamed_addr constant [23 x i8] c"I'm P1 and My name is \00", align 1
@5 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1

declare void @__print__(i8*)

declare void @__puti__(i64)

declare void @__flush__()

declare i8* @__getchar__()

declare i64 @__getint__()

declare i64 @__ord__(i8*)

declare i64 @__size__(i8*)

declare i8* @__substring__(i8*, i64, i64)

declare i8* @__concat__(i8*, i8*)

declare i64 @__not__(i64)

declare void @__exit__(i64)

declare i8* @alloc(i64)

declare i8* @__gets__()

declare i8* @alloc.1(i64)

define i64 @main() {
entry:
  %0 = call i64 @__getint__()
  %1 = alloca i64, align 8
  store i64 %0, i64* %1, align 4
  %2 = alloca i64, align 8
  store i64 2, i64* %2, align 4
  %3 = alloca i64, align 8
  store i64 0, i64* %3, align 4
  %4 = load i64, i64* %1, align 4
  %5 = mul i64 %4, 8
  %6 = call i8* @alloc(i64 %5)
  %array = bitcast i8* %6 to i64*
  br label %finit

finit:                                            ; preds = %entry
  %7 = alloca i32, align 4
  store i32 0, i32* %7, align 4
  br label %fcond

fcond:                                            ; preds = %fbody, %finit
  %8 = load i32, i32* %7, align 4
  %9 = trunc i64 %4 to i32
  %10 = icmp slt i32 %8, %9
  br i1 %10, label %fbody, label %fend

fbody:                                            ; preds = %fcond
  %11 = load i32, i32* %7, align 4
  %12 = getelementptr i64, i64* %array, i32 %11
  store i64 0, i64* %12, align 4
  %13 = load i32, i32* %7, align 4
  %14 = add i32 %13, 1
  store i32 %14, i32* %7, align 4
  br label %fcond

fend:                                             ; preds = %fcond
  %15 = alloca i64*, align 8
  store i64* %array, i64** %15, align 8
  %16 = call i8* @alloc(i64 16)
  %17 = bitcast i8* %16 to %person*
  %18 = getelementptr %person, %person* %17, i32 0, i32 0
  store i8* getelementptr inbounds ([2 x i8], [2 x i8]* @0, i32 0, i32 0), i8** %18, align 8
  %19 = getelementptr %person, %person* %17, i32 0, i32 1
  store i64 10, i64* %19, align 4
  %20 = alloca %person*, align 8
  store %person* %17, %person** %20, align 8
  %21 = load i64*, i64** %15, align 8
  %22 = load i64, i64* %1, align 4
  call void @printArr(i64* %21, i64 %22)
  %23 = load i64*, i64** %15, align 8
  %24 = load i64, i64* %1, align 4
  call void @initArr(i64* %23, i64 %24)
  %25 = load i64*, i64** %15, align 8
  %26 = load i64, i64* %1, align 4
  call void @printArr(i64* %25, i64 %26)
  %27 = load i64, i64* %3, align 4
  %28 = load i64*, i64** %15, align 8
  %29 = load i64, i64* %1, align 4
  %30 = load i64, i64* %2, align 4
  %31 = call i64 @initArr2(i64* %28, i64 %29, i64 %30)
  %32 = icmp eq i64 %27, %31
  %33 = zext i1 %32 to i64
  call void @__print__(i8* getelementptr inbounds ([23 x i8], [23 x i8]* @4, i32 0, i32 0))
  %34 = load %person*, %person** %20, align 8
  %35 = getelementptr %person, %person* %34, i32 0, i32 0
  %36 = load i8*, i8** %35, align 8
  call void @__print__(i8* %36)
  call void @__print__(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @5, i32 0, i32 0))
  %37 = load i64, i64* %3, align 4
  call void @__puti__(i64 %37)
  ret i64 0
}

define internal void @printArr(i64* %0, i64 %1) {
entry:
  %2 = alloca i64*, align 8
  store i64* %0, i64** %2, align 8
  %3 = alloca i64, align 8
  store i64 %1, i64* %3, align 4
  br label %finit

finit:                                            ; preds = %entry
  %4 = alloca i64, align 8
  store i64 0, i64* %4, align 4
  br label %fcond

fcond:                                            ; preds = %fbody, %finit
  %5 = load i64, i64* %3, align 4
  %6 = sub i64 %5, 1
  %7 = load i64, i64* %4, align 4
  %8 = icmp sle i64 %7, %6
  br i1 %8, label %fbody, label %fend

fbody:                                            ; preds = %fcond
  %9 = load i64*, i64** %2, align 8
  %10 = load i64, i64* %4, align 4
  %11 = trunc i64 %10 to i32
  %12 = getelementptr i64, i64* %9, i32 %11
  %13 = load i64, i64* %12, align 4
  call void @__puti__(i64 %13)
  call void @__print__(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @1, i32 0, i32 0))
  %14 = load i64, i64* %4, align 4
  %15 = add i64 %14, 1
  store i64 %15, i64* %4, align 4
  br label %fcond

fend:                                             ; preds = %fcond
  call void @__print__(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @2, i32 0, i32 0))
  ret void
}

define internal void @initArr(i64* %0, i64 %1) {
entry:
  %2 = alloca i64*, align 8
  store i64* %0, i64** %2, align 8
  %3 = alloca i64, align 8
  store i64 %1, i64* %3, align 4
  br label %finit

finit:                                            ; preds = %entry
  %4 = alloca i64, align 8
  store i64 0, i64* %4, align 4
  br label %fcond

fcond:                                            ; preds = %fbody, %finit
  %5 = load i64, i64* %3, align 4
  %6 = sub i64 %5, 1
  %7 = load i64, i64* %4, align 4
  %8 = icmp sle i64 %7, %6
  br i1 %8, label %fbody, label %fend

fbody:                                            ; preds = %fcond
  %9 = load i64*, i64** %2, align 8
  %10 = load i64, i64* %4, align 4
  %11 = trunc i64 %10 to i32
  %12 = getelementptr i64, i64* %9, i32 %11
  %13 = load i64, i64* %3, align 4
  store i64 %13, i64* %12, align 4
  %14 = load i64, i64* %4, align 4
  %15 = add i64 %14, 1
  store i64 %15, i64* %4, align 4
  br label %fcond

fend:                                             ; preds = %fcond
  ret void
}

define internal i64 @initArr2(i64* %0, i64 %1, i64 %2) {
entry:
  %3 = alloca i64*, align 8
  store i64* %0, i64** %3, align 8
  %4 = alloca i64, align 8
  store i64 %1, i64* %4, align 4
  %5 = alloca i64, align 8
  store i64 %2, i64* %5, align 4
  br label %finit

finit:                                            ; preds = %entry
  %6 = alloca i64, align 8
  store i64 0, i64* %6, align 4
  br label %fcond

fcond:                                            ; preds = %fbody, %finit
  %7 = load i64, i64* %4, align 4
  %8 = sub i64 %7, 1
  %9 = load i64, i64* %6, align 4
  %10 = icmp sle i64 %9, %8
  br i1 %10, label %fbody, label %fend

fbody:                                            ; preds = %fcond
  %11 = load i64*, i64** %3, align 8
  %12 = load i64, i64* %6, align 4
  %13 = trunc i64 %12 to i32
  %14 = getelementptr i64, i64* %11, i32 %13
  %15 = load i64, i64* %4, align 4
  store i64 %15, i64* %14, align 4
  %16 = load i64, i64* %6, align 4
  %17 = add i64 %16, 1
  store i64 %17, i64* %6, align 4
  br label %fcond

fend:                                             ; preds = %fcond
  store i64 1000, i64* %4, align 4
  %18 = load i64, i64* %5, align 4
  %19 = add i64 %18, 1
  store i64 %19, i64* %5, align 4
  %20 = load i64, i64* %4, align 4
  call void @__puti__(i64 %20)
  call void @__print__(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @3, i32 0, i32 0))
  ret i64 1
}
