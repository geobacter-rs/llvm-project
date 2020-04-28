; RUN: llc -O0 -global-isel %s -o - | FileCheck %s

target triple = "spirv32-vulkan-unknown"

; CHECK: OpEntryPoint Vertex [[EXE_MODEL_FN_ID:%.+]] "exe_model"
; CHECK: OpEntryPoint Kernel [[EXE_MODE_FN_ID:%.+]] "exe_modes"

; CHECK: [[V_TY_ID:%.+]] = OpTypeVoid
; CHECK: [[FN_TY_ID:%.+]] = OpTypeFunction [[V_TY_ID]]

; CHECK: [[EXE_MODEL_FN_ID:%.+]] = OpFunction [[V_TY_ID]] None [[FN_TY_ID]]
define spir_kernel void @exe_model() !spirv.ExecutionModel !0 {
entry:
  ret void
}
; CHECK: [[EXE_MODE_FN_ID:%.+]] = OpFunction [[V_TY_ID]] None [[FN_TY_ID]]
; CHECK: OpExecutionMode [[EXE_MODE_FN_ID]] LocalSize 5 5 5
; CHECK: OpExecutionMode [[EXE_MODE_FN_ID]] LocalSizeHint 5 5 5
; CHECK: OpExecutionMode [[EXE_MODE_FN_ID]] SpacingEqual
define spir_kernel void @exe_modes() !spirv.ExecutionMode !1 {
entry:
  ret void
}

!0 = !{!"Vertex"}
!1 = !{!2, !3, !4}
!2 = !{!"LocalSize", i64 5, i64 5, i64 5}
!3 = !{!"LocalSizeHint", i64 5, i64 5, i64 5}
!4 = !{!"SpacingEqual"}
