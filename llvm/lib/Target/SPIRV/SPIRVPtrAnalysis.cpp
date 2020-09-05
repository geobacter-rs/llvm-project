//===-- SPIRVPtrAnalysis.cpp - Hoist Globals & Number VRegs - C++ ===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Visit each value in a function and ensure that GEPs of GlobalVariables get
// the correct SPIRV type based on the GVs metadata.
//
//===----------------------------------------------------------------------===//

#include "SPIRVPtrAnalysis.h"

#include "SPIRV.h"
#include "SPIRVEnums.h"
#include "SPIRVMetadata.h"
#include "SPIRVTypeRegistry.h"

#include "llvm/Support/Debug.h"

using namespace llvm;
using namespace llvm::SPIRV;

#define DEBUG_TYPE "spirv-ptr-analysis"

SPIRVPtrAnalysis::SPIRVPtrAnalysis(SPIRVTypeRegistry *TR, const Function *F)
    : TR(TR), F(F) {
  GlobalTypeSpecMDKindID = F->getParent()->getMDKindID(kMD::GlobalTypeSpec);
  GlobalDecorationMDKindID = F->getParent()->getMDKindID(kMD::GlobalDecoration);
}

void SPIRVPtrAnalysis::enqueueUsers(Instruction &I) {
  for (Use &U : I.uses()) {
    if (VisitedUses.insert(&U).second) {
      UseToVisit NewU = {
          UseToVisit::UseAndIsOffsetKnownPair(&U, IsOffsetKnown),
          SPIRVTy,
      };
      Worklist.push_back(std::move(NewU));
    }
  }
}

SPIRVType *SPIRVPtrAnalysis::getImageSampledType(StringRef S) {
  if (S == "void") {
    return TR->getOpTypeVoid(*MIRB);
  }
  if (S == "f16") {
    return TR->getOpTypeFloat(16, *MIRB);
  }
  if (S == "f32") {
    return TR->getOpTypeFloat(32, *MIRB);
  }
  if (S == "f64") {
    return TR->getOpTypeFloat(64, *MIRB);
  }
  if (S == "i8") {
    return TR->getOpTypeInt(8, *MIRB, true);
  }
  if (S == "u8") {
    return TR->getOpTypeInt(8, *MIRB, false);
  }
  if (S == "i16") {
    return TR->getOpTypeInt(16, *MIRB, true);
  }
  if (S == "u16") {
    return TR->getOpTypeInt(16, *MIRB, false);
  }
  if (S == "i32") {
    return TR->getOpTypeInt(32, *MIRB, true);
  }
  if (S == "u32") {
    return TR->getOpTypeInt(32, *MIRB, false);
  }
  if (S == "i64") {
    return TR->getOpTypeInt(64, *MIRB, true);
  }
  if (S == "u64") {
    return TR->getOpTypeInt(64, *MIRB, false);
  }
  return nullptr;
}

SPIRVType *SPIRVPtrAnalysis::decodeMDOpaqueTypeSpec(const Metadata *This,
                                                    MDNode::op_range Operands) {
  const auto *Begin = Operands.begin();
  auto Next = [&]() {
    if (Begin != Operands.end()) {
      return &**(Begin++);
    } else {
      return (Metadata *)nullptr;
    }
  };

  auto TypeNameOpt = decodeMDStr(Next());
  if (!TypeNameOpt) {
    return nullptr;
  }

  const auto &TypeName = TypeNameOpt.getValue();

  auto decodeImageTS = [&]() -> SPIRVType * {
    SPIRVType *SampledTy;
    if (auto SampledTyStr = decodeMDStr(Next())) {
      SampledTy = getImageSampledType(SampledTyStr.getValue());
      if (!SampledTy) {
        return nullptr;
      }
    } else {
      return nullptr;
    }

    Dim D;
    if (auto DimOpt = decodeDimMD(Next())) {
      D = DimOpt.getValue();
    } else {
      return nullptr;
    }

    uint32_t Depth;
    if (auto DepthOpt = decodeMDConstInt(Next(), 2)) {
      Depth = DepthOpt.getValue();
    } else {
      return nullptr;
    }

    uint32_t Arrayed;
    if (auto ArrayedOpt = decodeMDConstInt(Next(), 1)) {
      Arrayed = ArrayedOpt.getValue();
    } else {
      return nullptr;
    }

    uint32_t MS;
    if (auto MSOpt = decodeMDConstInt(Next(), 1)) {
      MS = MSOpt.getValue();
    } else {
      return nullptr;
    }

    uint32_t Sampled;
    if (auto SampledOpt = decodeMDConstInt(Next(), 2)) {
      Sampled = SampledOpt.getValue();
    } else {
      return nullptr;
    }

    ImageFormat Format;
    if (auto FormatOpt = decodeImageFormatMD(Next())) {
      Format = FormatOpt.getValue();
    } else {
      return nullptr;
    }

    AccessQualifier AQ;
    if (auto AQOpt = decodeAccessQualifierMD(Next())) {
      AQ = *AQOpt;
    } else {
      return nullptr;
    }

    return TR->getOpTypeImage(*MIRB, SampledTy, D, Depth, Arrayed, MS, Sampled,
                              Format, AQ);
  };

  if (TypeName == "Image") {
    return decodeImageTS();
  } else if (TypeName == "Sampler") {
    // no operands
    return TR->getSamplerType(*MIRB);
  } else if (TypeName == "SampledImage") {
    if (const auto *ImgTy = decodeImageTS()) {
      return TR->getSampledImageType(ImgTy, *MIRB);
    }
  }
  return nullptr;
}
void SPIRVPtrAnalysis::decodeMDDecorations(const Metadata *MD,
                                           SPIRVType *Target,
                                           Optional<uint32_t> Member) {
  if (const auto *Node = dyn_cast_or_null<MDNode>(MD)) {
    for (const auto &Operand : Node->operands()) {
      decodeMDDecoration(&*Operand, Target, Member);
    }
  }
}
void SPIRVPtrAnalysis::decodeMDDecoration(const Metadata *MD, SPIRVType *Target,
                                          Optional<uint32_t> Member) {
  if (!MD) {
    return;
  }

  auto Opcode = OpDecorate;
  if (Member) {
    Opcode = OpMemberDecorate;
  }

  if (auto DecOpt = decodeDecorationMD(MD)) {
    auto MIB = MIRB->buildInstr(Opcode).addUse(TR->getSPIRVTypeID(Target));
    if (Member) {
      MIB.addImm(*Member);
    }
    MIB.addImm(static_cast<int64_t>(*DecOpt));
  } else if (auto *OpMD = dyn_cast<MDNode>(MD)) {
    if (OpMD->getNumOperands() == 0) {
      return;
    }

    // a decoration with data operands
    auto MIB =
        MIRB->buildInstrNoInsert(Opcode).addUse(TR->getSPIRVTypeID(Target));
    if (Member) {
      MIB.addImm(*Member);
    }
    if (auto DecOpt = decodeDecorationMD(OpMD->getOperand(0))) {
      MIB.addImm(static_cast<int64_t>(*DecOpt));
    } else {
      return;
    }

    for (unsigned I = 1; I != OpMD->getNumOperands(); ++I) {
      const auto &DecOperand = OpMD->getOperand(I);

      if (auto Lit = decodeMDConstInt(&*DecOperand)) {
        MIB.addImm(*Lit);
      } else {
        return;
      }
    }

    MIRB->insertInstr(MIB);
  }
}

SPIRVType *SPIRVPtrAnalysis::transTypeSpecMDInner(
    Type *Ty, const Metadata *Spec,
    SPIRVPtrAnalysis::TypeSpecVisitedMD &Visited, SPIRVType *FwdPtr) {
  const auto *MD = dyn_cast_or_null<MDNode>(Spec);
  if (!MD || MD->getNumOperands() != 2) {
    return nullptr;
  }
  const auto *TypeSpec = dyn_cast<MDNode>(&*MD->getOperand(0));
  const auto *DecSpec = dyn_cast<MDNode>(&*MD->getOperand(1));
  return transTypeSpecMDInner(Ty, TypeSpec, DecSpec, Visited, FwdPtr);
}

// Spec is a two element tuple. The first element is the type spec, and the
// second is the decorations.
SPIRVType *SPIRVPtrAnalysis::transTypeSpecMDInner(Type *Ty,
                                                  const MDNode *TypeSpec,
                                                  const MDNode *DecSpec,
                                                  TypeSpecVisitedMD &Visited,
                                                  SPIRVType *FwdPtr) {

  if (!TypeSpec) {
    return nullptr;
  }

  if (DecSpec && DecSpec->getNumOperands() == 0) {
    DecSpec = nullptr;
  }
  // XXX
  const auto* OriginalDecSpec = DecSpec;
  if (isa<PointerType>(Ty)) {
    DecSpec = nullptr;
  }

  const TypeSpecVisitedKey Key = {
      Ty,
      {
          TypeSpec,
          DecSpec,
      },
  };
  auto Inserted = Visited.insert({
      Key,
      nullptr,
  });
  if (!Inserted.second) {
    return Inserted.first->second;
  }

  LLVM_DEBUG(dbgs() << "Decoding " << *Ty << " type spec: " << *TypeSpec);
  if(DecSpec) {
    LLVM_DEBUG(dbgs() << " decorations: " << *DecSpec << "\n");
  } else {
    LLVM_DEBUG(dbgs() << "\n");
  }

  Cleanup Cleanup_{this, Visited, Key, nullptr};

  SPIRVType *&Out = Cleanup_.Out;

  Type *ElemTy = nullptr;
  SPIRVType *SPIRVElemTy = nullptr;
  SPIRVType *ElemFwdPtr = nullptr;
  StorageClass SC = StorageClass::Generic;

  auto *PTy = dyn_cast<PointerType>(Ty);
  auto *ATy = dyn_cast<ArrayType>(Ty);
  auto *VTy = dyn_cast<VectorType>(Ty);
  if (PTy) {
    ElemTy = PTy->getPointerElementType();
    SC = TR->addressSpaceToStorageClass(PTy->getAddressSpace());
  } else if (VTy) {
    ElemTy = VTy->getVectorElementType();
  } else if (ATy) {
    ElemTy = ATy->getArrayElementType();
  }
  if (ElemTy && !VTy && !ATy) {
    assert(PTy);
    if (isa<StructType>(ElemTy)) {
      // XXX redefinition of a virtual register.
      //ElemFwdPtr = TR->getOpTypeForwardPointer(SC, *MIRB);
      Out = ElemFwdPtr;
    }
    // VTy must have a scalar element type, so can't have any
    // special types which get encoded in this metadata.
    SPIRVElemTy =
        transTypeSpecMDInner(ElemTy, TypeSpec, OriginalDecSpec, Visited, ElemFwdPtr);
    if (!SPIRVElemTy) {
      return nullptr;
    }
  }

  auto TypeSpecOperands = TypeSpec->operands();
  Optional<StringRef> TypeSpecKind = None;
  MDNode::op_iterator TypeSpecIt = TypeSpecOperands.end();

  if (TypeSpec->getNumOperands() > 0) {
    TypeSpecIt = TypeSpecOperands.begin();
    TypeSpecKind = decodeMDStr(&**(TypeSpecIt++));
  }
  auto NextSpec = [&]() {
    if (TypeSpecIt != TypeSpecOperands.end()) {
      auto *InnerTypeMD = &**(TypeSpecIt++);
      if (auto *InnerTypeSpec = dyn_cast<MDTuple>(InnerTypeMD)) {
        if (InnerTypeSpec->getNumOperands() == 2) {
          return InnerTypeSpec;
        }
      }
    }
    return (MDTuple *)nullptr;
  };

  if (PTy) {
    Out = TR->getOpTypePointer(SC, SPIRVElemTy, *MIRB, ElemFwdPtr);
    TR->assignSPIRVTypeToVReg(Out, Out->getOperand(0).getReg(), *MIRB);
  } else if (VTy) {
    Optional<std::pair<unsigned, unsigned>> Params = None;
    if (TypeSpecKind == StringRef("Matrix")) {
      if (auto *MatrixSpec = NextSpec()) {
        if (auto *MDParams = dyn_cast<MDTuple>(MatrixSpec->getOperand(0))) {
          if (MDParams->getNumOperands() >= 2) {
            auto Columns = decodeMDConstInt(MDParams->getOperand(0), 4);
            auto Rows = decodeMDConstInt(MDParams->getOperand(1), 4);
            if (Columns && Rows) {
              Params = std::pair<unsigned, unsigned>{
                  *Columns,
                  *Rows,
              };
            }
          }
        }
      }
    }

    if (Params) {
      Out = TR->getOpTypeMatrix(Params->first, Params->second, SPIRVElemTy,
                                *MIRB);
      TR->assignSPIRVTypeToVReg(Out, Out->getOperand(0).getReg(), *MIRB);
    } else {
      // fallback to a regular vector.
      Out = TR->getOrCreateSPIRVType(VTy, *MIRB);
    }
  }

  if (PTy || VTy) {
    if (VTy) {
      // don't duplicate decorations on the pointer type.
      decodeMDDecorations(DecSpec, Out);
    }
    return Out;
  }

  if (auto *STy = dyn_cast<StructType>(Ty)) {
    const auto ElementCount = STy->getNumElements();

    if (ElementCount == 0) {
      // this is probably a special opaque SPIRV type, like images, samplers,
      // etc. Delegate to the type spec, if present.
      if (auto *OTy = decodeMDOpaqueTypeSpec(TypeSpec, TypeSpec->operands())) {
        Out = OTy;
        return Out;
      }
    }

    StringRef Name;
    if (STy->hasName()) {
      Name = STy->getName();
    }

    SmallVector<SPIRVType *, 16> SPIRVElems;
    SmallVector<Metadata *, 16> DecorationMds;

    for (auto MemberTy = STy->subtype_begin(); MemberTy != STy->subtype_end();
         ++MemberTy) {
      auto *MemberSpec = NextSpec();
      MDNode *Spec = nullptr;
      // Decorations applied to the structure member, but not to the member
      // itself.
      Metadata *Dec = nullptr;
      if (MemberSpec) {
        Spec = dyn_cast<MDNode>(&*MemberSpec->getOperand(0));
        Dec = &*MemberSpec->getOperand(1);
      }
      DecorationMds.push_back(Dec);

      const auto *SPIRVMTy =
          transTypeSpecMDInner(*MemberTy, Spec, Visited);
      if (!SPIRVMTy) {
        SPIRVMTy = TR->getOrCreateSPIRVType(*MemberTy, *MIRB);
      }
      SPIRVElems.push_back(SPIRVMTy);
    }
    assert(DecorationMds.size() == SPIRVElems.size());

    const auto *SPIRVSTy = TR->getOpTypeStruct(SPIRVElems, *MIRB, Name);
    TR->assignSPIRVTypeToVReg(SPIRVSTy, SPIRVSTy->getOperand(0).getReg(),
                              *MIRB);

    for (unsigned MemberIdx = 0; MemberIdx < DecorationMds.size();
         MemberIdx++) {
      auto *Decoration = DecorationMds[MemberIdx];
      decodeMDDecorations(Decoration, SPIRVSTy, MemberIdx);
    }

    Out = SPIRVSTy;
    return Out;
  } else if (ATy) {
    auto *Spec = NextSpec();

    SPIRVElemTy = transTypeSpecMDInner(ElemTy, Spec, Visited);
    if (!SPIRVElemTy) {
      Out = TR->getOrCreateSPIRVType(Ty, *MIRB);
      return Out;
    }

    auto Length = ATy->getArrayNumElements();
    Out = TR->getOpTypeArray(Length, SPIRVElemTy, *MIRB);
    TR->assignSPIRVTypeToVReg(Out, Out->getOperand(0).getReg(), *MIRB);
    return Out;
  }

  Out = TR->getOrCreateSPIRVType(Ty, *MIRB);
  return Out;
}

SPIRVType *SPIRVPtrAnalysis::transTypeSpecMD(const GlobalObject *GO) {
  if (!GO) {
    return nullptr;
  }

  MDNode *MD = GO->getMetadata(GlobalTypeSpecMDKindID);
  if (!MD) {
    return nullptr;
  }

  LLVM_DEBUG(dbgs() << "Decoding type spec for GO " << *GO << "\n");

  auto *Ty = GO->getType();
  TypeSpecVisitedMD Visited;

  return transTypeSpecMDInner(Ty, MD, Visited);
}

SPIRVType *SPIRVPtrAnalysis::visitGlobal(const GlobalVariable *GV,
                                         MachineIRBuilder *MIRB) {
  // As long as we're run after SPIRVLegalizeConstsPass, the only users should
  // be instructions

  this->MIRB = MIRB;

  SPIRVTy = transTypeSpecMD(GV);
  if (!SPIRVTy) {
    return nullptr;
  }
  auto *GVSPIRVTy = SPIRVTy;
  TR->assignValueType(GV, SPIRVTy);

  IsOffsetKnown = true;
  PI.reset();
  VisitedUses.clear();

  for (const Use &U : GV->uses()) {
    if (auto *I = dyn_cast<Instruction>(U.getUser())) {
      if (I->getFunction() == F) {
        if (VisitedUses.insert(&U).second) {
          UseToVisit NewU = {
              UseToVisit::UseAndIsOffsetKnownPair(&U, IsOffsetKnown),
              SPIRVTy,
          };
          Worklist.push_back(std::move(NewU));
        }
      }
    }
  }

  // Visit all the uses off the worklist until it is empty.
  while (!Worklist.empty()) {
    UseToVisit ToVisit = Worklist.pop_back_val();
    U = ToVisit.UseAndIsOffsetKnown.getPointer();
    IsOffsetKnown = ToVisit.UseAndIsOffsetKnown.getInt();
    SPIRVTy = ToVisit.SPIRVTy;

    Instruction *I = cast<Instruction>(U->getUser());
    this->visit(I);
    if (PI.isAborted())
      break;
  }

  return GVSPIRVTy;
}
void SPIRVPtrAnalysis::visitGetElementPtrInst(GetElementPtrInst &I) {
  if (PI.isEscaped()) {
    return;
  }

  assert(SPIRVTy);

  auto SC = static_cast<StorageClass>(SPIRVTy->getOperand(1).getImm());
  auto *AggTy = getPointerElemTy();
  for (unsigned CurIdx = 1; CurIdx < I.getNumIndices(); ++CurIdx) {
    const auto AggTyOp = AggTy->getOpcode();
    switch (AggTyOp) {
    case OpTypeVector:
    case OpTypeArray:
      AggTy = TR->getSPIRVTypeForVReg(AggTy->getOperand(1).getReg());
      break;
    case OpTypeStruct: {
      auto CIdx = cast<ConstantInt>(I.getOperand(CurIdx + 1));
      auto Idx = CIdx->getZExtValue();
      auto Ty = AggTy->getOperand(Idx + 1).getReg();
      AggTy = TR->getSPIRVTypeForVReg(Ty);
      break;
    }
    default: {
      llvm::errs() << "Unexpected type opcode: " << *AggTy << "\n";
      llvm_unreachable("unexpected type opcode");
    }
    }
  }

  SPIRVTy = TR->getOpTypePointer(SC, AggTy, *MIRB);
  TR->assignValueType(&I, SPIRVTy);
  enqueueUsers(I);

  return;
}
void SPIRVPtrAnalysis::visitAddrSpaceCastInst(AddrSpaceCastInst &I) {
  I.getFunction()->getParent()->dump();
  auto SC = TR->addressSpaceToStorageClass(I.getDestAddressSpace());
  auto *PteTy = getPointerElemTy();

  SPIRVTy = TR->getOpTypePointer(SC, PteTy, *MIRB);
  assignValueType(I);
  enqueueUsers(I);
  return;
}
void SPIRVPtrAnalysis::visitLoadInst(LoadInst &LI) {
  auto LoadTy = getPointerElemTy();
  TR->assignValueType(&LI, LoadTy);
  return;
}
SPIRVType *SPIRVPtrAnalysis::getPointerElemTy(SPIRVType *Ty) {
  assert(Ty->getOpcode() == OpTypePointer);
  return TR->getSPIRVTypeForVReg(SPIRVTy->getOperand(2).getReg());
}
