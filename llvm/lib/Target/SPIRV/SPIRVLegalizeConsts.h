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

#pragma once
#include "SPIRVSubtarget.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

class SPIRVLegalizeConsts : public PassInfoMixin<SPIRVLegalizeConsts> {
  const SPIRVSubtarget *ST = nullptr;
  SetVector<Instruction *> WorkList;

  bool legalExpr(ConstantExpr *CE) const;
  bool legalOpcode(ConstantExpr *CE) const;
  Value *legalizeOperand(ConstantExpr *CE, Instruction *User);

public:
  SPIRVLegalizeConsts() = default;
  PreservedAnalyses runImpl(Function &F, const SPIRVSubtarget &ST);
};

} // namespace llvm
