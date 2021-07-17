; RUN: opt < %s -S -mtriple=amdgcn-amd-amdhsa-amdgiz5 -early-cse | FileCheck %s

source_filename = "geobacter-cross-codegen.3a1fbbbh-cgu.0"
target datalayout = "e-p:64:64-p1:64:64-p2:32:32-p3:32:32-p4:64:64-p5:32:32-p6:32:32-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-v2048:2048-n32:64-S32-A5-ni:7"
target triple = "amdgcn-amd-amdhsa-amdgiz"

%"unwind::libunwind::_Unwind_Exception.25.145.265.295.308.416.548.560.572.584.596.608.620.632.644.656.668.692.704.716.728.752.764.788.800.823.856.867.1023" = type { i64, void (i32, %"unwind::libunwind::_Unwind_Exception.25.145.265.295.308.416.548.560.572.584.596.608.620.632.644.656.668.692.704.716.728.752.764.788.800.823.856.867.1023"*)*, [6 x i64] }
%"unwind::libunwind::_Unwind_Context.26.146.266.296.309.417.549.561.573.585.597.609.621.633.645.657.669.693.705.717.729.753.765.789.801.824.857.868.1024" = type { [0 x i8] }

; CHECK-LABEL: @"_ZN42_$LT$$RF$T$u20$as$u20$core..fmt..Debug$GT$3fmt17h78f04206d1e4cf0cE"
define dso_local void @"_ZN42_$LT$$RF$T$u20$as$u20$core..fmt..Debug$GT$3fmt17h78f04206d1e4cf0cE"() unnamed_addr #0 personality i32 (i32, i32, i64, %"unwind::libunwind::_Unwind_Exception.25.145.265.295.308.416.548.560.572.584.596.608.620.632.644.656.668.692.704.716.728.752.764.788.800.823.856.867.1023"*, %"unwind::libunwind::_Unwind_Context.26.146.266.296.309.417.549.561.573.585.597.609.621.633.645.657.669.693.705.717.729.753.765.789.801.824.857.868.1024"*)* @_ZN22geobacter_runtime_core7codegen8stubbing5stubs19rust_eh_personality17h6cbb7a1197e18773E {
start:
  %buf.i.i.i = alloca [128 x i8], align 1, addrspace(5)
  %_18.0.i.i.i = bitcast [128 x i8] addrspace(5)* %buf.i.i.i to [0 x i8] addrspace(5)*
  %0 = addrspacecast [0 x i8] addrspace(5)* %_18.0.i.i.i to [0 x i8]*
  %1 = bitcast [0 x i8]* %0 to i8*
  %2 = getelementptr i8, i8* %1, i64 128
  %_12.i.i65.i.i.i = icmp eq i8* %1, %2
  call void @llvm.assume(i1 %_12.i.i65.i.i.i)
  unreachable
}

; Function Attrs: nounwind willreturn
declare void @llvm.assume(i1) #1

declare i32 @_ZN22geobacter_runtime_core7codegen8stubbing5stubs19rust_eh_personality17h6cbb7a1197e18773E(i32, i32, i64, %"unwind::libunwind::_Unwind_Exception.25.145.265.295.308.416.548.560.572.584.596.608.620.632.644.656.668.692.704.716.728.752.764.788.800.823.856.867.1023"*, %"unwind::libunwind::_Unwind_Context.26.146.266.296.309.417.549.561.573.585.597.609.621.633.645.657.669.693.705.717.729.753.765.789.801.824.857.868.1024"*) unnamed_addr #0

attributes #0 = { "uniform-work-group-size"="true" }
attributes #1 = { nounwind willreturn }

!llvm.module.flags = !{!0}

!0 = !{i32 2, !"Debug Info Version", i32 3}
