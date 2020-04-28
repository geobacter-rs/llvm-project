//===-- SPIRVEnums.h - SPIR-V Enums and Related Functions -------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines macros to generate all SPIR-V enum types.
// The macros also define helper functions for each enum such as
// getEnumName(Enum e) and getEnumCapabilities(Enum e) which can be
// auto-generated in SPIRVEnums.cpp and used when printing SPIR-V in
// textual form, or checking for required extensions, versions etc.
//
// TODO Auto-genrate this from SPIR-V header files
//
// If the names of any enums change in this file, SPIRVEnums.td and
// InstPrinter/SPIRVInstPrinter.h must also be updated, as the name of the enums
// is used to select the correct assembly printing methods.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SPIRV_ENUMS_H
#define LLVM_LIB_TARGET_SPIRV_ENUMS_H

#include "SPIRVExtensions.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/StringRef.h"
#include <string>
#include <vector>

namespace llvm {
class Metadata;
}

// Macros to define an enum and the functions to return its name and its
// required capabilities, extensions, and SPIR-V versions

#define LIST(...) __VA_ARGS__

#define MAKE_ENUM(Enum, Var, Val, Caps, Exts, MinVer, MaxVer) Var = Val,

#define MAKE_NAME_CASE(Enum, Var, Val, Caps, Exts, MinVer, MaxVer)             \
  case Enum::Var:                                                              \
    return #Var;

#define MAKE_RNAME_CASE(Enum, Var, Val, Caps, Exts, MinVer, MaxVer)            \
  .Case(#Var, {Enum::Var})

#define MAKE_MASK_ENUM_NAME_CASE(Enum, Var, Val, Caps, Exts, MinVer, MaxVer)   \
  if (e == Enum::Var) {                                                        \
    return #Var;                                                               \
  } else if (((uint32_t)Enum::Var != 0) &&                                     \
             ((uint32_t)e & (uint32_t)Enum::Var)) {                            \
    nameString += sep + #Var;                                                  \
    sep = "|";                                                                 \
  }

#define MAKE_BUILTIN_LINK_CASE(Enum, Var, Val, Caps, Exts, MinVer, MaxVer)     \
  case Enum::Var:                                                              \
    return "__spirv_BuiltIn" #Var;

#define DEF_ENUM(EnumName, DefEnumCommand)                                     \
  enum class EnumName : uint32_t { DefEnumCommand(EnumName, MAKE_ENUM) };

#define DEF_NAME_FUNC_HEADER(EnumName)                                         \
  std::string get##EnumName##Name(EnumName e);

#define DEF_RNAME_FUNC_HEADER(EnumName)                                        \
  llvm::Optional<EnumName> get##EnumName##FromStr(llvm::StringRef Name);

#define DEF_DECODE_MD_FUNC_HEADER(EnumName)                                    \
  llvm::Optional<EnumName> decode##EnumName##MD(const llvm::Metadata *MD);

// Use this for enums that can only take a single value
#define DEF_NAME_FUNC_BODY(EnumName, DefEnumCommand)                           \
  std::string get##EnumName##Name(EnumName e) {                                \
    switch (e) { DefEnumCommand(EnumName, MAKE_NAME_CASE) }                    \
    return std::string();                                                      \
  }

#define DEF_RNAME_FUNC_BODY(EnumName, DefEnumCommand)                          \
  llvm::Optional<EnumName> get##EnumName##FromStr(llvm::StringRef Name) {      \
    return llvm::StringSwitch<llvm::Optional<EnumName>>(Name)                  \
        DefEnumCommand(EnumName, MAKE_RNAME_CASE)                              \
            .Default(llvm::None);                                              \
  }

#define DEF_DECODE_MD_FUNC_BODY(EnumName, DefEnumCommand)                      \
  llvm::Optional<EnumName> decode##EnumName##MD(const llvm::Metadata *MD) {    \
    if (!MD) {                                                                 \
      return llvm::None;                                                       \
    }                                                                          \
    if (auto *MDStr = llvm::dyn_cast<llvm::MDString>(MD)) {                    \
      return get##EnumName##FromStr(MDStr->getString());                       \
    }                                                                          \
    if (auto *C = llvm::mdconst::dyn_extract<llvm::ConstantInt>(MD)) {         \
      const auto Value = static_cast<EnumName>(C->getLimitedValue(~0ULL));     \
      if (!get##EnumName##Name(Value).empty()) {                               \
        return {Value};                                                        \
      }                                                                        \
    }                                                                          \
    return llvm::None;                                                         \
  }

// Use this for bitmasks that can take multiple values e.g. DontInline|Const
#define DEF_MASK_NAME_FUNC_BODY(EnumName, DefEnumCommand)                      \
  std::string get##EnumName##Name(EnumName e) {                                \
    std::string nameString = "";                                               \
    std::string sep = "";                                                      \
    DefEnumCommand(EnumName, MAKE_MASK_ENUM_NAME_CASE);                        \
    return nameString;                                                         \
  }

#define DEF_BUILTIN_LINK_STR_FUNC_BODY()                                       \
  const char *getLinkStrForBuiltIn(BuiltIn builtIn) {                          \
    switch (builtIn) { DEF_BuiltIn(BuiltIn, MAKE_BUILTIN_LINK_CASE) }          \
    return "UNKNOWN_BUILTIN";                                                  \
  }

#define GEN_ENUM_HEADER(EnumName)                                              \
  DEF_ENUM(EnumName, DEF_##EnumName)                                           \
  DEF_NAME_FUNC_HEADER(EnumName)                                               \
  DEF_RNAME_FUNC_HEADER(EnumName)                                              \
  DEF_DECODE_MD_FUNC_HEADER(EnumName)

// Use this for enums that can only take a single value
#define GEN_ENUM_IMPL(EnumName)                                                \
  DEF_NAME_FUNC_BODY(EnumName, DEF_##EnumName)                                 \
  DEF_RNAME_FUNC_BODY(EnumName, DEF_##EnumName)                                \
  DEF_DECODE_MD_FUNC_BODY(EnumName, DEF_##EnumName)

// Use this for bitmasks that can take multiple values e.g. DontInline|Const
#define GEN_MASK_ENUM_IMPL(EnumName)                                           \
  DEF_MASK_NAME_FUNC_BODY(EnumName, DEF_##EnumName)

#define GEN_INSTR_PRINTER_IMPL(EnumName)                                       \
  void SPIRVInstPrinter::print##EnumName(const MCInst *MI, unsigned OpNo,      \
                                         raw_ostream &O) {                     \
    if (OpNo < MI->getNumOperands()) {                                         \
      EnumName e = static_cast<EnumName>(MI->getOperand(OpNo).getImm());       \
      O << get##EnumName##Name(e);                                             \
    }                                                                          \
  }

//===----------------------------------------------------------------------===//
// The actual enum definitions are added below
//
// Call GEN_ENUM_HEADER here in the header.
//
// Call GEN_ENUM_IMPL or GEN_MASK_ENUM_IMPL in SPIRVEnums.cpp, depending on
// whether the enum can only take a single value, or whether it can be a bitmask
// of multiple values e.g. FunctionControl which can be DontInline|Const.
//
// Call GEN_INSTR_PRINTER_IMPL in SPIRVInstPrinter.cpp, and ensure the
// corresponding operand printing function declaration is in SPIRVInstPrinter.h,
// with the correct enum name in SPIRVEnums.td
//
// Syntax for each line is:
//    X(N, Name, IdNum, Capabilities, Extensions, MinVer, MaxVer) \
// Capabilities and Extensions must be either empty brackets {}, a single
// bracketed capability e.g. {Matrix}, or if multiple capabilities are required,
// use the LIST macro:
//    LIST({SampledBuffer, Matrix})
// MinVer and MaxVer are 32 bit integers in the format 0|Major|Minor|0 e.g
// SPIR-V v1.3 = 0x00010300. Using 0 means unspecified.
//
// Each enum def must fit on a  single line, so additional macros are sometimes
// used for defining capabilities with long names.
//===----------------------------------------------------------------------===//

#define SK(Suff) SPV_KHR_##Suff

#define DRAW_PARAMS SPV_KHR_shader_draw_parameters
#define SPV_16_BIT SPV_KHR_16bit_storeage
#define SPV_VAR_PTR SPV_KHR_variable_pointers
#define SPV_PDC SPV_KHR_post_depth_coverage
#define SPV_FLT_CTRL SPV_KHR_float_controls

#define DEF_Capability(N, X)                                                   \
  X(N, Matrix, 0, {}, {}, 0, 0)                                                \
  X(N, Shader, 1, {Capability::Matrix}, {}, 0, 0)                              \
  X(N, Geometry, 2, {Capability::Shader}, {}, 0, 0)                            \
  X(N, Tessellation, 3, {Capability::Shader}, {}, 0, 0)                        \
  X(N, Addresses, 4, {}, {}, 0, 0)                                             \
  X(N, Linkage, 5, {}, {}, 0, 0)                                               \
  X(N, Kernel, 6, {}, {}, 0, 0)                                                \
  X(N, Vector16, 7, {Capability::Kernel}, {}, 0, 0)                            \
  X(N, Float16Buffer, 8, {Capability::Kernel}, {}, 0, 0)                       \
  X(N, Float16, 9, {}, {}, 0, 0)                                               \
  X(N, Float64, 10, {}, {}, 0, 0)                                              \
  X(N, Int64, 11, {}, {}, 0, 0)                                                \
  X(N, Int64Atomics, 12, {Capability::Int64}, {}, 0, 0)                        \
  X(N, ImageBasic, 13, {Capability::Kernel}, {}, 0, 0)                         \
  X(N, ImageReadWrite, 14, {Capability::ImageBasic}, {}, 0, 0)                 \
  X(N, ImageMipmap, 15, {Capability::ImageBasic}, {}, 0, 0)                    \
  X(N, Pipes, 17, {Capability::Kernel}, {}, 0, 0)                              \
  X(N, Groups, 18, {}, {}, 0, 0)                                               \
  X(N, DeviceEnqueue, 19, {}, {}, 0, 0)                                        \
  X(N, LiteralSampler, 20, {Capability::Kernel}, {}, 0, 0)                     \
  X(N, AtomicStorage, 21, {Capability::Shader}, {}, 0, 0)                      \
  X(N, Int16, 22, {}, {}, 0, 0)                                                \
  X(N, TessellationPointSize, 23, {Capability::Tessellation}, {}, 0, 0)        \
  X(N, GeometryPointSize, 24, {Capability::Geometry}, {}, 0, 0)                \
  X(N, ImageGatherExtended, 25, {Capability::Shader}, {}, 0, 0)                \
  X(N, StorageImageMultisample, 27, {Capability::Shader}, {}, 0, 0)            \
  X(N, UniformBufferArrayDynamicIndexing, 28, {Capability::Shader}, {}, 0, 0)  \
  X(N, SampledImageArrayDymnamicIndexing, 29, {Capability::Shader}, {}, 0, 0)  \
  X(N, ClipDistance, 32, {Capability::Shader}, {}, 0, 0)                       \
  X(N, CullDistance, 33, {Capability::Shader}, {}, 0, 0)                       \
  X(N, ImageCubeArray, 34, {Capability::SampledCubeArray}, {}, 0, 0)           \
  X(N, SampleRateShading, 35, {Capability::Shader}, {}, 0, 0)                  \
  X(N, ImageRect, 36, {Capability::SampledRect}, {}, 0, 0)                     \
  X(N, SampledRect, 37, {Capability::Shader}, {}, 0, 0)                        \
  X(N, GenericPointer, 38, {Capability::Addresses}, {}, 0, 0)                  \
  X(N, Int8, 39, {}, {}, 0, 0)                                                 \
  X(N, InputAttachment, 40, {Capability::Shader}, {}, 0, 0)                    \
  X(N, SparseResidency, 41, {Capability::Shader}, {}, 0, 0)                    \
  X(N, MinLod, 42, {Capability::Shader}, {}, 0, 0)                             \
  X(N, Sampled1D, 43, {}, {}, 0, 0)                                            \
  X(N, Image1D, 44, {Capability::Sampled1D}, {}, 0, 0)                         \
  X(N, SampledCubeArray, 45, {Capability::Shader}, {}, 0, 0)                   \
  X(N, SampledBuffer, 46, {}, {}, 0, 0)                                        \
  X(N, ImageBuffer, 47, {Capability::SampledBuffer}, {}, 0, 0)                 \
  X(N, ImageMSArray, 48, {Capability::Shader}, {}, 0, 0)                       \
  X(N, StorageImageExtendedFormats, 49, {Capability::Shader}, {}, 0, 0)        \
  X(N, ImageQuery, 50, {Capability::Shader}, {}, 0, 0)                         \
  X(N, DerivativeControl, 51, {Capability::Shader}, {}, 0, 0)                  \
  X(N, InterpolationFunction, 52, {Capability::Shader}, {}, 0, 0)              \
  X(N, TransformFeedback, 53, {Capability::Shader}, {}, 0, 0)                  \
  X(N, GeometryStreams, 54, {Capability::Geometry}, {}, 0, 0)                  \
  X(N, StorageImageReadWithoutFormat, 55, {Capability::Shader}, {}, 0, 0)      \
  X(N, StorageImageWriteWithoutFormat, 56, {Capability::Shader}, {}, 0, 0)     \
  X(N, MultiViewport, 57, {Capability::Geometry}, {}, 0, 0)                    \
  X(N, SubgroupDispatch, 58, {Capability::DeviceEnqueue}, {}, 0x10100, 0)      \
  X(N, NamedBarrier, 59, {Capability::Kernel}, {}, 0x10100, 0)                 \
  X(N, PipeStorage, 60, {Capability::Pipes}, {}, 0x10100, 0)                   \
  X(N, GroupNonUniform, 61, {}, {}, 0x10300, 0)                                \
  X(N, GroupNonUniformVote, 62, {Capability::GroupNonUniform}, {}, 0x10300, 0) \
  X(N, GroupNonUniformArithmetic, 63, {Capability::GroupNonUniform}, {},       \
    0x10300, 0)                                                                \
  X(N, GroupNonUniformBallot, 64, {Capability::GroupNonUniform}, {}, 0x10300,  \
    0)                                                                         \
  X(N, GroupNonUniformShuffle, 65, {Capability::GroupNonUniform}, {}, 0x10300, \
    0)                                                                         \
  X(N, GroupNonUniformShuffleRelative, 66, {Capability::GroupNonUniform}, {},  \
    0x10300, 0)                                                                \
  X(N, GroupNonUniformClustered, 67, {Capability::GroupNonUniform}, {},        \
    0x10300, 0)                                                                \
  X(N, GroupNonUniformQuad, 68, {Capability::GroupNonUniform}, {}, 0x10300, 0) \
  X(N, SubgroupBallotKHR, 4423, {}, {SPV_KHR_shader_ballot}, 0, 0)             \
  X(N, DrawParameters, 4427, {Capability::Shader}, {DRAW_PARAMS}, 0x10300, 0)  \
  X(N, SubgroupVoteKHR, 4431, {}, {SPV_KHR_subgroup_vote}, 0, 0)               \
  X(N, StorageBuffer16BitAccess, 4433, {}, {SPV_16_BIT}, 0x10300, 0)           \
  X(N, StorageUniform16, 4434, {Capability::StorageBuffer16BitAccess},         \
    {SPV_16_BIT}, 0x10300, 0)                                                  \
  X(N, StoragePushConstant16, 4435, {}, {SPV_16_BIT}, 0x10300, 0)              \
  X(N, StorageInputOutput16, 4436, {}, {SPV_16_BIT}, 0x10300, 0)               \
  X(N, DeviceGroup, 4437, {}, {SPV_KHR_device_group}, 0x10300, 0)              \
  X(N, MultiView, 4439, {Capability::Shader}, {SPV_KHR_multiview}, 0x10300, 0) \
  X(N, VariablePointersStorageBuffer, 4441, {Capability::Shader},              \
    {SPV_VAR_PTR}, 0x10300, 0)                                                 \
  X(N, VariablePointers, 4442, {Capability::VariablePointersStorageBuffer},    \
    {SPV_VAR_PTR}, 0x10300, 0)                                                 \
  X(N, AtomicStorageOps, 4445, {}, {SPV_KHR_shader_atomic_counter_ops}, 0, 0)  \
  X(N, SampleMaskPostDepthCoverage, 4447, {}, {SPV_PDC}, 0, 0)                 \
  X(N, StorageBuffer8BitAccess, 4448, {}, {SPV_KHR_8bit_storage}, 0, 0)        \
  X(N, UniformAndStorageBuffer8BitAccess, 4449,                                \
    {Capability::StorageBuffer8BitAccess}, {SPV_KHR_8bit_storage}, 0, 0)       \
  X(N, StoragePushConstant8, 4450, {}, {SPV_KHR_8bit_storage}, 0, 0)           \
  X(N, DenormPreserve, 4464, {}, {SPV_FLT_CTRL}, 0x10400, 0)                   \
  X(N, DenormFlushToZero, 4465, {}, {SPV_FLT_CTRL}, 0x10400, 0)                \
  X(N, SignedZeroInfNanPreserve, 4466, {}, {SPV_FLT_CTRL}, 0x10400, 0)         \
  X(N, RoundingModeRTE, 4467, {}, {SPV_FLT_CTRL}, 0x10400, 0)                  \
  X(N, RoundingModeRTZ, 4468, {}, {SPV_FLT_CTRL}, 0x10400, 0)                  \
  X(N, Float16ImageAMD, 5008, {Capability::Shader}, {}, 0, 0)                  \
  X(N, ImageGatherBiasLodAMD, 5009, {Capability::Shader}, {}, 0, 0)            \
  X(N, FragmentMaskAMD, 5010, {Capability::Shader}, {}, 0, 0)                  \
  X(N, StencilExportEXT, 5013, {Capability::Shader}, {}, 0, 0)                 \
  X(N, ImageReadWriteLodAMD, 5015, {Capability::Shader}, {}, 0, 0)             \
  X(N, SampleMaskOverrideCoverageNV, 5249, {Capability::SampleRateShading},    \
    {}, 0, 0)                                                                  \
  X(N, GeometryShaderPassthroughNV, 5251, {Capability::Geometry}, {}, 0, 0)    \
  X(N, ShaderViewportIndexLayerEXT, 5254, {Capability::MultiViewport}, {}, 0,  \
    0)                                                                         \
  X(N, ShaderViewportMaskNV, 5255, {Capability::ShaderViewportIndexLayerEXT},  \
    {}, 0, 0)                                                                  \
  X(N, ShaderStereoViewNV, 5259, {Capability::ShaderViewportMaskNV}, {}, 0, 0) \
  X(N, PerViewAttributesNV, 5260, {Capability::MultiView}, {}, 0, 0)           \
  X(N, FragmentFullyCoveredEXT, 5265, {Capability::Shader}, {}, 0, 0)          \
  X(N, MeshShadingNV, 5266, {Capability::Shader}, {}, 0, 0)                    \
  X(N, ShaderNonUniformEXT, 5301, {Capability::Shader}, {}, 0, 0)              \
  X(N, RuntimeDescriptorArrayEXT, 5302, {Capability::Shader}, {}, 0, 0)        \
  X(N, InputAttachmentArrayDynamicIndexingEXT, 5303,                           \
    {Capability::InputAttachment}, {}, 0, 0)                                   \
  X(N, UniformTexelBufferArrayDynamicIndexingEXT, 5304,                        \
    {Capability::SampledBuffer}, {}, 0, 0)                                     \
  X(N, StorageTexelBufferArrayDynamicIndexingEXT, 5305,                        \
    {Capability::ImageBuffer}, {}, 0, 0)                                       \
  X(N, UniformBufferArrayNonUniformIndexingEXT, 5306,                          \
    {Capability::ShaderNonUniformEXT}, {}, 0, 0)                               \
  X(N, SampledImageArrayNonUniformIndexingEXT, 5307,                           \
    {Capability::ShaderNonUniformEXT}, {}, 0, 0)                               \
  X(N, StorageBufferArrayNonUniformIndexingEXT, 5308,                          \
    {Capability::ShaderNonUniformEXT}, {}, 0, 0)                               \
  X(N, StorageImageArrayNonUniformIndexingEXT, 5309,                           \
    {Capability::ShaderNonUniformEXT}, {}, 0, 0)                               \
  X(N, InputAttachmentArrayNonUniformIndexingEXT, 5310,                        \
    LIST({Capability::InputAttachment, Capability::ShaderNonUniformEXT}), {},  \
    0, 0)                                                                      \
  X(N, UniformTexelBufferArrayNonUniformIndexingEXT, 5311,                     \
    LIST({Capability::SampledBuffer, Capability::ShaderNonUniformEXT}), {}, 0, \
    0)                                                                         \
  X(N, StorageTexelBufferArrayNonUniformIndexingEXT, 5312,                     \
    LIST({Capability::ImageBuffer, Capability::ShaderNonUniformEXT}), {}, 0,   \
    0)                                                                         \
  X(N, RayTracingNV, 5340, {Capability::Shader}, {}, 0, 0)                     \
  X(N, SubgroupShuffleINTEL, 5568, {}, {}, 0, 0)                               \
  X(N, SubgroupBufferBlockIOINTEL, 5569, {}, {}, 0, 0)                         \
  X(N, SubgroupImageBlockIOINTEL, 5570, {}, {}, 0, 0)                          \
  X(N, SubgroupImageMediaBlockIOINTEL, 5579, {}, {}, 0, 0)                     \
  X(N, SubgroupAvcMotionEstimationINTEL, 5696, {}, {}, 0, 0)                   \
  X(N, SubgroupAvcMotionEstimationIntraINTEL, 5697, {}, {}, 0, 0)              \
  X(N, SubgroupAvcMotionEstimationChromaINTEL, 5698, {}, {}, 0, 0)             \
  X(N, GroupNonUniformPartitionedNV, 5297, {}, {}, 0, 0)                       \
  X(N, VulkanMemoryModelKHR, 5345, {}, {}, 0, 0)                               \
  X(N, VulkanMemoryModelDeviceScopeKHR, 5346, {}, {}, 0, 0)                    \
  X(N, ImageFootprintNV, 5282, {}, {}, 0, 0)                                   \
  X(N, FragmentBarycentricNV, 5284, {}, {}, 0, 0)                              \
  X(N, ComputeDerivativeGroupQuadsNV, 5288, {}, {}, 0, 0)                      \
  X(N, ComputeDerivativeGroupLinearNV, 5350, {}, {}, 0, 0)                     \
  X(N, FragmentDensityEXT, 5291, {Capability::Shader}, {}, 0, 0)               \
  X(N, PhysicalStorageBufferAddresses, 5347, {Capability::Shader}, {}, 0, 0)   \
  X(N, CooperativeMatrixNV, 5357, {Capability::Shader}, {}, 0, 0)
GEN_ENUM_HEADER(Capability)

#define DEF_SourceLanguage(N, X)                                               \
  X(N, Unknown, 0, {}, {}, 0, 0)                                               \
  X(N, ESSL, 1, {}, {}, 0, 0)                                                  \
  X(N, GLSL, 2, {}, {}, 0, 0)                                                  \
  X(N, OpenCL_C, 3, {}, {}, 0, 0)                                              \
  X(N, OpenCL_CPP, 4, {}, {}, 0, 0)                                            \
  X(N, HLSL, 5, {}, {}, 0, 0)
GEN_ENUM_HEADER(SourceLanguage)

#define DEF_AddressingModel(N, X)                                              \
  X(N, Logical, 0, {}, {}, 0, 0)                                               \
  X(N, Physical32, 1, {Capability::Addresses}, {}, 0, 0)                       \
  X(N, Physical64, 2, {Capability::Addresses}, {}, 0, 0)                       \
  X(N, PhysicalStorageBuffer64, 5348,                                          \
    {Capability::PhysicalStorageBufferAddresses}, {}, 0, 0)
GEN_ENUM_HEADER(AddressingModel)

#define DEF_ExecutionModel(N, X)                                               \
  X(N, Vertex, 0, {Capability::Shader}, {}, 0, 0)                              \
  X(N, TessellationControl, 1, {Capability::Tessellation}, {}, 0, 0)           \
  X(N, TessellationEvaluation, 2, {Capability::Tessellation}, {}, 0, 0)        \
  X(N, Geometry, 3, {Capability::Geometry}, {}, 0, 0)                          \
  X(N, Fragment, 4, {Capability::Shader}, {}, 0, 0)                            \
  X(N, GLCompute, 5, {Capability::Shader}, {}, 0, 0)                           \
  X(N, Kernel, 6, {Capability::Kernel}, {}, 0, 0)                              \
  X(N, TaskNV, 5267, {Capability::MeshShadingNV}, {}, 0, 0)                    \
  X(N, MeshNV, 5268, {Capability::MeshShadingNV}, {}, 0, 0)                    \
  X(N, RayGenerationNV, 5313, {Capability::RayTracingNV}, {}, 0, 0)            \
  X(N, IntersectionNV, 5314, {Capability::RayTracingNV}, {}, 0, 0)             \
  X(N, AnyHitNV, 5315, {Capability::RayTracingNV}, {}, 0, 0)                   \
  X(N, ClosestHitNV, 5316, {Capability::RayTracingNV}, {}, 0, 0)               \
  X(N, MissNV, 5317, {Capability::RayTracingNV}, {}, 0, 0)                     \
  X(N, CallableNV, 5318, {Capability::RayTracingNV}, {}, 0, 0)
GEN_ENUM_HEADER(ExecutionModel)

#define DEF_MemoryModel(N, X)                                                  \
  X(N, Simple, 0, {Capability::Shader}, {}, 0, 0)                              \
  X(N, GLSL450, 1, {Capability::Shader}, {}, 0, 0)                             \
  X(N, OpenCL, 2, {Capability::Kernel}, {}, 0, 0)                              \
  X(N, VulkanKHR, 3, {Capability::VulkanMemoryModelKHR}, {}, 0, 0)
GEN_ENUM_HEADER(MemoryModel)

#define DEF_ExecutionMode(N, X)                                                \
  X(N, Invocations, 0, {Capability::Geometry}, {}, 0, 0)                       \
  X(N, SpacingEqual, 1, {Capability::Tessellation}, {}, 0, 0)                  \
  X(N, SpacingFractionalEven, 2, {Capability::Tessellation}, {}, 0, 0)         \
  X(N, SpacingFractionalOdd, 3, {Capability::Tessellation}, {}, 0, 0)          \
  X(N, VertexOrderCw, 4, {Capability::Tessellation}, {}, 0, 0)                 \
  X(N, VertexOrderCcw, 5, {Capability::Tessellation}, {}, 0, 0)                \
  X(N, PixelCenterInteger, 6, {Capability::Shader}, {}, 0, 0)                  \
  X(N, OriginUpperLeft, 7, {Capability::Shader}, {}, 0, 0)                     \
  X(N, OriginLowerLeft, 8, {Capability::Shader}, {}, 0, 0)                     \
  X(N, EarlyFragmentTests, 9, {Capability::Shader}, {}, 0, 0)                  \
  X(N, PointMode, 10, {Capability::Tessellation}, {}, 0, 0)                    \
  X(N, Xfb, 11, {Capability::TransformFeedback}, {}, 0, 0)                     \
  X(N, DepthReplacing, 12, {Capability::Shader}, {}, 0, 0)                     \
  X(N, DepthGreater, 14, {Capability::Shader}, {}, 0, 0)                       \
  X(N, DepthLess, 15, {Capability::Shader}, {}, 0, 0)                          \
  X(N, DepthUnchanged, 16, {Capability::Shader}, {}, 0, 0)                     \
  X(N, LocalSize, 17, {}, {}, 0, 0)                                            \
  X(N, LocalSizeHint, 18, {Capability::Kernel}, {}, 0, 0)                      \
  X(N, InputPoints, 19, {Capability::Geometry}, {}, 0, 0)                      \
  X(N, InputLines, 20, {Capability::Geometry}, {}, 0, 0)                       \
  X(N, InputLinesAdjacency, 21, {Capability::Geometry}, {}, 0, 0)              \
  X(N, Triangles, 22, LIST({Capability::Geometry, Capability::Tessellation}),  \
    {}, 0, 0)                                                                  \
  X(N, InputTrianglesAdjacency, 23, {Capability::Geometry}, {}, 0, 0)          \
  X(N, Quads, 24, {Capability::Tessellation}, {}, 0, 0)                        \
  X(N, Isolines, 25, {Capability::Tessellation}, {}, 0, 0)                     \
  X(N, OutputVertices, 26,                                                     \
    LIST({Capability::Geometry, Capability::Tessellation,                      \
          Capability::MeshShadingNV}),                                         \
    {}, 0, 0)                                                                  \
  X(N, OutputPoints, 27,                                                       \
    LIST({Capability::Geometry, Capability::MeshShadingNV}), {}, 0, 0)         \
  X(N, OutputLineStrip, 28, {Capability::Geometry}, {}, 0, 0)                  \
  X(N, OutputTriangleStrip, 29, {Capability::Geometry}, {}, 0, 0)              \
  X(N, VecTypeHint, 30, {Capability::Kernel}, {}, 0, 0)                        \
  X(N, ContractionOff, 31, {Capability::Kernel}, {}, 0, 0)                     \
  X(N, Initializer, 33, {Capability::Kernel}, {}, 0, 0)                        \
  X(N, Finalizer, 34, {Capability::Kernel}, {}, 0, 0)                          \
  X(N, SubgroupSize, 35, {Capability::SubgroupDispatch}, {}, 0, 0)             \
  X(N, SubgroupsPerWorkgroup, 36, {Capability::SubgroupDispatch}, {}, 0, 0)    \
  X(N, SubgroupsPerWorkgroupId, 37, {Capability::SubgroupDispatch}, {}, 0, 0)  \
  X(N, LocalSizeId, 38, {}, {}, 0, 0)                                          \
  X(N, LocalSizeHintId, 39, {Capability::Kernel}, {}, 0, 0)                    \
  X(N, PostDepthCoverage, 4446, {Capability::SampleMaskPostDepthCoverage}, {}, \
    0, 0)                                                                      \
  X(N, DenormPreserve, 4459, {Capability::DenormPreserve}, {}, 0, 0)           \
  X(N, DenormFlushToZero, 4460, {Capability::DenormFlushToZero}, {}, 0, 0)     \
  X(N, SignedZeroInfNanPreserve, 4461, {Capability::SignedZeroInfNanPreserve}, \
    {}, 0, 0)                                                                  \
  X(N, RoundingModeRTE, 4462, {Capability::RoundingModeRTE}, {}, 0, 0)         \
  X(N, RoundingModeRTZ, 4463, {Capability::RoundingModeRTZ}, {}, 0, 0)         \
  X(N, StencilRefReplacingEXT, 5027, {Capability::StencilExportEXT}, {}, 0, 0) \
  X(N, OutputLinesNV, 5269, {Capability::MeshShadingNV}, {}, 0, 0)             \
  X(N, DerivativeGroupQuadsNV, 5289,                                           \
    {Capability::ComputeDerivativeGroupQuadsNV}, {}, 0, 0)                     \
  X(N, DerivativeGroupLinearNV, 5290,                                          \
    {Capability::ComputeDerivativeGroupLinearNV}, {}, 0, 0)                    \
  X(N, OutputTrianglesNV, 5298, {Capability::MeshShadingNV}, {}, 0, 0)
GEN_ENUM_HEADER(ExecutionMode)

#define DEF_StorageClass(N, X)                                                 \
  X(N, UniformConstant, 0, {}, {}, 0, 0)                                       \
  X(N, Input, 1, {}, {}, 0, 0)                                                 \
  X(N, Uniform, 2, {Capability::Shader}, {}, 0, 0)                             \
  X(N, Output, 3, {Capability::Shader}, {}, 0, 0)                              \
  X(N, Workgroup, 4, {}, {}, 0, 0)                                             \
  X(N, CrossWorkgroup, 5, {}, {}, 0, 0)                                        \
  X(N, Private, 6, {Capability::Shader}, {}, 0, 0)                             \
  X(N, Function, 7, {}, {}, 0, 0)                                              \
  X(N, Generic, 8, {Capability::GenericPointer}, {}, 0, 0)                     \
  X(N, PushConstant, 9, {Capability::Shader}, {}, 0, 0)                        \
  X(N, AtomicCounter, 10, {Capability::AtomicStorage}, {}, 0, 0)               \
  X(N, Image, 11, {}, {}, 0, 0)                                                \
  X(N, StorageBuffer, 12, {Capability::Shader}, {}, 0, 0)                      \
  X(N, CallableDataNV, 5328, {Capability::RayTracingNV}, {}, 0, 0)             \
  X(N, IncomingCallableDataNV, 5329, {Capability::RayTracingNV}, {}, 0, 0)     \
  X(N, RayPayloadNV, 5338, {Capability::RayTracingNV}, {}, 0, 0)               \
  X(N, HitAttributeNV, 5339, {Capability::RayTracingNV}, {}, 0, 0)             \
  X(N, IncomingRayPayloadNV, 5342, {Capability::RayTracingNV}, {}, 0, 0)       \
  X(N, ShaderRecordBufferNV, 5343, {Capability::RayTracingNV}, {}, 0, 0)       \
  X(N, PhysicalStorageBuffer, 5349,                                            \
    {Capability::PhysicalStorageBufferAddresses}, {}, 0, 0)
GEN_ENUM_HEADER(StorageClass)

// Need to manually do the getDimName() function, as "1D" is not a valid token
// so it can't be auto-generated with these macros

#define DEF_Dim(N, X)                                                          \
  X(N, DIM_1D, 0, LIST({Capability::Sampled1D, Capability::Image1D}), {}, 0,   \
    0)                                                                         \
  X(N, DIM_2D, 1,                                                              \
    LIST({Capability::Shader, Capability::Kernel, Capability::ImageMSArray}),  \
    {}, 0, 0)                                                                  \
  X(N, DIM_3D, 2, {}, {}, 0, 0)                                                \
  X(N, DIM_Cube, 3, LIST({Capability::Shader, Capability::ImageCubeArray}),    \
    {}, 0, 0)                                                                  \
  X(N, DIM_Rect, 4, LIST({Capability::SampledRect, Capability::ImageRect}),    \
    {}, 0, 0)                                                                  \
  X(N, DIM_Buffer, 5,                                                          \
    LIST({Capability::SampledBuffer, Capability::ImageBuffer}), {}, 0, 0)      \
  X(N, DIM_SubpassData, 6, {Capability::InputAttachment}, {}, 0, 0)
GEN_ENUM_HEADER(Dim)

#define DEF_SamplerAddressingMode(N, X)                                        \
  X(N, None, 0, {Capability::Kernel}, {}, 0, 0)                                \
  X(N, ClampToEdge, 1, {Capability::Kernel}, {}, 0, 0)                         \
  X(N, Clamp, 2, {Capability::Kernel}, {}, 0, 0)                               \
  X(N, Repeat, 3, {Capability::Kernel}, {}, 0, 0)                              \
  X(N, RepeatMirrored, 4, {Capability::Kernel}, {}, 0, 0)
GEN_ENUM_HEADER(SamplerAddressingMode)

#define DEF_SamplerFilterMode(N, X)                                            \
  X(N, Nearest, 0, {Capability::Kernel}, {}, 0, 0)                             \
  X(N, Linear, 1, {Capability::Kernel}, {}, 0, 0)
GEN_ENUM_HEADER(SamplerFilterMode)

#define DEF_ImageFormat(N, X)                                                  \
  X(N, Unknown, 0, {}, {}, 0, 0)                                               \
  X(N, Rgba32f, 1, {Capability::Shader}, {}, 0, 0)                             \
  X(N, Rgba16f, 2, {Capability::Shader}, {}, 0, 0)                             \
  X(N, R32f, 3, {Capability::Shader}, {}, 0, 0)                                \
  X(N, Rgba8, 4, {Capability::Shader}, {}, 0, 0)                               \
  X(N, Rgba8Snorm, 5, {Capability::Shader}, {}, 0, 0)                          \
  X(N, Rg32f, 6, {Capability::StorageImageExtendedFormats}, {}, 0, 0)          \
  X(N, Rg16f, 7, {Capability::StorageImageExtendedFormats}, {}, 0, 0)          \
  X(N, R11fG11fB10f, 8, {Capability::StorageImageExtendedFormats}, {}, 0, 0)   \
  X(N, R16f, 9, {Capability::StorageImageExtendedFormats}, {}, 0, 0)           \
  X(N, Rgba16, 10, {Capability::StorageImageExtendedFormats}, {}, 0, 0)        \
  X(N, Rgb10A2, 11, {Capability::StorageImageExtendedFormats}, {}, 0, 0)       \
  X(N, Rg16, 12, {Capability::StorageImageExtendedFormats}, {}, 0, 0)          \
  X(N, Rg8, 13, {Capability::StorageImageExtendedFormats}, {}, 0, 0)           \
  X(N, R16, 14, {Capability::StorageImageExtendedFormats}, {}, 0, 0)           \
  X(N, R8, 15, {Capability::StorageImageExtendedFormats}, {}, 0, 0)            \
  X(N, Rgba16Snorm, 16, {Capability::StorageImageExtendedFormats}, {}, 0, 0)   \
  X(N, Rg16Snorm, 17, {Capability::StorageImageExtendedFormats}, {}, 0, 0)     \
  X(N, Rg8Snorm, 18, {Capability::StorageImageExtendedFormats}, {}, 0, 0)      \
  X(N, R16Snorm, 19, {Capability::StorageImageExtendedFormats}, {}, 0, 0)      \
  X(N, R8Snorm, 20, {Capability::StorageImageExtendedFormats}, {}, 0, 0)       \
  X(N, Rgba32i, 21, {Capability::Shader}, {}, 0, 0)                            \
  X(N, Rgba16i, 22, {Capability::Shader}, {}, 0, 0)                            \
  X(N, Rgba8i, 23, {Capability::Shader}, {}, 0, 0)                             \
  X(N, R32i, 24, {Capability::Shader}, {}, 0, 0)                               \
  X(N, Rg32i, 25, {Capability::StorageImageExtendedFormats}, {}, 0, 0)         \
  X(N, Rg16i, 26, {Capability::StorageImageExtendedFormats}, {}, 0, 0)         \
  X(N, Rg8i, 27, {Capability::StorageImageExtendedFormats}, {}, 0, 0)          \
  X(N, R16i, 28, {Capability::StorageImageExtendedFormats}, {}, 0, 0)          \
  X(N, R8i, 29, {Capability::StorageImageExtendedFormats}, {}, 0, 0)           \
  X(N, Rgba32ui, 30, {Capability::Shader}, {}, 0, 0)                           \
  X(N, Rgba16ui, 31, {Capability::Shader}, {}, 0, 0)                           \
  X(N, Rgba8ui, 32, {Capability::Shader}, {}, 0, 0)                            \
  X(N, R32ui, 33, {Capability::Shader}, {}, 0, 0)                              \
  X(N, Rgb10a2ui, 34, {Capability::StorageImageExtendedFormats}, {}, 0, 0)     \
  X(N, Rg32ui, 35, {Capability::StorageImageExtendedFormats}, {}, 0, 0)        \
  X(N, Rg16ui, 36, {Capability::StorageImageExtendedFormats}, {}, 0, 0)        \
  X(N, Rg8ui, 37, {Capability::StorageImageExtendedFormats}, {}, 0, 0)         \
  X(N, R16ui, 38, {Capability::StorageImageExtendedFormats}, {}, 0, 0)         \
  X(N, R8ui, 39, {Capability::StorageImageExtendedFormats}, {}, 0, 0)
GEN_ENUM_HEADER(ImageFormat)

#define DEF_ImageChannelOrder(N, X)                                            \
  X(N, R, 0, {Capability::Kernel}, {}, 0, 0)                                   \
  X(N, A, 1, {Capability::Kernel}, {}, 0, 0)                                   \
  X(N, RG, 2, {Capability::Kernel}, {}, 0, 0)                                  \
  X(N, RA, 3, {Capability::Kernel}, {}, 0, 0)                                  \
  X(N, RGB, 4, {Capability::Kernel}, {}, 0, 0)                                 \
  X(N, RGBA, 5, {Capability::Kernel}, {}, 0, 0)                                \
  X(N, BGRA, 6, {Capability::Kernel}, {}, 0, 0)                                \
  X(N, ARGB, 7, {Capability::Kernel}, {}, 0, 0)                                \
  X(N, Intensity, 8, {Capability::Kernel}, {}, 0, 0)                           \
  X(N, Luminance, 9, {Capability::Kernel}, {}, 0, 0)                           \
  X(N, Rx, 10, {Capability::Kernel}, {}, 0, 0)                                 \
  X(N, RGx, 11, {Capability::Kernel}, {}, 0, 0)                                \
  X(N, RGBx, 12, {Capability::Kernel}, {}, 0, 0)                               \
  X(N, Depth, 13, {Capability::Kernel}, {}, 0, 0)                              \
  X(N, DepthStencil, 14, {Capability::Kernel}, {}, 0, 0)                       \
  X(N, sRGB, 15, {Capability::Kernel}, {}, 0, 0)                               \
  X(N, sRGBx, 16, {Capability::Kernel}, {}, 0, 0)                              \
  X(N, sRGBA, 17, {Capability::Kernel}, {}, 0, 0)                              \
  X(N, sBGRA, 18, {Capability::Kernel}, {}, 0, 0)                              \
  X(N, ABGR, 19, {Capability::Kernel}, {}, 0, 0)
GEN_ENUM_HEADER(ImageChannelOrder)

#define DEF_ImageChannelDataType(N, X)                                         \
  X(N, SnormInt8, 0, {}, {}, 0, 0)                                             \
  X(N, SnormInt16, 1, {}, {}, 0, 0)                                            \
  X(N, UnormInt8, 2, {Capability::Kernel}, {}, 0, 0)                           \
  X(N, UnormInt16, 3, {Capability::Kernel}, {}, 0, 0)                          \
  X(N, UnormShort565, 4, {Capability::Kernel}, {}, 0, 0)                       \
  X(N, UnormShort555, 5, {Capability::Kernel}, {}, 0, 0)                       \
  X(N, UnormInt101010, 6, {Capability::Kernel}, {}, 0, 0)                      \
  X(N, SignedInt8, 7, {Capability::Kernel}, {}, 0, 0)                          \
  X(N, SignedInt16, 8, {Capability::Kernel}, {}, 0, 0)                         \
  X(N, SignedInt32, 9, {Capability::Kernel}, {}, 0, 0)                         \
  X(N, UnsignedInt8, 10, {Capability::Kernel}, {}, 0, 0)                       \
  X(N, UnsignedInt16, 11, {Capability::Kernel}, {}, 0, 0)                      \
  X(N, UnsigendInt32, 12, {Capability::Kernel}, {}, 0, 0)                      \
  X(N, HalfFloat, 13, {Capability::Kernel}, {}, 0, 0)                          \
  X(N, Float, 14, {Capability::Kernel}, {}, 0, 0)                              \
  X(N, UnormInt24, 15, {Capability::Kernel}, {}, 0, 0)                         \
  X(N, UnormInt101010_2, 16, {Capability::Kernel}, {}, 0, 0)
GEN_ENUM_HEADER(ImageChannelDataType)

#define DEF_ImageOperand(N, X)                                                 \
  X(N, None, 0x0, {}, {}, 0, 0)                                                \
  X(N, Bias, 0x1, {Capability::Shader}, {}, 0, 0)                              \
  X(N, Lod, 0x2, {}, {}, 0, 0)                                                 \
  X(N, Grad, 0x4, {}, {}, 0, 0)                                                \
  X(N, ConstOffset, 0x8, {}, {}, 0, 0)                                         \
  X(N, Offset, 0x10, {Capability::ImageGatherExtended}, {}, 0, 0)              \
  X(N, ConstOffsets, 0x20, {Capability::ImageGatherExtended}, {}, 0, 0)        \
  X(N, Sample, 0x40, {}, {}, 0, 0)                                             \
  X(N, MinLod, 0x80, {Capability::MinLod}, {}, 0, 0)                           \
  X(N, MakeTexelAvailableKHR, 0x100, {Capability::VulkanMemoryModelKHR}, {},   \
    0, 0)                                                                      \
  X(N, MakeTexelVisibleKHR, 0x200, {Capability::VulkanMemoryModelKHR}, {}, 0,  \
    0)                                                                         \
  X(N, NonPrivateTexelKHR, 0x400, {Capability::VulkanMemoryModelKHR}, {}, 0,   \
    0)                                                                         \
  X(N, VolatileTexelKHR, 0x800, {Capability::VulkanMemoryModelKHR}, {}, 0, 0)  \
  X(N, SignExtend, 0x1000, {}, {}, 0, 0)                                       \
  X(N, ZeroExtend, 0x2000, {}, {}, 0, 0)
GEN_ENUM_HEADER(ImageOperand)

#define DEF_FPFastMathMode(N, X)                                               \
  X(N, None, 0x0, {}, {}, 0, 0)                                                \
  X(N, NotNaN, 0x1, {Capability::Kernel}, {}, 0, 0)                            \
  X(N, NotInf, 0x2, {Capability::Kernel}, {}, 0, 0)                            \
  X(N, NSZ, 0x4, {Capability::Kernel}, {}, 0, 0)                               \
  X(N, AllowRecip, 0x8, {Capability::Kernel}, {}, 0, 0)                        \
  X(N, Fast, 0x10, {Capability::Kernel}, {}, 0, 0)
GEN_ENUM_HEADER(FPFastMathMode)

#define DEF_FPRoundingMode(N, X)                                               \
  X(N, RTE, 0, {}, {}, 0, 0)                                                   \
  X(N, RTZ, 1, {}, {}, 0, 0)                                                   \
  X(N, RTP, 2, {}, {}, 0, 0)                                                   \
  X(N, RTN, 3, {}, {}, 0, 0)
GEN_ENUM_HEADER(FPRoundingMode)

#define DEF_LinkageType(N, X)                                                  \
  X(N, Export, 0, {Capability::Linkage}, {}, 0, 0)                             \
  X(N, Import, 1, {Capability::Linkage}, {}, 0, 0)
GEN_ENUM_HEADER(LinkageType)

#define DEF_AccessQualifier(N, X)                                              \
  X(N, ReadOnly, 0, {Capability::Kernel}, {}, 0, 0)                            \
  X(N, WriteOnly, 1, {Capability::Kernel}, {}, 0, 0)                           \
  X(N, ReadWrite, 2, {Capability::Kernel}, {}, 0, 0)
GEN_ENUM_HEADER(AccessQualifier)

#define DEF_FunctionParameterAttribute(N, X)                                   \
  X(N, Zext, 0, {Capability::Kernel}, {}, 0, 0)                                \
  X(N, Sext, 1, {Capability::Kernel}, {}, 0, 0)                                \
  X(N, ByVal, 2, {Capability::Kernel}, {}, 0, 0)                               \
  X(N, Sret, 3, {Capability::Kernel}, {}, 0, 0)                                \
  X(N, NoAlias, 4, {Capability::Kernel}, {}, 0, 0)                             \
  X(N, NoCapture, 5, {Capability::Kernel}, {}, 0, 0)                           \
  X(N, NoWrite, 6, {Capability::Kernel}, {}, 0, 0)                             \
  X(N, NoReadWrite, 7, {Capability::Kernel}, {}, 0, 0)
GEN_ENUM_HEADER(FunctionParameterAttribute)

#define DEF_Decoration(N, X)                                                   \
  X(N, RelaxedPrecision, 0, {Capability::Shader}, {}, 0, 0)                    \
  X(N, SpecId, 1, LIST({Capability::Shader, Capability::Kernel}), {}, 0, 0)    \
  X(N, Block, 2, {Capability::Shader}, {}, 0, 0)                               \
  X(N, BufferBlock, 3, {Capability::Shader}, {}, 0, 0)                         \
  X(N, RowMajor, 4, {Capability::Matrix}, {}, 0, 0)                            \
  X(N, ColMajor, 5, {Capability::Matrix}, {}, 0, 0)                            \
  X(N, ArrayStride, 6, {Capability::Shader}, {}, 0, 0)                         \
  X(N, MatrixStride, 7, {Capability::Matrix}, {}, 0, 0)                        \
  X(N, GLSLShared, 8, {Capability::Shader}, {}, 0, 0)                          \
  X(N, GLSLPacked, 9, {Capability::Shader}, {}, 0, 0)                          \
  X(N, CPacked, 10, {Capability::Kernel}, {}, 0, 0)                            \
  X(N, BuiltIn, 11, {}, {}, 0, 0)                                              \
  X(N, NoPerspective, 13, {Capability::Shader}, {}, 0, 0)                      \
  X(N, Flat, 14, {Capability::Shader}, {}, 0, 0)                               \
  X(N, Patch, 15, {Capability::Tessellation}, {}, 0, 0)                        \
  X(N, Centroid, 16, {Capability::Shader}, {}, 0, 0)                           \
  X(N, Sample, 17, {Capability::SampleRateShading}, {}, 0, 0)                  \
  X(N, Invariant, 18, {Capability::Shader}, {}, 0, 0)                          \
  X(N, Restrict, 19, {}, {}, 0, 0)                                             \
  X(N, Aliased, 20, {}, {}, 0, 0)                                              \
  X(N, Volatile, 21, {}, {}, 0, 0)                                             \
  X(N, Constant, 22, {Capability::Kernel}, {}, 0, 0)                           \
  X(N, Coherent, 23, {}, {}, 0, 0)                                             \
  X(N, NonWritable, 24, {}, {}, 0, 0)                                          \
  X(N, NonReadable, 25, {}, {}, 0, 0)                                          \
  X(N, Uniform, 26, {Capability::Shader}, {}, 0, 0)                            \
  X(N, UniformId, 27, {Capability::Shader}, {}, 0, 0)                          \
  X(N, SaturatedConversion, 28, {Capability::Kernel}, {}, 0, 0)                \
  X(N, Stream, 29, {Capability::GeometryStreams}, {}, 0, 0)                    \
  X(N, Location, 30, {Capability::Shader}, {}, 0, 0)                           \
  X(N, Component, 31, {Capability::Shader}, {}, 0, 0)                          \
  X(N, Index, 32, {Capability::Shader}, {}, 0, 0)                              \
  X(N, Binding, 33, {Capability::Shader}, {}, 0, 0)                            \
  X(N, DescriptorSet, 34, {Capability::Shader}, {}, 0, 0)                      \
  X(N, Offset, 35, {Capability::Shader}, {}, 0, 0)                             \
  X(N, XfbBuffer, 36, {Capability::TransformFeedback}, {}, 0, 0)               \
  X(N, XfbStride, 37, {Capability::TransformFeedback}, {}, 0, 0)               \
  X(N, FuncParamAttr, 38, {Capability::Kernel}, {}, 0, 0)                      \
  X(N, FPRoundingMode, 39, {}, {}, 0, 0)                                       \
  X(N, FPFastMathMode, 40, {Capability::Kernel}, {}, 0, 0)                     \
  X(N, LinkageAttributes, 41, {Capability::Linkage}, {}, 0, 0)                 \
  X(N, NoContraction, 42, {Capability::Shader}, {}, 0, 0)                      \
  X(N, InputAttachmentIndex, 43, {Capability::InputAttachment}, {}, 0, 0)      \
  X(N, Alignment, 44, {Capability::Kernel}, {}, 0, 0)                          \
  X(N, MaxByteOffset, 45, {Capability::Addresses}, {}, 0, 0)                   \
  X(N, AlignmentId, 46, {Capability::Kernel}, {}, 0, 0)                        \
  X(N, MaxByteOffsetId, 47, {Capability::Addresses}, {}, 0, 0)                 \
  X(N, NoSignedWrap, 4469, {}, {SK(no_integer_wrap_decoration)}, 0x10400, 0)   \
  X(N, NoUnsignedWrap, 4470, {}, {SK(no_integer_wrap_decoration)}, 0x10400, 0) \
  X(N, ExplicitInterpAMD, 4999, {}, {}, 0, 0)                                  \
  X(N, OverrideCoverageNV, 5248, {Capability::SampleMaskOverrideCoverageNV},   \
    {}, 0, 0)                                                                  \
  X(N, PassthroughNV, 5250, {Capability::GeometryShaderPassthroughNV}, {}, 0,  \
    0)                                                                         \
  X(N, ViewportRelativeNV, 5252, {Capability::ShaderViewportMaskNV}, {}, 0, 0) \
  X(N, SecondaryViewportRelativeNV, 5256, {Capability::ShaderStereoViewNV},    \
    {}, 0, 0)                                                                  \
  X(N, PerPrimitiveNV, 5271, {Capability::MeshShadingNV}, {}, 0, 0)            \
  X(N, PerViewNV, 5272, {Capability::MeshShadingNV}, {}, 0, 0)                 \
  X(N, PerVertexNV, 5273, {Capability::FragmentBarycentricNV}, {}, 0, 0)       \
  X(N, NonUniformEXT, 5300, {Capability::ShaderNonUniformEXT}, {}, 0, 0)       \
  X(N, CountBuffer, 5634, {}, {}, 0, 0)                                        \
  X(N, UserSemantic, 5635, {}, {}, 0, 0)                                       \
  X(N, RestrictPointer, 5355, {Capability::PhysicalStorageBufferAddresses},    \
    {}, 0, 0)                                                                  \
  X(N, AliasedPointer, 5356, {Capability::PhysicalStorageBufferAddresses}, {}, \
    0, 0)
GEN_ENUM_HEADER(Decoration)

#define DEF_BuiltIn(N, X)                                                      \
  X(N, Position, 0, {Capability::Shader}, {}, 0, 0)                            \
  X(N, PointSize, 1, {Capability::Shader}, {}, 0, 0)                           \
  X(N, ClipDistance, 3, {Capability::ClipDistance}, {}, 0, 0)                  \
  X(N, CullDistance, 4, {Capability::CullDistance}, {}, 0, 0)                  \
  X(N, VertexId, 5, {Capability::Shader}, {}, 0, 0)                            \
  X(N, InstanceId, 6, {Capability::Shader}, {}, 0, 0)                          \
  X(N, PrimitiveId, 7,                                                         \
    LIST({Capability::Geometry, Capability::Tessellation,                      \
          Capability::RayTracingNV}),                                          \
    {}, 0, 0)                                                                  \
  X(N, InvocationId, 8,                                                        \
    LIST({Capability::Geometry, Capability::Tessellation}), {}, 0, 0)          \
  X(N, Layer, 9, {Capability::Geometry}, {}, 0, 0)                             \
  X(N, ViewportIndex, 10, {Capability::MultiViewport}, {}, 0, 0)               \
  X(N, TessLevelOuter, 11, {Capability::Tessellation}, {}, 0, 0)               \
  X(N, TessLevelInner, 12, {Capability::Tessellation}, {}, 0, 0)               \
  X(N, TessCoord, 13, {Capability::Tessellation}, {}, 0, 0)                    \
  X(N, PatchVertices, 14, {Capability::Tessellation}, {}, 0, 0)                \
  X(N, FragCoord, 15, {Capability::Shader}, {}, 0, 0)                          \
  X(N, PointCoord, 16, {Capability::Shader}, {}, 0, 0)                         \
  X(N, FrontFacing, 17, {Capability::Shader}, {}, 0, 0)                        \
  X(N, SampleId, 18, {Capability::SampleRateShading}, {}, 0, 0)                \
  X(N, SamplePosition, 19, {Capability::SampleRateShading}, {}, 0, 0)          \
  X(N, SampleMask, 20, {Capability::Shader}, {}, 0, 0)                         \
  X(N, FragDepth, 22, {Capability::Shader}, {}, 0, 0)                          \
  X(N, HelperInvocation, 23, {Capability::Shader}, {}, 0, 0)                   \
  X(N, NumWorkGroups, 24, {}, {}, 0, 0)                                        \
  X(N, WorkgroupSize, 25, {}, {}, 0, 0)                                        \
  X(N, WorkgroupId, 26, {}, {}, 0, 0)                                          \
  X(N, LocalInvocationId, 27, {}, {}, 0, 0)                                    \
  X(N, GlobalInvocationId, 28, {}, {}, 0, 0)                                   \
  X(N, LocalInvocationIndex, 29, {}, {}, 0, 0)                                 \
  X(N, WorkDim, 30, {Capability::Kernel}, {}, 0, 0)                            \
  X(N, GlobalSize, 31, {Capability::Kernel}, {}, 0, 0)                         \
  X(N, EnqueuedWorkgroupSize, 32, {Capability::Kernel}, {}, 0, 0)              \
  X(N, GlobalOffset, 33, {Capability::Kernel}, {}, 0, 0)                       \
  X(N, GlobalLinearId, 34, {Capability::Kernel}, {}, 0, 0)                     \
  X(N, SubgroupSize, 36,                                                       \
    LIST({Capability::Kernel, Capability::GroupNonUniform,                     \
          Capability::SubgroupBallotKHR}),                                     \
    {}, 0, 0)                                                                  \
  X(N, SubgroupMaxSize, 37, {Capability::Kernel}, {}, 0, 0)                    \
  X(N, NumSubgroups, 38,                                                       \
    LIST({Capability::Kernel, Capability::GroupNonUniform}), {}, 0, 0)         \
  X(N, NumEnqueuedSubgroups, 39, {Capability::Kernel}, {}, 0, 0)               \
  X(N, SubgroupId, 40,                                                         \
    LIST({Capability::Kernel, Capability::GroupNonUniform}), {}, 0, 0)         \
  X(N, SubgroupLocalInvocationId, 41,                                          \
    LIST({Capability::Kernel, Capability::GroupNonUniform,                     \
          Capability::SubgroupBallotKHR}),                                     \
    {}, 0, 0)                                                                  \
  X(N, VertexIndex, 42, {Capability::Shader}, {}, 0, 0)                        \
  X(N, InstanceIndex, 43, {Capability::Shader}, {}, 0, 0)                      \
  X(N, SubgroupEqMask, 4416,                                                   \
    LIST({Capability::SubgroupBallotKHR, Capability::GroupNonUniformBallot}),  \
    {}, 0, 0)                                                                  \
  X(N, SubgroupGeMask, 4417,                                                   \
    LIST({Capability::SubgroupBallotKHR, Capability::GroupNonUniformBallot}),  \
    {}, 0, 0)                                                                  \
  X(N, SubgroupGtMask, 4418,                                                   \
    LIST({Capability::SubgroupBallotKHR, Capability::GroupNonUniformBallot}),  \
    {}, 0, 0)                                                                  \
  X(N, SubgroupLeMask, 4419,                                                   \
    LIST({Capability::SubgroupBallotKHR, Capability::GroupNonUniformBallot}),  \
    {}, 0, 0)                                                                  \
  X(N, SubgroupLtMask, 4420,                                                   \
    LIST({Capability::SubgroupBallotKHR, Capability::GroupNonUniformBallot}),  \
    {}, 0, 0)                                                                  \
  X(N, BaseVertex, 4424, {Capability::DrawParameters}, {}, 0, 0)               \
  X(N, BaseInstance, 4425, {Capability::DrawParameters}, {}, 0, 0)             \
  X(N, DrawIndex, 4426,                                                        \
    LIST({Capability::DrawParameters, Capability::MeshShadingNV}), {}, 0, 0)   \
  X(N, DeviceIndex, 4438, {Capability::DeviceGroup}, {}, 0, 0)                 \
  X(N, ViewIndex, 4440, {Capability::MultiView}, {}, 0, 0)                     \
  X(N, BaryCoordNoPerspAMD, 4492, {}, {}, 0, 0)                                \
  X(N, BaryCoordNoPerspCentroidAMD, 4493, {}, {}, 0, 0)                        \
  X(N, BaryCoordNoPerspSampleAMD, 4494, {}, {}, 0, 0)                          \
  X(N, BaryCoordSmoothAMD, 4495, {}, {}, 0, 0)                                 \
  X(N, BaryCoordSmoothCentroid, 4496, {}, {}, 0, 0)                            \
  X(N, BaryCoordSmoothSample, 4497, {}, {}, 0, 0)                              \
  X(N, BaryCoordPullModel, 4498, {}, {}, 0, 0)                                 \
  X(N, FragStencilRefEXT, 5014, {Capability::StencilExportEXT}, {}, 0, 0)      \
  X(N, ViewportMaskNV, 5253,                                                   \
    LIST({Capability::ShaderViewportMaskNV, Capability::MeshShadingNV}), {},   \
    0, 0)                                                                      \
  X(N, SecondaryPositionNV, 5257, {Capability::ShaderStereoViewNV}, {}, 0, 0)  \
  X(N, SecondaryViewportMaskNV, 5258, {Capability::ShaderStereoViewNV}, {}, 0, \
    0)                                                                         \
  X(N, PositionPerViewNV, 5261,                                                \
    LIST({Capability::PerViewAttributesNV, Capability::MeshShadingNV}), {}, 0, \
    0)                                                                         \
  X(N, ViewportMaskPerViewNV, 5262,                                            \
    LIST({Capability::PerViewAttributesNV, Capability::MeshShadingNV}), {}, 0, \
    0)                                                                         \
  X(N, FullyCoveredEXT, 5264, {Capability::FragmentFullyCoveredEXT}, {}, 0, 0) \
  X(N, TaskCountNV, 5274, {Capability::MeshShadingNV}, {}, 0, 0)               \
  X(N, PrimitiveCountNV, 5275, {Capability::MeshShadingNV}, {}, 0, 0)          \
  X(N, PrimitiveIndicesNV, 5276, {Capability::MeshShadingNV}, {}, 0, 0)        \
  X(N, ClipDistancePerViewNV, 5277, {Capability::MeshShadingNV}, {}, 0, 0)     \
  X(N, CullDistancePerViewNV, 5278, {Capability::MeshShadingNV}, {}, 0, 0)     \
  X(N, LayerPerViewNV, 5279, {Capability::MeshShadingNV}, {}, 0, 0)            \
  X(N, MeshViewCountNV, 5280, {Capability::MeshShadingNV}, {}, 0, 0)           \
  X(N, MeshViewIndices, 5281, {Capability::MeshShadingNV}, {}, 0, 0)           \
  X(N, BaryCoordNV, 5286, {Capability::FragmentBarycentricNV}, {}, 0, 0)       \
  X(N, BaryCoordNoPerspNV, 5287, {Capability::FragmentBarycentricNV}, {}, 0,   \
    0)                                                                         \
  X(N, FragSizeEXT, 5292, {Capability::FragmentDensityEXT}, {}, 0, 0)          \
  X(N, FragInvocationCountEXT, 5293, {Capability::FragmentDensityEXT}, {}, 0,  \
    0)                                                                         \
  X(N, LaunchIdNV, 5319, {Capability::RayTracingNV}, {}, 0, 0)                 \
  X(N, LaunchSizeNV, 5320, {Capability::RayTracingNV}, {}, 0, 0)               \
  X(N, WorldRayOriginNV, 5321, {Capability::RayTracingNV}, {}, 0, 0)           \
  X(N, WorldRayDirectionNV, 5322, {Capability::RayTracingNV}, {}, 0, 0)        \
  X(N, ObjectRayOriginNV, 5323, {Capability::RayTracingNV}, {}, 0, 0)          \
  X(N, ObjectRayDirectionNV, 5324, {Capability::RayTracingNV}, {}, 0, 0)       \
  X(N, RayTminNV, 5325, {Capability::RayTracingNV}, {}, 0, 0)                  \
  X(N, RayTmaxNV, 5326, {Capability::RayTracingNV}, {}, 0, 0)                  \
  X(N, InstanceCustomIndexNV, 5327, {Capability::RayTracingNV}, {}, 0, 0)      \
  X(N, ObjectToWorldNV, 5330, {Capability::RayTracingNV}, {}, 0, 0)            \
  X(N, WorldToObjectNV, 5331, {Capability::RayTracingNV}, {}, 0, 0)            \
  X(N, HitTNV, 5332, {Capability::RayTracingNV}, {}, 0, 0)                     \
  X(N, HitKindNV, 5333, {Capability::RayTracingNV}, {}, 0, 0)                  \
  X(N, IncomingRayFlagsNV, 5351, {Capability::RayTracingNV}, {}, 0, 0)
GEN_ENUM_HEADER(BuiltIn)

#define DEF_SelectionControl(N, X)                                             \
  X(N, None, 0x0, {}, {}, 0, 0)                                                \
  X(N, Flatten, 0x1, {}, {}, 0, 0)                                             \
  X(N, DontFlatten, 0x2, {}, {}, 0, 0)
GEN_ENUM_HEADER(SelectionControl)

#define DEF_LoopControl(N, X)                                                  \
  X(N, None, 0x0, {}, {}, 0, 0)                                                \
  X(N, Unroll, 0x1, {}, {}, 0, 0)                                              \
  X(N, DontUnroll, 0x2, {}, {}, 0, 0)                                          \
  X(N, DependencyInfinite, 0x4, {}, {}, 0, 0)                                  \
  X(N, DependencyLength, 0x8, {}, {}, 0, 0)                                    \
  X(N, MinIterations, 0x10, {}, {}, 0, 0)                                      \
  X(N, MaxIterations, 0x20, {}, {}, 0, 0)                                      \
  X(N, IterationMultiple, 0x40, {}, {}, 0, 0)                                  \
  X(N, PeelCount, 0x80, {}, {}, 0, 0)                                          \
  X(N, PartialCount, 0x100, {}, {}, 0, 0)
GEN_ENUM_HEADER(LoopControl)

#define DEF_FunctionControl(N, X)                                              \
  X(N, None, 0x0, {}, {}, 0, 0)                                                \
  X(N, Inline, 0x1, {}, {}, 0, 0)                                              \
  X(N, DontInline, 0x2, {}, {}, 0, 0)                                          \
  X(N, Pure, 0x4, {}, {}, 0, 0)                                                \
  X(N, Const, 0x8, {}, {}, 0, 0)
GEN_ENUM_HEADER(FunctionControl)

#define DEF_MemorySemantics(N, X)                                              \
  X(N, None, 0x0, {}, {}, 0, 0)                                                \
  X(N, Acquire, 0x2, {}, {}, 0, 0)                                             \
  X(N, Release, 0x4, {}, {}, 0, 0)                                             \
  X(N, AcquireRelease, 0x8, {}, {}, 0, 0)                                      \
  X(N, SequentiallyConsistent, 0x10, {}, {}, 0, 0)                             \
  X(N, UniformMemory, 0x40, {Capability::Shader}, {}, 0, 0)                    \
  X(N, SubgroupMemory, 0x80, {}, {}, 0, 0)                                     \
  X(N, WorkgroupMemory, 0x100, {}, {}, 0, 0)                                   \
  X(N, CrossWorkgroupMemory, 0x200, {}, {}, 0, 0)                              \
  X(N, AtomicCounterMemory, 0x400, {Capability::AtomicStorage}, {}, 0, 0)      \
  X(N, ImageMemory, 0x800, {}, {}, 0, 0)                                       \
  X(N, OutputMemoryKHR, 0x1000, {Capability::VulkanMemoryModelKHR}, {}, 0, 0)  \
  X(N, MakeAvailableKHR, 0x2000, {Capability::VulkanMemoryModelKHR}, {}, 0, 0) \
  X(N, MakeVisibleKHR, 0x4000, {Capability::VulkanMemoryModelKHR}, {}, 0, 0)
GEN_ENUM_HEADER(MemorySemantics)

#define DEF_MemoryOperand(N, X)                                                \
  X(N, None, 0x0, {}, {}, 0, 0)                                                \
  X(N, Volatile, 0x1, {}, {}, 0, 0)                                            \
  X(N, Aligned, 0x2, {}, {}, 0, 0)                                             \
  X(N, Nontemporal, 0x4, {}, {}, 0, 0)                                         \
  X(N, MakePointerAvailableKHR, 0x8, {Capability::VulkanMemoryModelKHR}, {},   \
    0, 0)                                                                      \
  X(N, MakePointerVisibleKHR, 0x10, {Capability::VulkanMemoryModelKHR}, {}, 0, \
    0)                                                                         \
  X(N, NonPrivatePointerKHR, 0x20, {Capability::VulkanMemoryModelKHR}, {}, 0, 0)
GEN_ENUM_HEADER(MemoryOperand)

#define DEF_Scope(N, X)                                                        \
  X(N, CrossDevice, 0, {}, {}, 0, 0)                                           \
  X(N, Device, 1, {}, {}, 0, 0)                                                \
  X(N, Workgroup, 2, {}, {}, 0, 0)                                             \
  X(N, Subgroup, 3, {}, {}, 0, 0)                                              \
  X(N, Invocation, 4, {}, {}, 0, 0)                                            \
  X(N, QueueFamilyKHR, 5, {Capability::VulkanMemoryModelKHR}, {}, 0, 0)
GEN_ENUM_HEADER(Scope)

#define GNUA Capability::GroupNonUniformArithmetic
#define GNUB Capability::GroupNonUniformBallot
#define GNUPNV Capability::GroupNonUniformPartitionedNV
#define DEF_GroupOperation(N, X)                                               \
  X(N, Reduce, 0, LIST({Capability::Kernel, GNUA, GNUB}), {}, 0, 0)            \
  X(N, InclusiveScan, 1, LIST({Capability::Kernel, GNUA, GNUB}), {}, 0, 0)     \
  X(N, ExclusiveScan, 2, LIST({Capability::Kernel, GNUA, GNUB}), {}, 0, 0)     \
  X(N, ClusteredReduce, 3, {Capability::GroupNonUniformClustered}, {}, 0, 0)   \
  X(N, PartitionedReduceNV, 6, {Capability::GroupNonUniformPartitionedNV}, {}, \
    0, 0)                                                                      \
  X(N, PartitionedInclusiveScanNV, 7, {GNUPNV}, {}, 0, 0)                      \
  X(N, PartitionedExclusiveScanNV, 8, {GNUPNV}, {}, 0, 0)
GEN_ENUM_HEADER(GroupOperation)

#define DEF_KernelEnqueueFlags(N, X)                                           \
  X(N, NoWait, 0, {Capability::Kernel}, {}, 0, 0)                              \
  X(N, WaitKernel, 1, {Capability::Kernel}, {}, 0, 0)                          \
  X(N, WaitWorkGroup, 2, {Capability::Kernel}, {}, 0, 0)
GEN_ENUM_HEADER(KernelEnqueueFlags)

#define DEF_KernelProfilingInfo(N, X)                                          \
  X(N, None, 0x0, {}, {}, 0, 0)                                                \
  X(N, CmdExecTime, 0x1, {Capability::Kernel}, {}, 0, 0)
GEN_ENUM_HEADER(KernelProfilingInfo)

MemorySemantics getMemSemanticsForStorageClass(StorageClass sc);

const char *getLinkStrForBuiltIn(BuiltIn builtIn);

namespace kMD {
const static char EntryPoint[] = "spirv.EntryPoint";
const static char ExecutionMode[] = "spirv.ExecutionMode";
const static char GlobalStorageClass[] = "spirv.StorageClass";
const static char GlobalDecoration[] = "spirv.Decoration";
const static char GlobalTypeSpec[] = "spirv.TypeSpec";
const static char EntryExeModel[] = "spirv.ExecutionModel";
} // namespace kMD

#endif
