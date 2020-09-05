//===--- SPIRVEntryPoints.cpp - Insert OpEntryPoint instructions -*- C++
//-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//
//
//===----------------------------------------------------------------------===//

#include "SPIRV.h"
#include "SPIRVMetadata.h"
#include "SPIRVStrings.h"
#include "SPIRVSubtarget.h"

#include "llvm/CodeGen/GlobalISel/MachineIRBuilder.h"
#include "llvm/CodeGen/MachineFunctionPass.h"

using namespace llvm;
using namespace SPIRV;

#define DEBUG_TYPE "spirv-entry-points"

namespace {
class SPIRVEntryPoints : public MachineFunctionPass {
  bool Changed = false;
  const SPIRVSubtarget *ST = nullptr;
  const Function *F = nullptr;
  MachineFunction *MF = nullptr;
  Optional<Register> FVReg;
  SmallVector<Register, 16> Interface;

  Optional<MachineIRBuilder> MIRB;

public:
  static char ID;
  SPIRVEntryPoints() : MachineFunctionPass(ID) {
    initializeSPIRVEntryPointsPass(*PassRegistry::getPassRegistry());
  }
  bool runOnMachineFunction(MachineFunction &MF) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
    MachineFunctionPass::getAnalysisUsage(AU);
  }
  StringRef getPassName() const override {
    return "SPIRV EntryPoint Annotator";
  }

  inline bool needsEntryPoint() const {
    return MF->getFunction().getCallingConv() == CallingConv::SPIR_KERNEL;
  }

  void findFunctionVReg();

  void buildEntryPointInterface();
  void buildEntryPoint();
};
} // namespace
char SPIRVEntryPoints::ID = 0;
char &llvm::SPIRVEntryPointsID = SPIRVEntryPoints::ID;

INITIALIZE_PASS_BEGIN(SPIRVEntryPoints, DEBUG_TYPE,
                      "SPIRV EntryPoint Annotator", false, false)
INITIALIZE_PASS_END(SPIRVEntryPoints, DEBUG_TYPE, "SPIRV EntryPoint Annotator",
                    false, false)

bool SPIRVEntryPoints::runOnMachineFunction(MachineFunction &MF_) {
  Changed = false;
  MF = &MF_;
  ST = &MF->getSubtarget<SPIRVSubtarget>();
  F = &MF->getFunction();

  if (needsEntryPoint()) {
    MIRB = MachineIRBuilder(*MF);
    MIRB->setInsertPt(MF->front(), MF->front().instr_begin());

    findFunctionVReg();
    buildEntryPointInterface();
    buildEntryPoint();
  }

  Interface.clear();
  MIRB = None;
  FVReg = None;

  return Changed;
}

void SPIRVEntryPoints::findFunctionVReg() {
  for (const auto &MB : *MF) {
    for (const auto &I : MB) {
      if (I.getOpcode() == OpFunction) {
        FVReg = I.defs().begin()->getReg();
        return;
      }
    }
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  MF->dump();
#endif
  llvm_unreachable("no OpFunction found for function");
}

void SPIRVEntryPoints::buildEntryPointInterface() {
  // Go through all instructions and look for global OpVariables.
  for (const auto &MB : *MF) {
    for (const auto &I : MB) {
      if (I.getOpcode() == OpFunctionCall) {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
        MF->dump();
#endif
        llvm_unreachable("unimplemented function call");
      }
      if (I.getOpcode() != SPIRV::OpVariable) {
        continue;
      }

      const auto SC = static_cast<StorageClass>(I.getOperand(2).getImm());
      switch (SC) {
      case StorageClass::Workgroup:
      case StorageClass::CrossWorkgroup:
      case StorageClass::Private:
      case StorageClass::Function:
        // Not global
        break;
      default:
        Interface.push_back(I.defs().begin()->getReg());
        break;
      }
    }
  }
}
void SPIRVEntryPoints::buildEntryPoint() {
  auto ExecModel = ExecutionModel::Kernel;
  if (ST->getTargetTriple().isVulkanEnvironment()) {
    ExecModel = ExecutionModel::GLCompute;
  }
  if (Metadata *MD = F->getMetadata(kMD::EntryExeModel)) {
    if (auto *Tuple = dyn_cast<MDTuple>(MD)) {
      if (Tuple->getNumOperands() >= 1) {
        MD = &*Tuple->getOperand(0);
      }
    }
    if (auto E = decodeExecutionModelMD(MD)) {
      ExecModel = *E;
    }
  }
  auto MIB =
      MIRB->buildInstr(OpEntryPoint).addImm((uint32_t)ExecModel).addUse(*FVReg);
  addStringImm(F->getName(), MIB);

  for (auto &GV : Interface) {
    MIB.addUse(GV);
  }

  if (auto *MD = F->getMetadata(kMD::ExecutionMode)) {
    for (const auto &Operand : MD->operands()) {
      const auto *Tuple = dyn_cast<MDNode>(&*Operand);
      if (!Tuple) {
        continue;
      }

      auto Operands = Tuple->operands();

      const auto *Begin = Operands.begin();
      auto Next = [&]() {
        if (Begin != Operands.end()) {
          return &**(Begin++);
        } else {
          return (Metadata *)nullptr;
        }
      };

      auto EMode = decodeExecutionModeMD(Next());
      if (!EMode) {
        continue;
      }

      auto MI = MIRB->buildInstr(OpExecutionMode)
                    .addUse(*FVReg)
                    .addImm((uint32_t)*EMode);

      switch (*EMode) {
      case ExecutionMode::Invocations: {
        // requires an operand, but don't panic if not present (not our job).
        if (auto O = decodeMDConstInt(Next())) {
          MI.addImm(*O);
        }
        break;
      }
      case ExecutionMode::LocalSize:
      case ExecutionMode::LocalSizeHint: {
        unsigned X = 1, Y = 1, Z = 1;

        if (auto Opt = decodeMDConstInt(Next())) {
          X = *Opt;
        }
        if (auto Opt = decodeMDConstInt(Next())) {
          Y = *Opt;
        }
        if (auto Opt = decodeMDConstInt(Next())) {
          Z = *Opt;
        }

        MI.addImm(X).addImm(Y).addImm(Z);
        break;
      }
      case ExecutionMode::VecTypeHint:
      case ExecutionMode::SubgroupSize:
      case ExecutionMode::SubgroupsPerWorkgroup: {
        unsigned X = 1;
        if (auto Opt = decodeMDConstInt(Next())) {
          X = *Opt;
        }
        MI.addImm(X);
        break;
      }
      default:
        break;
      }
    }
  }
}
