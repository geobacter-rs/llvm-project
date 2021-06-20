add_llvm_component_library(LLVMSPIRVDesc
  SPIRVMCTargetDesc.cpp
  SPIRVAsmBackend.cpp
  SPIRVMCCodeEmitter.cpp
  SPIRVObjectTargetWriter.cpp

  LINK_COMPONENTS
  SPIRVAsmPrinter
  SPIRVUtils
  )
