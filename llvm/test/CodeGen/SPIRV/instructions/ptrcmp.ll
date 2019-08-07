; RUN: llc %s -o - | FileCheck %s

target triple = "spirv32-unknown-unknown"

; CHECK-DAG: OpName [[EQ:%.*]] "test_eq"
; CHECK-DAG: OpName [[NE:%.*]] "test_ne"
; CHECK-DAG: OpName [[ULT:%.*]] "test_ult"
; CHECK-DAG: OpName [[SLT:%.*]] "test_slt"
; CHECK-DAG: OpName [[ULE:%.*]] "test_ule"
; CHECK-DAG: OpName [[SLE:%.*]] "test_sle"
; CHECK-DAG: OpName [[UGT:%.*]] "test_ugt"
; CHECK-DAG: OpName [[SGT:%.*]] "test_sgt"
; CHECK-DAG: OpName [[UGE:%.*]] "test_uge"
; CHECK-DAG: OpName [[SGE:%.*]] "test_sge"

; CHECK: [[EQ]] = OpFunction
; CHECK-NEXT: [[A:%.*]] = OpFunctionParameter
; CHECK-NEXT: [[B:%.*]] = OpFunctionParameter
; CHECK-NEXT: OpLabel
; CHECK-NEXT: [[R:%.*]] = OpPtrEqual {{%.+}} [[A]] [[B]]
; CHECK-NEXT: OpReturnValue [[R]]
; CHECK-NEXT: OpFunctionEnd
define i1 @test_eq(i16* %a, i16* %b) {
  %r = icmp eq i16* %a, %b
  ret i1 %r
}

; CHECK: [[NE]] = OpFunction
; CHECK-NEXT: [[A:%.*]] = OpFunctionParameter
; CHECK-NEXT: [[B:%.*]] = OpFunctionParameter
; CHECK-NEXT: OpLabel
; CHECK-NEXT: [[R:%.*]] = OpPtrNotEqual {{%.+}} [[A]] [[B]]
; CHECK-NEXT: OpReturnValue [[R]]
; CHECK-NEXT: OpFunctionEnd
define i1 @test_ne(i16* %a, i16* %b) {
  %r = icmp ne i16* %a, %b
  ret i1 %r
}

; CHECK: [[SLT]] = OpFunction
; CHECK-NEXT: [[A:%.*]] = OpFunctionParameter
; CHECK-NEXT: [[B:%.*]] = OpFunctionParameter
; CHECK-NEXT: OpLabel
; CHECK: [[R:%.*]] = OpSLessThan {{%.+}} [[A]] [[B]]
; CHECK-NEXT: OpReturnValue [[R]]
; CHECK-NEXT: OpFunctionEnd
define i1 @test_slt(i16* %a, i16* %b) {
  %r = icmp slt i16* %a, %b
  ret i1 %r
}

; CHECK: [[ULT]] = OpFunction
; CHECK-NEXT: [[A:%.*]] = OpFunctionParameter
; CHECK-NEXT: [[B:%.*]] = OpFunctionParameter
; CHECK-NEXT: OpLabel
; CHECK: [[R:%.*]] = OpULessThan {{%.+}} [[A]] [[B]]
; CHECK-NEXT: OpReturnValue [[R]]
; CHECK-NEXT: OpFunctionEnd
define i1 @test_ult(i16* %a, i16* %b) {
  %r = icmp ult i16* %a, %b
  ret i1 %r
}

; CHECK: [[ULE]] = OpFunction
; CHECK-NEXT: [[A:%.*]] = OpFunctionParameter
; CHECK-NEXT: [[B:%.*]] = OpFunctionParameter
; CHECK-NEXT: OpLabel
; CHECK: [[R:%.*]] = OpULessThanEqual {{%.+}} [[A]] [[B]]
; CHECK-NEXT: OpReturnValue [[R]]
; CHECK-NEXT: OpFunctionEnd
define i1 @test_ule(i16* %a, i16* %b) {
  %r = icmp ule i16* %a, %b
  ret i1 %r
}

; CHECK: [[SLE]] = OpFunction
; CHECK-NEXT: [[A:%.*]] = OpFunctionParameter
; CHECK-NEXT: [[B:%.*]] = OpFunctionParameter
; CHECK-NEXT: OpLabel
; CHECK: [[R:%.*]] = OpSLessThanEqual {{%.+}} [[A]] [[B]]
; CHECK-NEXT: OpReturnValue [[R]]
; CHECK-NEXT: OpFunctionEnd
define i1 @test_sle(i16* %a, i16* %b) {
  %r = icmp sle i16* %a, %b
  ret i1 %r
}

; CHECK: [[UGT]] = OpFunction
; CHECK-NEXT: [[A:%.*]] = OpFunctionParameter
; CHECK-NEXT: [[B:%.*]] = OpFunctionParameter
; CHECK-NEXT: OpLabel
; CHECK: [[R:%.*]] = OpUGreaterThan {{%.+}} [[A]] [[B]]
; CHECK-NEXT: OpReturnValue [[R]]
; CHECK-NEXT: OpFunctionEnd
define i1 @test_ugt(i16* %a, i16* %b) {
  %r = icmp ugt i16* %a, %b
  ret i1 %r
}

; CHECK: [[SGT]] = OpFunction
; CHECK-NEXT: [[A:%.*]] = OpFunctionParameter
; CHECK-NEXT: [[B:%.*]] = OpFunctionParameter
; CHECK-NEXT: OpLabel
; CHECK: [[R:%.*]] = OpSGreaterThan {{%.+}} [[A]] [[B]]
; CHECK-NEXT: OpReturnValue [[R]]
; CHECK-NEXT: OpFunctionEnd
define i1 @test_sgt(i16* %a, i16* %b) {
  %r = icmp sgt i16* %a, %b
  ret i1 %r
}

; CHECK: [[UGE]] = OpFunction
; CHECK-NEXT: [[A:%.*]] = OpFunctionParameter
; CHECK-NEXT: [[B:%.*]] = OpFunctionParameter
; CHECK-NEXT: OpLabel
; CHECK: [[R:%.*]] = OpUGreaterThanEqual {{%.+}} [[A]] [[B]]
; CHECK-NEXT: OpReturnValue [[R]]
; CHECK-NEXT: OpFunctionEnd
define i1 @test_uge(i16* %a, i16* %b) {
  %r = icmp uge i16* %a, %b
  ret i1 %r
}

; CHECK: [[SGE]] = OpFunction
; CHECK-NEXT: [[A:%.*]] = OpFunctionParameter
; CHECK-NEXT: [[B:%.*]] = OpFunctionParameter
; CHECK-NEXT: OpLabel
; CHECK: [[R:%.*]] = OpSGreaterThanEqual {{%.+}} [[A]] [[B]]
; CHECK-NEXT: OpReturnValue [[R]]
; CHECK-NEXT: OpFunctionEnd
define i1 @test_sge(i16* %a, i16* %b) {
  %r = icmp sge i16* %a, %b
  ret i1 %r
}
