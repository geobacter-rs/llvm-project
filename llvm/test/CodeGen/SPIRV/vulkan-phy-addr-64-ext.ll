; RUN: llc -O0 -global-isel -mattr=+physical-storage-buffer-addresses %s -o - | FileCheck %s

target triple = "spirv64-unknown-unknown-vulkan"

; CHECK: OpMemoryModel Physical64

define spir_kernel void @dummy() {
entry:
  ret void
}
