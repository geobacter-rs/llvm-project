//===-- SPIRV.h - Top-level interface for SPIR-V representation -*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SPIRV_SPIRV_H
#define LLVM_LIB_TARGET_SPIRV_SPIRV_H

#include "MCTargetDesc/SPIRVMCTargetDesc.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
class SPIRVTargetMachine;
class SPIRVRegisterBankInfo;
class SPIRVSubtarget;
class InstructionSelector;

FunctionPass *createSPIRVBasicBlockDominancePass();
FunctionPass *createSPIRVBlockLabelerPass();
FunctionPass *createSPIRVAddRequirementsPass();
FunctionPass *createSPIRVLegalizeConstsPass();
ModulePass *createSPIRVGlobalTypesAndRegNumPass();

InstructionSelector *
createSPIRVInstructionSelector(const SPIRVTargetMachine &TM,
                               const SPIRVSubtarget &Subtarget,
                               const SPIRVRegisterBankInfo &RBI);

void initializeSPIRVBasicBlockDominancePass(PassRegistry &);
void initializeSPIRVBlockLabelerPass(PassRegistry &);
void initializeSPIRVAddRequirementsPass(PassRegistry &);
void initializeSPIRVLegalizeConstsPassPass(PassRegistry &);
void initializeSPIRVGlobalTypesAndRegNumPass(PassRegistry &);

void initializeSPIRVStructCFPass(PassRegistry &);
extern char &SPIRVStructCFID;
void initializeSPIRVEntryPointsPass(PassRegistry &);
extern char &SPIRVEntryPointsID;
} // namespace llvm

#endif
