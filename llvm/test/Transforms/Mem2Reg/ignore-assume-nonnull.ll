; RUN: opt -mem2reg -S -o - < %s | FileCheck %s

declare void @llvm.assume(i1)

define void @test1() {
; CHECK: test1
; CHECK-NOT: alloca
  %A = alloca i32
  %b = icmp ne i32* %A, null
  call void @llvm.assume(i1 %b)
  store i32 1, i32* %A
  ret void
}
