//===-- SPIRVSubtarget.h - SPIR-V Subtarget Information --------*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares the SPIR-V specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SPIRV_SPIRVSUBTARGET_H
#define LLVM_LIB_TARGET_SPIRV_SPIRVSUBTARGET_H

#include "SPIRVFrameLowering.h"
#include "SPIRVISelLowering.h"
#include "SPIRVInstrInfo.h"
#include "llvm/CodeGen/GlobalISel/CallLowering.h"
#include "llvm/CodeGen/GlobalISel/InstructionSelector.h"
#include "llvm/CodeGen/GlobalISel/LegalizerInfo.h"
#include "llvm/CodeGen/GlobalISel/RegisterBankInfo.h"
#include "llvm/CodeGen/SelectionDAGTargetInfo.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/Target/TargetMachine.h"

#include "SPIRVCallLowering.h"
#include "SPIRVEnums.h"
#include "SPIRVExtInsts.h"
#include "SPIRVExtensions.h"
#include "SPIRVRegisterBankInfo.h"

#include <unordered_set>

#define GET_SUBTARGETINFO_HEADER
#include "SPIRVGenSubtargetInfo.inc"

namespace llvm {
class StringRef;

class SPIRVTargetMachine;
class SPIRVTypeRegistry;

class SPIRVSubtarget : public SPIRVGenSubtargetInfo {
private:
  SPIRVInstrInfo InstrInfo;
  SPIRVFrameLowering FrameLowering;
  SPIRVTargetLowering TLInfo;

  const unsigned int pointerSize;

  const bool usesLogicalAddressing;
  const bool usesVulkanEnv;
  const bool usesOpenCLEnv;

  const uint32_t targetSPIRVVersion;
  const uint32_t targetOpenCLVersion;
  const uint32_t targetVulkanVersion;

  const bool openCLFullProfile;
  const bool openCLImageSupport;

  void updateCapabilitiesFromFeatures();
  void enableFeatureCapability(const Capability Cap);
  void enableFeatureCapabilities(const ArrayRef<Capability> Caps);

  // TODO Some of these fields might work without unique_ptr.
  //      But they are shared with other classes, so if the SPIRVSubtarget
  //      moves, not relying on unique_ptr breaks things.
  std::unique_ptr<SPIRVTypeRegistry> TR;
  std::unique_ptr<SPIRVCallLowering> CallLoweringInfo;
  std::unique_ptr<SPIRVRegisterBankInfo> RegBankInfo;

  std::unordered_set<Extension> availableExtensions;
  std::unordered_set<ExtInstSet> availableExtInstSets;
  std::unordered_set<Capability> availableCaps;

  // The legalizer and instruction selector both rely on the set of available
  // extensions, capabilities, register bank information, and so on.
  std::unique_ptr<LegalizerInfo> Legalizer;
  std::unique_ptr<InstructionSelector> InstSelector;

private:
  // Initialise the available extensions, extended instruction sets and
  // capabilities based on the environment settings (i.e. the previous
  // properties of SPIRVSubtarget).
  //
  // These functions must be called in the order they are declared to satisfy
  // dependencies during initialisation.
  void initAvailableExtensions(const Triple &TT);
  void initAvailableExtInstSets(const Triple &TT);
  void initAvailableCapabilities(const Triple &TT);

protected:
  // DummyFeature defined in SPIRV.td. This is for illustration purpose only
  // and isn't used in practice.
  bool isDummyMode;

public:
#define MAKE_CAP_FEATURE_FIELDS(Enum, Var, Val, Caps, Exts, MinVer, MaxVer)    \
  bool Has##Var : 1;
#define DEF_CAP_FEATURES(EnumName, DefCommand)                                 \
  DefCommand(EnumName, MAKE_CAP_FEATURE_FIELDS)

  // TODO: Finish writing the tablegen feature defs.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-private-field"
  DEF_CAP_FEATURES(Capability, DEF_Capability)
#pragma GCC diagnostic pop

#undef DEF_CAP_FEATURES
#undef MAKE_CAP_FEATURE_FIELDS

  // This constructor initializes the data members to match that
  // of the specified triple.
  SPIRVSubtarget(const Triple &TT, const std::string &CPU,
                 const std::string &FS, const SPIRVTargetMachine &TM);

  SPIRVSubtarget &initSubtargetDependencies(StringRef CPU, StringRef FS);

  // Parses features string setting specified subtarget options.
  //
  // The definition of this function is auto generated by tblgen.
  void ParseSubtargetFeatures(StringRef CPU, StringRef FS);

  unsigned int getPointerSize() const { return pointerSize; }
  bool canDirectlyComparePointers() const;

  bool isLogicalAddressing() const;
  bool isKernel() const;
  bool isShader() const;

  uint32_t getTargetSPIRVVersion() const { return targetSPIRVVersion; };

  bool canUseCapability(Capability c) const;
  bool canUseExtension(Extension e) const;
  bool canUseExtInstSet(ExtInstSet e) const;

  const std::unordered_set<Extension> &getAvailableExtensions() const {
    return availableExtensions;
  }

  SPIRVTypeRegistry *getSPIRVTypeRegistry() const { return TR.get(); }

  const CallLowering *getCallLowering() const override {
    return CallLoweringInfo.get();
  }

  InstructionSelector *getInstructionSelector() const override {
    return InstSelector.get();
  }

  const LegalizerInfo *getLegalizerInfo() const override {
    return Legalizer.get();
  }

  const RegisterBankInfo *getRegBankInfo() const override {
    return RegBankInfo.get();
  }

  bool isLittleEndian() const { return true; }

  const SPIRVInstrInfo *getInstrInfo() const override { return &InstrInfo; }

  const SPIRVFrameLowering *getFrameLowering() const override {
    return &FrameLowering;
  }

  const SPIRVTargetLowering *getTargetLowering() const override {
    return &TLInfo;
  }

  const SPIRVRegisterInfo *getRegisterInfo() const override {
    return &InstrInfo.getRegisterInfo();
  }
};
} // namespace llvm

#endif
