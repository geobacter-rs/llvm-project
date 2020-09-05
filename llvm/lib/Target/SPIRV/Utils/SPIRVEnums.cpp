//===-- SPIRVEnums.cpp - SPIR-V Enums and Related Functions -----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains macros implementing the enum helper functions defined in
// SPIRVEnums.h, such as getEnumName(Enum e) and getEnumCapabilities(Enum e)
//
//===----------------------------------------------------------------------===//

#include "SPIRVEnums.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Metadata.h"

GEN_ENUM_IMPL(Capability)
GEN_ENUM_IMPL(SourceLanguage)
GEN_ENUM_IMPL(ExecutionModel)
GEN_ENUM_IMPL(AddressingModel)
GEN_ENUM_IMPL(MemoryModel)
GEN_ENUM_IMPL(ExecutionMode)
GEN_ENUM_IMPL(StorageClass)

// Dim must be implemented manually, as "1D" is not a valid C++ naming token
std::string getDimName(Dim dim) {
  switch (dim) {
  case Dim::DIM_1D:
    return "1D";
  case Dim::DIM_2D:
    return "2D";
  case Dim::DIM_3D:
    return "3D";
  case Dim::DIM_Cube:
    return "Cube";
  case Dim::DIM_Rect:
    return "Rect";
  case Dim::DIM_Buffer:
    return "Buffer";
  case Dim::DIM_SubpassData:
    return "SubpassData";
  }
}
llvm::Optional<Dim> getDimFromStr(llvm::StringRef Name) {
  return llvm::StringSwitch<llvm::Optional<Dim>>(Name)
      .Case("1D", {Dim::DIM_1D})
      .Case("2D", {Dim::DIM_2D})
      .Case("3D", {Dim::DIM_3D})
      .Case("Cube", {Dim::DIM_Cube})
      .Case("Rect", {Dim::DIM_Rect})
      .Case("Buffer", {Dim::DIM_Buffer})
      .Case("SubpassData", {Dim::DIM_SubpassData})
      .Default(llvm::None);
}
llvm::Optional<Dim> decodeDimMD(const llvm::Metadata* MD) {
  if (!MD) {
    return llvm::None;
  }
  if (auto *MDStr = llvm::dyn_cast<llvm::MDString>(MD)) {
    return getDimFromStr(MDStr->getString());
  }
  if (auto *C = llvm::mdconst::dyn_extract<llvm::ConstantInt>(MD)) {
    const auto Value = static_cast<Dim>(C->getLimitedValue(~0ULL));
    if (!getDimName(Value).empty()) {
      return {Value};
    }
  }
  return llvm::None;
}

GEN_ENUM_IMPL(SamplerAddressingMode)
GEN_ENUM_IMPL(SamplerFilterMode)

GEN_ENUM_IMPL(ImageFormat)
GEN_ENUM_IMPL(ImageChannelOrder)
GEN_ENUM_IMPL(ImageChannelDataType)
GEN_MASK_ENUM_IMPL(ImageOperand)

GEN_MASK_ENUM_IMPL(FPFastMathMode)
GEN_ENUM_IMPL(FPRoundingMode)

GEN_ENUM_IMPL(LinkageType)
GEN_ENUM_IMPL(AccessQualifier)
GEN_ENUM_IMPL(FunctionParameterAttribute)

GEN_ENUM_IMPL(Decoration)
GEN_ENUM_IMPL(BuiltIn)

GEN_MASK_ENUM_IMPL(SelectionControl)
GEN_MASK_ENUM_IMPL(LoopControl)
GEN_MASK_ENUM_IMPL(FunctionControl)

GEN_MASK_ENUM_IMPL(MemorySemantics)
GEN_MASK_ENUM_IMPL(MemoryOperand)

GEN_ENUM_IMPL(Scope)
GEN_ENUM_IMPL(GroupOperation)

GEN_ENUM_IMPL(KernelEnqueueFlags)
GEN_MASK_ENUM_IMPL(KernelProfilingInfo)

MemorySemantics getMemSemanticsForStorageClass(StorageClass sc) {
  switch (sc) {
  case StorageClass::StorageBuffer:
  case StorageClass::Uniform:
    return MemorySemantics::UniformMemory;
  case StorageClass::Workgroup:
    return MemorySemantics::WorkgroupMemory;
  case StorageClass::CrossWorkgroup:
    return MemorySemantics::CrossWorkgroupMemory;
  case StorageClass::AtomicCounter:
    return MemorySemantics::AtomicCounterMemory;
  case StorageClass::Image:
    return MemorySemantics::ImageMemory;
  default:
    return MemorySemantics::None;
  }
}

DEF_BUILTIN_LINK_STR_FUNC_BODY()

GEN_EXTENSION_IMPL(Extension)
