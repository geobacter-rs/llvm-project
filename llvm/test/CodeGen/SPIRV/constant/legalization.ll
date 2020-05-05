; RUN: opt %s -spirv-legalize-consts -o - | FileCheck %s

target triple = "spirv64-unknown-unknown-vulkan"

@global = internal addrspace(1) global [1 x i32] undef

; CHECK-LABEL: @t
define spir_kernel void @t() {
entry:
; CHECK: %0 = getelementptr inbounds [1 x i32], [1 x i32] addrspace(1)* @global, i32 0, i32 0
; CHECK-NEXT: store i32 1, i32 addrspace(1)* %0
  store i32 1, i32 addrspace(1)* getelementptr ([1 x i32], [1 x i32] addrspace(1)* @global, i32 0, i32 0)
  ret void
}

; CHECK-LABEL: @t2
define spir_kernel void @t2() {
entry:
; CHECK: %0 = getelementptr inbounds [1 x i32], [1 x i32] addrspace(1)* @global, i32 0, i32 0
; CHECK-NEXT: store i32 1, i32 addrspace(1)* %0
; CHECK-NEXT: store i32 2, i32 addrspace(1)* %0
  store i32 1, i32 addrspace(1)* getelementptr ([1 x i32], [1 x i32] addrspace(1)* @global, i32 0, i32 0)
  store i32 2, i32 addrspace(1)* getelementptr ([1 x i32], [1 x i32] addrspace(1)* @global, i32 0, i32 0)
  ret void
}
