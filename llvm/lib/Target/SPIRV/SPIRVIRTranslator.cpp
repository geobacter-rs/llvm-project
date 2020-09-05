//===-- SPIRVIRTranslator.cpp - Override IRTranslator for SPIR-V *- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Overrides GlobalISel's IRTranslator to customize default lowering behavior.
// This is mostly to stop aggregate types like Structs from getting expanded
// into scalars, and to maintain type information required for SPIR-V using the
// SPIRVTypeRegistry before it gets discarded as LLVM IR is lowered to GMIR.
//
//===----------------------------------------------------------------------===//

#include "SPIRVIRTranslator.h"

#include "SPIRV.h"
#include "SPIRVStrings.h"
#include "SPIRVSubtarget.h"
#include "SPIRVMetadata.h"

#include "llvm/Analysis/OptimizationRemarkEmitter.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/Constants.h"
#include "llvm/Target/TargetIntrinsicInfo.h"

using namespace llvm;
using namespace SPIRV;

static bool buildOpConstantNull(const Constant &C, Register Reg,
                                MachineIRBuilder &MIRBuilder,
                                SPIRVTypeRegistry *TR) {
  SPIRVType *type = TR->getOrCreateSPIRVType(C.getType(), MIRBuilder);
  auto MIB = MIRBuilder.buildInstr(SPIRV::OpConstantNull)
                 .addDef(Reg)
                 .addUse(TR->getSPIRVTypeID(type));
  return TR->constrainRegOperands(MIB);
}

static bool isConstantCompositeConstructible(const Constant &C) {
  if (isa<ConstantDataSequential>(C)) {
    return true;
  } else if (const auto *CV = dyn_cast<ConstantAggregate>(&C)) {
    return std::all_of(CV->op_begin(), CV->op_end(), [](const Use &operand) {
      const auto *constOp = cast_or_null<Constant>(operand);
      return constOp && (isa<ConstantData>(operand) ||
                         isConstantCompositeConstructible(*constOp));
    });
  }
  return false;
}

static bool buildOpConstantComposite(const Constant &C, Register Reg,
                                     const SmallVectorImpl<Register> &Ops,
                                     MachineIRBuilder &MIRBuilder,
                                     SPIRVTypeRegistry *TR) {
  bool areAllConst = isConstantCompositeConstructible(C);
  SPIRVType *type = TR->getOrCreateSPIRVType(C.getType(), MIRBuilder);
  auto MIB = MIRBuilder
                 .buildInstr(areAllConst ? SPIRV::OpConstantComposite
                                         : SPIRV::OpCompositeConstruct)
                 .addDef(Reg)
                 .addUse(TR->getSPIRVTypeID(type));
  for (const auto &Op : Ops) {
    MIB.addUse(Op);
  }
  return TR->constrainRegOperands(MIB);
}

bool SPIRVIRTranslator::buildGlobalVariable(Register Reg,
                                            const GlobalVariable *GV,
                                            MachineIRBuilder &MIRBuilder) {
  SPIRVType *resType = TR->getOrCreateSPIRVType(GV, *EntryBuilder);

  Register initVReg = 0;
  if (GV->hasInitializer()) {
    initVReg = getOrCreateVReg(*GV->getInitializer());
  }

  auto addrSpace = GV->getAddressSpace();
  auto storage = TR->addressSpaceToStorageClass(addrSpace);
  if (ST->HasLinkage) {
    if (GV->getLinkage() == GlobalValue::LinkageTypes::ExternalLinkage &&
        storage != StorageClass::Function) {
      auto MIB = MIRBuilder.buildInstr(SPIRV::OpDecorate)
                     .addUse(Reg)
                     .addImm((uint32_t)Decoration::LinkageAttributes);
      addStringImm(GV->getName(), MIB);
      MIB.addImm((uint32_t)LinkageType::Export);
    }
  }

  auto MIB = MIRBuilder.buildInstr(SPIRV::OpVariable)
                 .addDef(Reg)
                 .addUse(TR->getSPIRVTypeID(resType))
                 .addImm((uint32_t)storage);
  // XXX This is useless because the OpName instructions aren't used, and
  // thus are deleted. But at the same time, marking OpName as having side
  // effects is also not workable as then dead globals won't be eliminated.
  buildOpName(Reg, GV->getName(), MIRBuilder);

  if (initVReg != 0 && storage != StorageClass::Input) {
    MIB.addUse(initVReg);
    // TODO should this check be here?
    // using namespace StorageClass;
    // if (storage != UniformConstant) {
    //   auto ucAddrspace = TR->StorageClassToAddressSpace(UniformConstant);
    //   errs() << "OpVariable initializers are only valid for the "
    //             "UniformConstant storage class (addrspace "
    //          << ucAddrspace << ") in: " << *GV << "\n";
    //   return false;
    // }
  }

  auto Decorate = [&](unsigned MDKindId, Decoration Dec) {
    if(auto* MD = GV->getMetadata(MDKindId)) {
      if(auto Id = decodeMDConstInt(MD)) {
        MIRBuilder.buildInstr(SPIRV::OpDecorate)
            .addDef(Reg)
            .addImm((uint32_t)Dec)
            .addImm(*Id);
      }
    }
  };
  Decorate(PipelineBindingMDKindID, Decoration::Binding);
  Decorate(PipelineDescSetMDKindID, Decoration::DescriptorSet);

  return TR->constrainRegOperands(MIB);
}

bool SPIRVIRTranslator::translate(const Constant &C, Register Reg) {
  if (isa<ConstantPointerNull>(C) || isa<ConstantAggregateZero>(C)) {
    // Null (needs handled here to not loose the type info)
    return buildOpConstantNull(C, Reg, *EntryBuilder, TR);
  } else if (auto GV = dyn_cast<GlobalVariable>(&C)) {
    // Global OpVariables (possibly with constant initializers)
    return buildGlobalVariable(Reg, GV, *EntryBuilder);
  } else if (auto CV = dyn_cast<ConstantDataSequential>(&C)) {
    // Vectors or arrays via OpConstantComposite
    if (CV->getNumElements() == 1)
      return translate(*CV->getElementAsConstant(0), Reg);
    SmallVector<Register, 4> Ops;
    for (unsigned i = 0; i < CV->getNumElements(); ++i) {
      Ops.push_back(getOrCreateVReg(*CV->getElementAsConstant(i)));
    }
    return buildOpConstantComposite(C, Reg, Ops, *EntryBuilder, TR);
  } else if (auto CV = dyn_cast<ConstantAggregate>(&C)) {
    // Structs via OpConstantComposite
    if (C.getNumOperands() == 1)
      return translate(*CV->getOperand(0), Reg);
    SmallVector<Register, 4> Ops;
    for (unsigned i = 0; i < C.getNumOperands(); ++i) {
      Ops.push_back(getOrCreateVReg(*C.getOperand(i)));
    }
    return buildOpConstantComposite(C, Reg, Ops, *EntryBuilder, TR);
  } else {
    // Default behavior is fine as Ints, Floats etc, won't get flattened
    return IRTranslator::translate(C, Reg);
  }
}

bool SPIRVIRTranslator::translateBitCast(const User &U,
                                         MachineIRBuilder &MIRBuilder) {
  // Maintain type info by always using G_BITCAST instead of just copying vreg
  return IRTranslator::translateCast(TargetOpcode::G_BITCAST, U, MIRBuilder);
}

ArrayRef<Register> SPIRVIRTranslator::getOrCreateVRegs(const Value &Val) {
  Type *Ty = Val.getType();

  const auto MRI = EntryBuilder->getMRI();

  ArrayRef<Register> ResVRegs;

  auto VRegsIt = VMap.findVRegs(Val);
  if (VRegsIt != VMap.vregs_end()) {
    ResVRegs = *VRegsIt->second;
  } else if (Ty->isVoidTy()) {
    ResVRegs = *VMap.getVRegs(Val);
  } else {
    assert(Ty->isSized() && "Cannot create unsized vreg");

    // Ensure type definition appears before uses. This is especially important
    // for values like OpConstant which get hoisted alongside types later
    SPIRVType *SPIRVTy = nullptr;
    if (auto *GV = dyn_cast<GlobalVariable>(&Val)) {
      SPIRVTy = PtrAnalysis->visitGlobal(GV, EntryBuilder.get());
    }

    if (!SPIRVTy) {
      SPIRVTy = TR->getOrCreateSPIRVType(&Val, *EntryBuilder);
    }

    // Create entry for this type.
    auto *NewVRegs = VMap.getVRegs(Val);
    auto llt = getLLTForType(*Ty, *DL);
    Register vreg = MRI->createGenericVirtualRegister(llt);
    NewVRegs->push_back(vreg);

    if (auto C = dyn_cast<Constant>(&Val)) {
      bool success = translate(*C, vreg);
      assert(success && "Could not translate constant");
    }
    ResVRegs = *NewVRegs;

    // Add type and name metadata
    if (!TR->hasSPIRVTypeForVReg(ResVRegs[0])) {
      TR->assignSPIRVTypeToVReg(SPIRVTy, ResVRegs[0], *EntryBuilder);
      if (Val.hasName()) {
        buildOpName(ResVRegs[0], Val.getName(), *EntryBuilder);
      }
    }
  }
  // Make sure there's always at least 1 placeholder offset to avoid crashes
  auto *Offsets = VMap.getOffsets(Val);
  if (Offsets->empty()) {
    Offsets->push_back(0);
  }
  return ResVRegs;
}

bool SPIRVIRTranslator::runOnMachineFunction(MachineFunction &MF) {
  // Initialize the type registry
  this->ST = static_cast<const SPIRVSubtarget *>(&MF.getSubtarget());
  this->TR = ST->getSPIRVTypeRegistry();
  this->PtrAnalysis = SPIRVPtrAnalysis(this->TR, &MF.getFunction());

  const auto* M = MF.getFunction().getParent();
  if(!this->PipelineBindingMDKindID) {
    this->PipelineBindingMDKindID = M->getMDKindID(kMD::GlobalPipelineBinding);
  }
  if(!this->PipelineDescSetMDKindID) {
    this->PipelineDescSetMDKindID = M->getMDKindID(kMD::GlobalPipelineDescSet);
  }

  // Run the regular IRTranslator
  bool success = IRTranslator::runOnMachineFunction(MF);

  // Clean up
  TR->reset();
  this->PtrAnalysis = None;
  return success;
}

bool SPIRVIRTranslator::translateGetElementPtr(const User &U,
                                               MachineIRBuilder &MIRBuilder) {
  using namespace SPIRV;

  // Determine which AccessChain opcode to use
  bool isPtrAccess = false;
  if (auto firstIndex = dyn_cast<ConstantInt>(U.getOperand(1))) {
    isPtrAccess = !firstIndex->isZero();
  }
  bool isInBounds = false;
  if (auto gepOp = dyn_cast<GEPOperator>(&U)) {
    isInBounds = gepOp->isInBounds();
  }
  unsigned opCode = isInBounds ? OpInBoundsAccessChain : OpAccessChain;
  if (isPtrAccess) {
    opCode = isInBounds ? OpInBoundsPtrAccessChain : OpPtrAccessChain;
  }

  // Build the AccessChain, but don't insert until all operands have been
  // assigned vregs
  Register baseVReg = getOrCreateVReg(*U.getOperand(0));
  Register resVReg = getOrCreateVReg(U);
  SPIRVType *resType = TR->getSPIRVTypeForVReg(resVReg);
  auto MIB = MIRBuilder.buildInstrNoInsert(opCode)
                 .addDef(resVReg)
                 .addUse(TR->getSPIRVTypeID(resType))
                 .addUse(baseVReg);

  // Create VRegs (and maybe G_CONSTANTs) for all operands
  const unsigned int numOperands = U.getNumOperands();
  for (unsigned int i = isPtrAccess ? 1 : 2; i < numOperands; ++i) {
    MIB.addUse(getOrCreateVReg(*U.getOperand(i)));
  }

  // Insert the AccessChain after operand definitions
  MIRBuilder.insertInstr(MIB);
  return TR->constrainRegOperands(MIB);
}

bool SPIRVIRTranslator::translateShuffleVector(const User &U,
                                               MachineIRBuilder &MIRBuilder) {
  auto ShuffleInst = dyn_cast<ShuffleVectorInst>(&U);

  auto V0 = getOrCreateVReg(*ShuffleInst->getOperand(0));
  auto V1 = getOrCreateVReg(*ShuffleInst->getOperand(1));
  auto ResVReg = getOrCreateVReg(*ShuffleInst);
  auto ResVRegType = TR->getSPIRVTypeForVReg(ResVReg);
  auto MIB = MIRBuilder.buildInstr(SPIRV::OpVectorShuffle)
                 .addDef(ResVReg)
                 .addUse(TR->getSPIRVTypeID(ResVRegType))
                 .addUse(V0)
                 .addUse(V1);
  for (unsigned idx : ShuffleInst->getShuffleMask()) {
    MIB.addImm(idx);
  }
  return TR->constrainRegOperands(MIB);
}

bool SPIRVIRTranslator::translateExtractValue(const User &U,
                                              MachineIRBuilder &MIRBuilder) {
  auto ExtractInst = dyn_cast<ExtractValueInst>(&U);
  auto AggVReg = getOrCreateVReg(*ExtractInst->getAggregateOperand());
  auto ResVReg = getOrCreateVReg(*ExtractInst);
  auto ResVRegType = TR->getSPIRVTypeForVReg(ResVReg);
  auto MIB = MIRBuilder.buildInstr(SPIRV::OpCompositeExtract)
                 .addDef(ResVReg)
                 .addUse(TR->getSPIRVTypeID(ResVRegType))
                 .addUse(AggVReg);
  for (unsigned idx : ExtractInst->indices()) {
    MIB.addImm(idx);
  }
  return TR->constrainRegOperands(MIB);
}

bool SPIRVIRTranslator::translateInsertValue(const User &U,
                                             MachineIRBuilder &MIRBuilder) {
  auto InsertInst = dyn_cast<InsertValueInst>(&U);
  auto AggVReg = getOrCreateVReg(*InsertInst->getAggregateOperand());
  auto toInsertVReg = getOrCreateVReg(*InsertInst->getInsertedValueOperand());
  auto ResVReg = getOrCreateVReg(*InsertInst);
  auto ResVRegType = TR->getSPIRVTypeForVReg(ResVReg);
  auto MIB = MIRBuilder.buildInstr(SPIRV::OpCompositeInsert)
                 .addDef(ResVReg)
                 .addUse(TR->getSPIRVTypeID(ResVRegType))
                 .addUse(toInsertVReg)
                 .addUse(AggVReg);
  for (unsigned idx : InsertInst->indices()) {
    MIB.addImm(idx);
  }
  return TR->constrainRegOperands(MIB);
}

// Retrieve an unsigned int from an MDNode with a list of them as operands
static unsigned int getMetadataUInt(const MDNode *mdNode, unsigned int opIndex,
                                    unsigned int defaultVal = 0) {
  if (mdNode && opIndex < mdNode->getNumOperands()) {
    const auto &op = mdNode->getOperand(opIndex);
    return mdconst::extract<ConstantInt>(op)->getZExtValue();
  } else {
    return defaultVal;
  }
}

static void addMemoryOperands(const Instruction *val, unsigned int alignment,
                              bool isVolatile, MachineInstrBuilder &MIB) {
  uint32_t spvMemOp = (uint32_t)MemoryOperand::None;
  if (isVolatile) {
    spvMemOp |= (uint32_t)MemoryOperand::Volatile;
  }
  if (getMetadataUInt(val->getMetadata("nontemporal"), 0) == 1) {
    spvMemOp |= (uint32_t)MemoryOperand::Nontemporal;
  }
  if (alignment != 0) {
    spvMemOp |= (uint32_t)MemoryOperand::Aligned;
  }

  if (spvMemOp != (uint32_t)MemoryOperand::None) {
    MIB.addImm(spvMemOp);
    if (spvMemOp & (uint32_t)MemoryOperand::Aligned) {
      MIB.addImm(alignment);
    }
  }
}

bool SPIRVIRTranslator::translateLoad(const User &U,
                                      MachineIRBuilder &MIRBuilder) {
  auto Load = cast<LoadInst>(&U);
  auto ResVReg = getOrCreateVReg(*Load);
  auto ResVRegType = TR->getSPIRVTypeForVReg(ResVReg);
  auto Ptr = getOrCreateVReg(*Load->getPointerOperand());

  auto LoadSPIRVTy = TR->getOrCreateSPIRVType(Load, *EntryBuilder);
  auto LoadedSPIRVTy = TR->getOrCreateSPIRVType(Load->getType(), *EntryBuilder);
  auto MIB = MIRBuilder.buildInstr(SPIRV::OpLoad)
                 .addDef(ResVReg)
                 .addUse(TR->getSPIRVTypeID(ResVRegType))
                 .addUse(Ptr);
  addMemoryOperands(Load, Load->getAlignment(), Load->isVolatile(), MIB);
  if (LoadSPIRVTy == LoadedSPIRVTy) {
    // no OpCopyLogical required
    return TR->constrainRegOperands(MIB);
  } else {
    // We need a OpCopyLogical to "cast" the type to the expected type.
    // This is required to work around the mistake that are structure member
    // decorations.
    auto CpyLogicalReg = MRI->createGenericVirtualRegister(LLT());
    auto CpyObjMIB = MIRBuilder.buildInstr(SPIRV::OpCopyObject)
                         .addDef(CpyLogicalReg)
                         .addUse(ResVReg);
    TR->assignSPIRVTypeToVReg(LoadedSPIRVTy, CpyLogicalReg, MIRBuilder);

    return TR->constrainRegOperands(MIB) ||
           TR->constrainRegOperands(CpyObjMIB);
  }
}

bool SPIRVIRTranslator::translateStore(const User &U,
                                       MachineIRBuilder &MIRBuilder) {
  auto Store = cast<StoreInst>(&U);
  auto Ptr = getOrCreateVReg(*Store->getPointerOperand());
  auto Obj = getOrCreateVReg(*Store->getValueOperand());
  auto PtrTy = TR->getSPIRVTypeForVReg(Ptr);
  auto ObjTy = TR->getSPIRVTypeForVReg(Obj);
  assert(PtrTy->getOpcode() == SPIRV::OpTypePointer);
  auto StoredTy = TR->getSPIRVTypeForVReg(PtrTy->getOperand(2).getReg());
  if (StoredTy != ObjTy) {
    // insert an OpCopyLogical "cast":
    auto CpyLogicalReg = MRI->createGenericVirtualRegister(LLT());
    MIRBuilder.buildInstr(SPIRV::OpCopyObject)
        .addDef(CpyLogicalReg)
        .addUse(Obj);
    TR->assignSPIRVTypeToVReg(StoredTy, CpyLogicalReg, MIRBuilder);

    Obj = CpyLogicalReg;
  }
  auto MIB = MIRBuilder.buildInstr(SPIRV::OpStore).addUse(Ptr).addUse(Obj);
  addMemoryOperands(Store, Store->getAlignment(), Store->isVolatile(), MIB);
  return TR->constrainRegOperands(MIB);
}

bool SPIRVIRTranslator::translateOverflowIntrinsic(
    const CallInst &CI, unsigned Op, MachineIRBuilder &MIRBuilder) {
  // Same as parent, but use Register(0) instead of ResRegs[1], as all structs
  // now use a single register
  ArrayRef<Register> ResRegs = getOrCreateVRegs(CI);
  MIRBuilder.buildInstr(Op)
      .addDef(ResRegs[0])
      .addDef(Register(0)) // Change this from Res[1]
      .addUse(getOrCreateVReg(*CI.getOperand(0)))
      .addUse(getOrCreateVReg(*CI.getOperand(1)));

  return true;
}

bool SPIRVIRTranslator::translateAtomicCmpXchg(const User &U,
                                               MachineIRBuilder &MIRBuilder) {
  // Same as parent class except use Register(0) instead of Res[1] to avoid
  // crash as all structs now use a single vreg rather than getting flattened
  // Also, avoid type checks for 2nd reg by using MIRBuilder.buildInstr instead
  // of MIRBuilder.buildAtomicCmpXchgWithSuccess;
  const AtomicCmpXchgInst &I = cast<AtomicCmpXchgInst>(U);

  if (I.isWeak())
    return false;

  auto Flags = I.isVolatile() ? MachineMemOperand::MOVolatile
                              : MachineMemOperand::MONone;
  Flags |= MachineMemOperand::MOLoad | MachineMemOperand::MOStore;

  Type *ResType = I.getType();
  Type *ValType = ResType->Type::getStructElementType(0);

  auto Res = getOrCreateVRegs(I);
  Register OldValRes = Res[0];
  Register SuccessRes = Register(0); // Change this from Res[1]
  Register Addr = getOrCreateVReg(*I.getPointerOperand());
  Register Cmp = getOrCreateVReg(*I.getCompareOperand());
  Register NewVal = getOrCreateVReg(*I.getNewValOperand());

  // Use buildInstr instead of buildAtomicCmpXchgWithSuccess to avoid asserts
  MIRBuilder.buildInstr(TargetOpcode::G_ATOMIC_CMPXCHG_WITH_SUCCESS)
      .addDef(OldValRes)
      .addDef(SuccessRes)
      .addUse(Addr)
      .addUse(Cmp)
      .addUse(NewVal)
      .addMemOperand(MF->getMachineMemOperand(
          MachinePointerInfo(I.getPointerOperand()), Flags,
          DL->getTypeStoreSize(ValType), getMemOpAlignment(I), AAMDNodes(),
          nullptr, I.getSyncScopeID(), I.getSuccessOrdering(),
          I.getFailureOrdering()));

  return true;
}
