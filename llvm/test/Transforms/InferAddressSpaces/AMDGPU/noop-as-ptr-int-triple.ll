; RUN: opt -mtriple=amdgcn-amd-amdhsa -S -o - -infer-address-spaces %s | FileCheck %s

source_filename = "geobacter-cross-codegen.3a1fbbbh-cgu.0"
target datalayout = "e-p:64:64-p1:64:64-p2:32:32-p3:32:32-p4:64:64-p5:32:32-p6:32:32-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-v2048:2048-n32:64-S32-A5-ni:7"
target triple = "amdgcn-amd-amdhsa-amdgiz"

%"generic_array::ArrayBuilder<u8, typenum::uint::UInt<typenum::uint::UInt<typenum::uint::UInt<typenum::uint::UInt<typenum::uint::UInt<typenum::uint::UInt<typenum::uint::UInt<typenum::uint::UTerm, typenum::bit::B1>, typenum::bit::B0>, typenum::bit::B0>, typenum::bit::B0>, typenum::bit::B0>, typenum::bit::B0>, typenum::bit::B0>>.20.49.72.95.118.141.240.282.415.434.453.472.491.510.529.548.567.605.662.681.700.772" = type { i64, %"core::mem::maybe_uninit::MaybeUninit<generic_array::GenericArray<u8, typenum::uint::UInt<typenum::uint::UInt<typenum::uint::UInt<typenum::uint::UInt<typenum::uint::UInt<typenum::uint::UInt<typenum::uint::UInt<typenum::uint::UTerm, typenum::bit::B1>, typenum::bit::B0>, typenum::bit::B0>, typenum::bit::B0>, typenum::bit::B0>, typenum::bit::B0>, typenum::bit::B0>>>.19.48.71.94.117.140.239.281.414.433.452.471.490.509.528.547.566.604.661.680.699.771" }
%"core::mem::maybe_uninit::MaybeUninit<generic_array::GenericArray<u8, typenum::uint::UInt<typenum::uint::UInt<typenum::uint::UInt<typenum::uint::UInt<typenum::uint::UInt<typenum::uint::UInt<typenum::uint::UInt<typenum::uint::UTerm, typenum::bit::B1>, typenum::bit::B0>, typenum::bit::B0>, typenum::bit::B0>, typenum::bit::B0>, typenum::bit::B0>, typenum::bit::B0>>>.19.48.71.94.117.140.239.281.414.433.452.471.490.509.528.547.566.604.661.680.699.771" = type { [64 x i8] }
%"unwind::libunwind::_Unwind_Exception.9.38.61.84.107.130.229.271.404.423.442.461.480.499.518.537.556.594.651.670.689.769" = type { i64, void (i32, %"unwind::libunwind::_Unwind_Exception.9.38.61.84.107.130.229.271.404.423.442.461.480.499.518.537.556.594.651.670.689.769"*)*, [6 x i64] }
%"unwind::libunwind::_Unwind_Context.10.39.62.85.108.131.230.272.405.424.443.462.481.500.519.538.557.595.652.671.690.770" = type { [0 x i8] }

; CHECK-LABEL: @_ZN21geobacter_runtime_amd6module13launch_kernel17hcfcc6e08d5049874E
define amdgpu_kernel void @_ZN21geobacter_runtime_amd6module13launch_kernel17hcfcc6e08d5049874E() unnamed_addr #0 personality i32 (i32, i32, i64, %"unwind::libunwind::_Unwind_Exception.9.38.61.84.107.130.229.271.404.423.442.461.480.499.518.537.556.594.651.670.689.769"*, %"unwind::libunwind::_Unwind_Context.10.39.62.85.108.131.230.272.405.424.443.462.481.500.519.538.557.595.652.671.690.770"*)* @_ZN22geobacter_runtime_core7codegen8stubbing5stubs19rust_eh_personality17h78a5067dc11d909aE {
start:
  br i1 undef, label %bb7.i, label %bb5.i

bb5.i:                                            ; preds = %start
  %0 = addrspacecast %"generic_array::ArrayBuilder<u8, typenum::uint::UInt<typenum::uint::UInt<typenum::uint::UInt<typenum::uint::UInt<typenum::uint::UInt<typenum::uint::UInt<typenum::uint::UInt<typenum::uint::UTerm, typenum::bit::B1>, typenum::bit::B0>, typenum::bit::B0>, typenum::bit::B0>, typenum::bit::B0>, typenum::bit::B0>, typenum::bit::B0>>.20.49.72.95.118.141.240.282.415.434.453.472.491.510.529.548.567.605.662.681.700.772" addrspace(5)* undef to %"generic_array::ArrayBuilder<u8, typenum::uint::UInt<typenum::uint::UInt<typenum::uint::UInt<typenum::uint::UInt<typenum::uint::UInt<typenum::uint::UInt<typenum::uint::UInt<typenum::uint::UTerm, typenum::bit::B1>, typenum::bit::B0>, typenum::bit::B0>, typenum::bit::B0>, typenum::bit::B0>, typenum::bit::B0>, typenum::bit::B0>>.20.49.72.95.118.141.240.282.415.434.453.472.491.510.529.548.567.605.662.681.700.772"*
  %1 = ptrtoint %"generic_array::ArrayBuilder<u8, typenum::uint::UInt<typenum::uint::UInt<typenum::uint::UInt<typenum::uint::UInt<typenum::uint::UInt<typenum::uint::UInt<typenum::uint::UInt<typenum::uint::UTerm, typenum::bit::B1>, typenum::bit::B0>, typenum::bit::B0>, typenum::bit::B0>, typenum::bit::B0>, typenum::bit::B0>, typenum::bit::B0>>.20.49.72.95.118.141.240.282.415.434.453.472.491.510.529.548.567.605.662.681.700.772"* %0 to i64
  %2 = inttoptr i64 %1 to i64*
; CHECK: store i64 undef, i64 addrspace(5)*
  store i64 undef, i64* %2, align 8
  unreachable

bb7.i:                                            ; preds = %start
  ret void
}

declare i32 @_ZN22geobacter_runtime_core7codegen8stubbing5stubs19rust_eh_personality17h78a5067dc11d909aE(i32, i32, i64, %"unwind::libunwind::_Unwind_Exception.9.38.61.84.107.130.229.271.404.423.442.461.480.499.518.537.556.594.651.670.689.769"*, %"unwind::libunwind::_Unwind_Context.10.39.62.85.108.131.230.272.405.424.443.462.481.500.519.538.557.595.652.671.690.770"*) unnamed_addr #0

attributes #0 = { "uniform-work-group-size"="true" }

!llvm.module.flags = !{!0}

!0 = !{i32 2, !"RtLibUseGOT", i32 1}
