; RUN: llc %s -o - | FileCheck %s

target triple = "spirv32-unknown-unknown"

declare i32 @llvm.spirv.local.invocation.index()
declare <3 x i32> @llvm.spirv.local.invocation.id()

define i32 @dummy0() {
start:
  %r = call i32 @llvm.spirv.local.invocation.index()
  ret i32 %r
}
define <3 x i32> @dummy1() {
start:
  %r = call <3 x i32> @llvm.spirv.local.invocation.id()
  ret <3 x i32> %r
}
