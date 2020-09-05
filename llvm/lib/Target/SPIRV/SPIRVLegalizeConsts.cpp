//===-- SPIRVPtrAnalysis.cpp - Hoist Globals & Number VRegs - C++ ===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Legalize constant expressions by "lowering" them into instructions and
// inserting them into functions where they're used.
//
//===----------------------------------------------------------------------===//

#include "SPIRVLegalizeConsts.h"
#include "SPIRV.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/IR/Dominators.h"

#define DEBUG_TYPE "spirv-legalize-consts"

using namespace llvm;

PreservedAnalyses SPIRVLegalizeConsts::runImpl(Function &F,
                                               const SPIRVSubtarget &RunST) {
  ST = &RunST;

  if (ST->HasKernel) {
    // The Kernel Capability supports everything. We have no work to do.
    // (Well, OpQuantizeToF16 isn't supported, but LLVM doesn't support that)
    PreservedAnalyses PA;
    PA.preserveSet<CFGAnalyses>();
    PA.preserve<GlobalsAA>();
    return PA;
  }

  for (auto &BB : F.getBasicBlockList()) {
    for (auto &I : BB) {
      WorkList.insert(&I);
    }
  }

  while (!WorkList.empty()) {
    auto *Top = WorkList.pop_back_val();
    for (unsigned OpIt = 0; OpIt < Top->getNumOperands(); ++OpIt) {
      auto *Op = Top->getOperand(OpIt);
      if (auto *CE = dyn_cast<ConstantExpr>(Op)) {
        if (!legalExpr(CE)) {
          auto *NewOp = legalizeOperand(CE, Top);
          Top->setOperand(OpIt, NewOp);
        }
      }
    }
  }

  PreservedAnalyses PA;
  PA.preserveSet<CFGAnalyses>();
  PA.preserve<GlobalsAA>();
  return PA;
}
bool SPIRVLegalizeConsts::legalExpr(ConstantExpr *CE) const {
  if (!legalOpcode(CE)) {
    return false;
  }

  for (auto &Op : CE->operands()) {
    if (auto *CE = dyn_cast<ConstantExpr>(Op.get())) {
      if (!legalExpr(CE)) {
        return false;
      }
    }
  }
  return true;
}
bool SPIRVLegalizeConsts::legalOpcode(ConstantExpr *CE) const {
  const auto Code = CE->getOpcode();
  switch (Code) {
  case Instruction::FPToSI:
  case Instruction::FPToUI:
  case Instruction::SIToFP:
  case Instruction::UIToFP:
  case Instruction::Trunc:
  case Instruction::ZExt:
  case Instruction::SExt:
  case Instruction::PtrToInt:
  case Instruction::IntToPtr:
  case Instruction::AddrSpaceCast:
  case Instruction::BitCast:
  case Instruction::FNeg:
  case Instruction::FAdd:
  case Instruction::FSub:
  case Instruction::FMul:
  case Instruction::FDiv:
  case Instruction::FRem:
  case Instruction::GetElementPtr:
    return false;
  default:
    return true;
  }
}
Value *SPIRVLegalizeConsts::legalizeOperand(ConstantExpr *CE,
                                            Instruction *User) {
  if (CE->getOpcode() == Instruction::AddrSpaceCast) {
    if (isa<ConstantPointerNull>(CE->getOperand(0))) {
      return ConstantPointerNull::get(cast<PointerType>(CE->getType()));
    }
  }
  auto *I = CE->getAsInstruction();
  WorkList.insert(I);
  I->insertBefore(User);
  return I;
}

namespace llvm {
namespace {
class SPIRVLegalizeConstsPass : public FunctionPass {
  SPIRVLegalizeConsts Impl;

public:
  static char ID;

  SPIRVLegalizeConstsPass() : FunctionPass(ID) {
    initializeSPIRVLegalizeConstsPassPass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override {
    const TargetPassConfig &TPC = getAnalysis<TargetPassConfig>();
    const TargetMachine &TM = TPC.getTM<TargetMachine>();
    const SPIRVSubtarget &ST = TM.getSubtarget<SPIRVSubtarget>(F);
    auto PA = Impl.runImpl(F, ST);
    return !PA.areAllPreserved();
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<TargetPassConfig>();
    AU.addPreserved<GlobalsAAWrapperPass>();
    AU.setPreservesCFG();
  }
};
} // namespace
} // namespace llvm
INITIALIZE_PASS_BEGIN(SPIRVLegalizeConstsPass, DEBUG_TYPE,
                      "SPIRV legalize constant expressions", false, false)
INITIALIZE_PASS_DEPENDENCY(TargetPassConfig)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_END(SPIRVLegalizeConstsPass, DEBUG_TYPE,
                    "SPIRV legalize constant expressions", false, false)

char SPIRVLegalizeConstsPass::ID = 0;

FunctionPass *llvm::createSPIRVLegalizeConstsPass() {
  return new SPIRVLegalizeConstsPass();
}
