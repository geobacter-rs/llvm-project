//===-- SPIRVMetadata.h - SPIR-V Metadata decoding utils --------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "SPIRVMetadata.h"

using namespace llvm;
using namespace SPIRV;

Optional<StringRef> llvm::SPIRV::decodeMDStr(const Metadata *MD) {
  if (auto *MDStr = dyn_cast_or_null<MDString>(MD)) {
    return {MDStr->getString()};
  } else {
    return None;
  }
}
Optional<uint32_t> llvm::SPIRV::decodeMDConstInt(const Metadata *MD,
                                                 uint32_t Limit) {
  if (auto *C = mdconst::dyn_extract<ConstantInt>(MD)) {
    const auto Value = C->getLimitedValue(~0ULL);
    if (Value <= Limit) {
      return {(uint32_t)Value};
    }
  }

  return None;
}
