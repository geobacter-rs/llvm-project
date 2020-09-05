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
    Intrinsic<[llvm_v3i32_ty], [], [IntrNoMem, IntrSpeculatable]>,
    GCCBuiltin<"__builtin_spirv_local_invocation_id">;
  def int_spirv_local_invocation_index :
    Intrinsic<[llvm_i32_ty], [], [IntrNoMem, IntrSpeculatable]>,
    GCCBuiltin<"__builtin_spirv_local_invocation_index">;
  // GlobalInvocationId
  def int_spirv_global_invocation_id :
    Intrinsic<[llvm_v3i32_ty], [], [IntrNoMem, IntrSpeculatable]>,
    GCCBuiltin<"__builtin_spirv_global_invocation_id">;
  def int_spirv_instance_id :
    Intrinsic<[llvm_i32_ty], [], [IntrNoMem, IntrSpeculatable]>,
    GCCBuiltin<"__builtin_spirv_instance_id">;
  def int_spirv_invocation_id :
    Intrinsic<[llvm_i32_ty], [], [IntrNoMem, IntrSpeculatable]>,
    GCCBuiltin<"__builtin_spirv_invocation_id">;
  def int_spirv_instance_index :
    Intrinsic<[llvm_i32_ty], [], [IntrNoMem, IntrSpeculatable]>,
    GCCBuiltin<"__builtin_spirv_instance_index">;
  def int_spirv_base_instance :
    Intrinsic<[llvm_i32_ty], [], [IntrNoMem, IntrSpeculatable]>,
    GCCBuiltin<"__builtin_spirv_base_instance">;
  def int_spirv_base_vertex :
    Intrinsic<[llvm_i32_ty], [], [IntrNoMem, IntrSpeculatable]>,
    GCCBuiltin<"__builtin_spirv_base_vertex">;
  def int_spirv_device_index :
    Intrinsic<[llvm_i32_ty], [], [IntrNoMem, IntrSpeculatable]>,
    GCCBuiltin<"__builtin_spirv_device_index">;
  def int_spirv_draw_index :
    Intrinsic<[llvm_i32_ty], [], [IntrNoMem, IntrSpeculatable]>,
    GCCBuiltin<"__builtin_spirv_draw_index">;
  def int_spirv_frag_coord :
    Intrinsic<[llvm_v4f32_ty], [], [IntrNoMem, IntrSpeculatable]>,
    GCCBuiltin<"__builtin_spirv_frag_coord">;
  def int_spirv_num_subgroups :
    Intrinsic<[llvm_i32_ty], [], [IntrNoMem, IntrSpeculatable]>,
    GCCBuiltin<"__builtin_spirv_num_subgroups">;
  def int_spirv_subgroup_id :
    Intrinsic<[llvm_i32_ty], [], [IntrNoMem, IntrSpeculatable]>,
    GCCBuiltin<"__builtin_spirv_subgroup_id">;
  def int_spirv_subgroup_local_invocation_id :
    Intrinsic<[llvm_i32_ty], [], [IntrNoMem, IntrSpeculatable]>,
    GCCBuiltin<"__builtin_spirv_subgroup_local_invocation_id">;
  def int_spirv_subgroup_size :
    Intrinsic<[llvm_i32_ty], [], [IntrNoMem, IntrSpeculatable]>,
    GCCBuiltin<"__builtin_spirv_subgroup_size">;
  def int_spirv_num_workgroups :
    Intrinsic<[llvm_v3i32_ty], [], [IntrNoMem, IntrSpeculatable]>,
    GCCBuiltin<"__builtin_spirv_num_workgroups">;
  def int_spirv_workgroup_id :
    Intrinsic<[llvm_v3i32_ty], [], [IntrNoMem, IntrSpeculatable]>,
    GCCBuiltin<"__builtin_spirv_workgroup_id">;
  def int_spirv_tess_coord :
    Intrinsic<[llvm_v3f32_ty], [], [IntrNoMem, IntrSpeculatable]>,
    GCCBuiltin<"__builtin_spirv_tess_coord">;
  def int_spirv_vertex_index :
    Intrinsic<[llvm_i32_ty], [], [IntrNoMem, IntrSpeculatable]>,
    GCCBuiltin<"__builtin_spirv_vertex_index">;
  def int_spirv_view_index :
    Intrinsic<[llvm_i32_ty], [], [IntrNoMem, IntrSpeculatable]>,
    GCCBuiltin<"__builtin_spirv_view_index">;

  // Instructions
  // discard for frag shaders
  def int_spirv_kill :
    Intrinsic<[], [], [IntrNoMem, IntrNoReturn]>,
    GCCBuiltin<"__builtin_spirv_kill">;
  def int_spirv_control_barrier :
    Intrinsic<[llvm_i32_ty, llvm_i32_ty, llvm_i32_ty], [],
              [IntrConvergent, ImmArg<0>, ImmArg<1>, ImmArg<2>]>,
    GCCBuiltin<"__builtin_spirv_control_barrier">;
}
