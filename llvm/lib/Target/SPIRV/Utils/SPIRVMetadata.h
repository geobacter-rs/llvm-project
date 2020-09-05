//===-- SPIRVMetadata.h - SPIR-V Metadata decoding utils --------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#pragma once

#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Metadata.h"

namespace llvm {
namespace SPIRV {
Optional<StringRef> decodeMDStr(const Metadata *MD);
Optional<uint32_t> decodeMDConstInt(const Metadata *MD,
                                    uint32_t Limit = UINT32_MAX);
}
}
