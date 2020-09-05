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

#pragma once

#include "SPIRVSubtarget.h"
#include "SPIRVTypeRegistry.h"
#include "llvm/Analysis/PtrUseVisitor.h"

namespace llvm {

class SPIRVPtrAnalysis : public InstVisitor<SPIRVPtrAnalysis> {
  using Base = InstVisitor<SPIRVPtrAnalysis>;

  SPIRVTypeRegistry *TR;
  const Function *F;
  MachineIRBuilder *MIRB = nullptr;
  unsigned GlobalDecorationMDKindID;
  unsigned GlobalTypeSpecMDKindID;

  detail::PtrUseVisitorBase::PtrInfo PI;

  /// A struct of the data needed to visit a particular use.
  ///
  /// This is used to maintain a worklist of to-visit uses. This is used to
  /// make the visit be iterative rather than recursive.
  struct UseToVisit {
    using UseAndIsOffsetKnownPair = PointerIntPair<const Use *, 1, bool>;

    UseAndIsOffsetKnownPair UseAndIsOffsetKnown;
    SPIRVType *SPIRVTy;
  };

  SmallVector<UseToVisit, 8> Worklist;
  SmallPtrSet<const Use *, 8> VisitedUses;

  const Use *U;
  bool IsOffsetKnown;
  SPIRVType *SPIRVTy;

  typedef std::pair<Type *, std::pair<const MDNode *, const MDNode *>>
      TypeSpecVisitedKey;
  typedef SmallDenseMap<TypeSpecVisitedKey, SPIRVType *, 16> TypeSpecVisitedMD;
  struct Cleanup {
    SPIRVPtrAnalysis *Parent;
    TypeSpecVisitedMD &Visited;
    TypeSpecVisitedKey Key;
    SPIRVType *Out;

    inline const MDNode* typeSpec() const {
      return Key.second.first;
    }
    inline const MDNode* decSpec() const {
      return Key.second.second;
    }

    inline const TypeSpecVisitedKey& key() const {
      return Key;
    }

    ~Cleanup() {
      if (Out == nullptr) {
        Visited.erase(key());
      } else {
        Parent->decodeMDDecorations(decSpec(), Out);
        Visited[key()] = Out;
      }
    }
  };

  inline SPIRVType *getPointerElemTy() { return getPointerElemTy(SPIRVTy); }
  SPIRVType *getPointerElemTy(SPIRVType *Ty);
  inline void assignValueType(Instruction &I) {
    TR->assignValueType(&I, SPIRVTy);
  }

  void enqueueUsers(Instruction &I);

  SPIRVType *getImageSampledType(StringRef S);
  SPIRVType *decodeMDOpaqueTypeSpec(const Metadata *This,
                                    MDNode::op_range Operands);
  SPIRVType *transTypeSpecMDInner(Type *Ty, const Metadata *Spec,
                                  TypeSpecVisitedMD &Visited,
                                  SPIRVType *FwdPtr = nullptr);
  SPIRVType *transTypeSpecMDInner(Type *Ty, const MDNode *TypeSpec,
                                  const MDNode *DecSpec,
                                  TypeSpecVisitedMD &Visited,
                                  SPIRVType *FwdPtr = nullptr);
  SPIRVType *transTypeSpecMD(const GlobalObject *GO);

  void decodeMDDecoration(const Metadata *MD, SPIRVType *Target,
                          Optional<uint32_t> Member = None);
  void decodeMDDecorations(const Metadata *MD, SPIRVType *Target,
                           Optional<uint32_t> Member = None);

public:
  SPIRVPtrAnalysis(SPIRVTypeRegistry *TR, const Function *F);

  SPIRVType *visitGlobal(const GlobalVariable *GV, MachineIRBuilder *MIRB);

  inline void visitBitCastInst(BitCastInst &BC) {
    PI.setEscaped(&BC);
    return;
  }

  void visitGetElementPtrInst(GetElementPtrInst &I);
  void visitAddrSpaceCastInst(AddrSpaceCastInst &I);
  void visitLoadInst(LoadInst &LI);
};

} // namespace llvm
