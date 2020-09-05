//===-- SPIRVStructCF.cpp ---------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Insert required OpLoopMerge && OpSelectionMerge instructions. This pass
// assumes SSA loops are simplified.
//
//===----------------------------------------------------------------------===//

#include "SPIRV.h"
#include "SPIRVSubtarget.h"

#include "llvm/CodeGen/GlobalISel/MachineIRBuilder.h"
#include "llvm/CodeGen/MachinePostDominators.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineLoopInfo.h"

using namespace llvm;
using namespace SPIRV;

#define DEBUG_TYPE "spirv-structured-control-flow"

namespace {
class SPIRVStructCF : public MachineFunctionPass {
  bool Changed = false;
  const SPIRVSubtarget *ST = nullptr;
  MachineFunction *MF = nullptr;
  MachinePostDominatorTree* MPDT;
  MachineLoopInfo *MLI = nullptr;

  /// An unreachable block to be used as the merge block in infinite loops.
  /// This is created on demand, since an actual infinite loop will almost
  /// certainly never happen.
  MachineBasicBlock *Unreachable = nullptr;

  /// Unprocessed blocks. As we annotate loops, we remove various loop blocks
  /// which have may have OpBranchConditional terminators but shouldn't get a
  /// OpSelectionMerge (because they'll get an OpLoopMerge).
  SmallPtrSet<MachineBasicBlock *, 32> Blocks;

  Optional<MachineIRBuilder> MIRB;

public:
  static char ID;
  SPIRVStructCF() : MachineFunctionPass(ID) {
    initializeSPIRVStructCFPass(*PassRegistry::getPassRegistry());
  }
  bool runOnMachineFunction(MachineFunction &MF) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<MachineLoopInfo>();
    AU.addPreserved<MachineLoopInfo>();
    AU.addRequired<MachinePostDominatorTree>();
    AU.addPreserved<MachinePostDominatorTree>();
    MachineFunctionPass::getAnalysisUsage(AU);
  }
  StringRef getPassName() const override {
    return "SPIRV Control Flow Annotator";
  }

  inline MachineBasicBlock *getUnreachableBlock() {
    if (!Unreachable) {
      Unreachable = MF->CreateMachineBasicBlock();
    }

    return Unreachable;
  }

  inline bool shouldAnnotateSelection(MachineBasicBlock *MB) const {
    if (MB->getFirstInstrTerminator() == MB->instr_end()) {
      // fallthrough "branch"
      return false;
    }

    assert(++MB->getFirstInstrTerminator() == MB->instr_end());
    const auto &Term = *MB->getFirstTerminator();
    const auto Opcode = Term.getOpcode();
    return Opcode == OpBranchConditional || Opcode == OpSwitch;
  }

  void annotateLoop(const MachineLoop *ML);
  void annotateSelection(MachineBasicBlock *MB);
};
} // namespace
char SPIRVStructCF::ID = 0;
char &llvm::SPIRVStructCFID = SPIRVStructCF::ID;

INITIALIZE_PASS_BEGIN(SPIRVStructCF, DEBUG_TYPE, "SPIRV Control Flow Annotator",
                      false, false)
INITIALIZE_PASS_END(SPIRVStructCF, DEBUG_TYPE, "SPIRV Control Flow Annotator",
                    false, false)

bool SPIRVStructCF::runOnMachineFunction(MachineFunction &MF_) {
  Changed = false;
  MF = &MF_;
  ST = &MF->getSubtarget<SPIRVSubtarget>();
  MPDT = &getAnalysis<MachinePostDominatorTree>();
  MLI = &getAnalysis<MachineLoopInfo>();
  Unreachable = nullptr;
  MIRB = MachineIRBuilder(*MF);

  for (auto &MB : *MF) {
    Blocks.insert(&MB);
  }

  for (auto TopLoopIt = MLI->begin(); TopLoopIt != MLI->end(); TopLoopIt++) {
    annotateLoop(*TopLoopIt);
  }

  // now annotate the remaining blocks (loop bodies and non-loop blocks)
  for (auto *MB : Blocks) {
    if (shouldAnnotateSelection(MB)) {
      annotateSelection(MB);
    }
  }

  MIRB = None;
  Blocks.clear();

  return Changed;
}

void SPIRVStructCF::annotateLoop(const MachineLoop *ML) {
  assert(ML->hasDedicatedExits());
  assert(ML->getLoopPreheader());
  assert(ML->getLoopLatch());

  SmallVector<MachineBasicBlock *, 8> Exits;
  ML->getExitBlocks(Exits);

  if (Exits.empty()) {
    // An infinite loop. SPIRV allows the merge block to not strictly
    // dominate the header when the merge block is unreachable.
    Exits.push_back(getUnreachableBlock());
  }

  auto *Merge = Exits.front();
  for (auto *It = Exits.begin() + 1; It != Exits.end(); ++It) {
    Merge = MPDT->findNearestCommonDominator(Merge, *It);
    assert(Merge && "no common merge block??");
  }

  auto *Preheader = ML->getLoopPreheader();
  auto *Header = ML->getHeader();
  auto *Latch = ML->getLoopLatch();

  // TODO decode parallel annotations into loop control operands
  if(Preheader->getFirstInstrTerminator() == Preheader->instr_end()) {
    MIRB->setMBB(*Preheader);
  } else {
    MIRB->setInstr(*Preheader->getFirstInstrTerminator());
  }
  MIRB->buildInstr(OpLoopMerge).addMBB(Merge).addMBB(Latch).addImm(0);
  Changed = true;

  Blocks.erase(Preheader);
  Blocks.erase(Header);
  Blocks.erase(Latch);

  // Now process any nested loops:
  for (const MachineLoop *InnerML : ML->getSubLoops()) {
    annotateLoop(InnerML);
  }
}
void SPIRVStructCF::annotateSelection(MachineBasicBlock *MB) {
  MachineBasicBlock *MergeB;

  auto &Term = *MB->getFirstInstrTerminator();
  const auto Opcode = Term.getOpcode();
  if (Opcode == OpBranchConditional) {
    auto *TrueB = Term.getOperand(1).getMBB();
    auto *FalseB = Term.getOperand(2).getMBB();

    MergeB = MPDT->findNearestCommonDominator(TrueB, FalseB);
    if (!MergeB) {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
      MB->getParent()->dump();
#endif
      report_fatal_error("no conditional branch merge block??");
    }

    MIRB->setInstr(Term);
    MIRB->buildInstr(OpSelectionMerge).addMBB(MergeB).addImm(0);
    Changed = true;
  } else if (Opcode == OpSwitch) {
    llvm_unreachable("TODO OpSwitch");
  } else {
    llvm_unreachable("unimplemented");
  }
}
