set(LLVM_LINK_COMPONENTS SPIRVUtils)

add_llvm_library(LLVMSPIRVAsmPrinter
  SPIRVInstPrinter.cpp
)
