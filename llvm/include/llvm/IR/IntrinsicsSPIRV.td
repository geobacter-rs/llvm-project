//===- IntrinsicsSPIRV.td - Defines SPIRV intrinsics -------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines all of the SPIRV-specific intrinsics.
//
//===----------------------------------------------------------------------===//

let TargetPrefix = "spirv" in {
  // Readonly builtins, to present a familiar interface to frontends.
  def int_spirv_local_invocation_id :
    GCCBuiltin<"__builtin_spirv_local_invocation_id">,
    Intrinsic<[llvm_v3i32_ty], [], [IntrNoMem, IntrSpeculatable]>;
  def int_spirv_local_invocation_index :
    GCCBuiltin<"__builtin_spirv_local_invocation_index">,
    Intrinsic<[llvm_i32_ty], [], [IntrNoMem, IntrSpeculatable]>;
  // GlobalInvocationId
  def int_spirv_global_invocation_id :
    GCCBuiltin<"__builtin_spirv_global_invocation_id">,
    Intrinsic<[llvm_v3i32_ty], [], [IntrNoMem, IntrSpeculatable]>;
  def int_spirv_instance_id :
    GCCBuiltin<"__builtin_spirv_instance_id">,
    Intrinsic<[llvm_i32_ty], [], [IntrNoMem, IntrSpeculatable]>;
  def int_spirv_invocation_id :
    GCCBuiltin<"__builtin_spirv_invocation_id">,
    Intrinsic<[llvm_i32_ty], [], [IntrNoMem, IntrSpeculatable]>;
  def int_spirv_instance_index :
    GCCBuiltin<"__builtin_spirv_instance_index">,
    Intrinsic<[llvm_i32_ty], [], [IntrNoMem, IntrSpeculatable]>;
  def int_spirv_base_instance :
    GCCBuiltin<"__builtin_spirv_base_instance">,
    Intrinsic<[llvm_i32_ty], [], [IntrNoMem, IntrSpeculatable]>;
  def int_spirv_base_vertex :
    GCCBuiltin<"__builtin_spirv_base_vertex">,
    Intrinsic<[llvm_i32_ty], [], [IntrNoMem, IntrSpeculatable]>;
  def int_spirv_device_index :
    GCCBuiltin<"__builtin_spirv_device_index">,
    Intrinsic<[llvm_i32_ty], [], [IntrNoMem, IntrSpeculatable]>;
  def int_spirv_draw_index :
    GCCBuiltin<"__builtin_spirv_draw_index">,
    Intrinsic<[llvm_i32_ty], [], [IntrNoMem, IntrSpeculatable]>;
  def int_spirv_frag_coord :
    GCCBuiltin<"__builtin_spirv_frag_coord">,
    Intrinsic<[llvm_v4f32_ty], [], [IntrNoMem, IntrSpeculatable]>;
  def int_spirv_num_subgroups :
    GCCBuiltin<"__builtin_spirv_num_subgroups">,
    Intrinsic<[llvm_i32_ty], [], [IntrNoMem, IntrSpeculatable]>;
  def int_spirv_subgroup_id :
    GCCBuiltin<"__builtin_spirv_subgroup_id">,
    Intrinsic<[llvm_i32_ty], [], [IntrNoMem, IntrSpeculatable]>;
  def int_spirv_subgroup_local_invocation_id :
    GCCBuiltin<"__builtin_spirv_subgroup_local_invocation_id">,
    Intrinsic<[llvm_i32_ty], [], [IntrNoMem, IntrSpeculatable]>;
  def int_spirv_subgroup_size :
    GCCBuiltin<"__builtin_spirv_subgroup_size">,
    Intrinsic<[llvm_i32_ty], [], [IntrNoMem, IntrSpeculatable]>;
  def int_spirv_num_workgroups :
    GCCBuiltin<"__builtin_spirv_num_workgroups">,
    Intrinsic<[llvm_v3i32_ty], [], [IntrNoMem, IntrSpeculatable]>;
  def int_spirv_workgroup_id :
    GCCBuiltin<"__builtin_spirv_workgroup_id">,
    Intrinsic<[llvm_v3i32_ty], [], [IntrNoMem, IntrSpeculatable]>;
  def int_spirv_tess_coord :
    GCCBuiltin<"__builtin_spirv_tess_coord">,
    Intrinsic<[llvm_v3f32_ty], [], [IntrNoMem, IntrSpeculatable]>;
  def int_spirv_vertex_index :
    GCCBuiltin<"__builtin_spirv_vertex_index">,
    Intrinsic<[llvm_i32_ty], [], [IntrNoMem, IntrSpeculatable]>;
  def int_spirv_view_index :
    GCCBuiltin<"__builtin_spirv_view_index">,
    Intrinsic<[llvm_i32_ty], [], [IntrNoMem, IntrSpeculatable]>;

  // Instructions
  // discard for frag shaders
  def int_spirv_kill :
    GCCBuiltin<"__builtin_spirv_kill">,
    Intrinsic<[], [], [IntrNoMem, IntrNoReturn]>;
  def int_spirv_control_barrier :
    GCCBuiltin<"__builtin_spirv_control_barrier">,
    Intrinsic<[llvm_i32_ty, llvm_i32_ty, llvm_i32_ty], [],
              [IntrConvergent, ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>, ImmArg<ArgIndex<2>>]>;
}
