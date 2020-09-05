//===-- SPIRV.td - Describe the SPIR-V Target Machine ------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

include "llvm/Target/Target.td"

include "SPIRVRegisterInfo.td"
include "SPIRVRegisterBanks.td"
include "SPIRVInstrInfo.td"

def SPIRVInstrInfo : InstrInfo;

class Proc<string Name, list<SubtargetFeature> Features>
 : Processor<Name, NoItineraries, Features>;

def : Proc<"generic", []>;

def DummyFeature : SubtargetFeature<"dummy", "isDummyMode",
                                    "true", "unused feature">;

def SPIRVInstPrinter : AsmWriter {
  string AsmWriterClassName  = "InstPrinter";
  bit isMCAsmWriter = 1;
}

def SPIRV : Target {
  let InstructionSet = SPIRVInstrInfo;
  let AssemblyWriters = [SPIRVInstPrinter];
}

class CapabilityFeature<string Name, string LowerName, int id,
                        list<SubtargetFeature> ImpliedCaps = [],
                        string Desc = ""> :
  SubtargetFeature<LowerName, "Has"#Name, "true", Desc, ImpliedCaps>
{
  // Not actually used?
  int Id = id;
}
def FeatureMatrix : CapabilityFeature<"Matrix", "matrix", 0>;
def FeatureShader : CapabilityFeature<"Shader", "shader", 1, [FeatureMatrix]>;
def FeatureGeometry : CapabilityFeature<"Geometry", "geometry", 2, [FeatureShader]>;
def FeatureTessellation : CapabilityFeature<"Tessellation", "tessellation", 3, [FeatureShader]>;
def FeatureAddresses : CapabilityFeature<"Addresses", "addresses", 4>;
def FeatureLinkage : CapabilityFeature<"Linkage", "linkage", 5>;
def FeatureKernel : CapabilityFeature<"Kernel", "kernel", 6>;
def FeatureVector16 : CapabilityFeature<"Vector16", "vector16", 7, [FeatureKernel]>;
def FeatureFloat16Buffer : CapabilityFeature<"Float16Buffer", "f16-buffer", 8, [FeatureKernel]>;
def FeatureFloat16 : CapabilityFeature<"Float16", "f16", 9>;
def FeatureFloat64 : CapabilityFeature<"Float64", "f64", 10>;
def FeatureInt64 : CapabilityFeature<"Int64", "i64", 11>;
def FeatureInt64Atomics : CapabilityFeature<"Int64Atomics", "i64-atomics", 12, [FeatureInt64]>;
def FeatureImageBasic : CapabilityFeature<"ImageBasic", "image-basic", 13, [FeatureKernel]>;
def FeatureImageReadWrite : CapabilityFeature<"ImageReadWrite", "image-read-write", 14, [FeatureImageBasic]>;
def FeatureImageMipmap : CapabilityFeature<"ImageMipmap", "image-mipmap", 15, [FeatureImageBasic]>;
def FeaturePipes : CapabilityFeature<"Pipes", "pipes", 17, [FeatureKernel]>;
def FeatureGroups : CapabilityFeature<"Groups", "groups", 18>;
def FeatureDeviceEnqueue : CapabilityFeature<"DeviceEnqueue", "device-enqueue", 19>;
def FeatureLiteralSampler : CapabilityFeature<"LiteralSampler", "literal-sampler", 20, [FeatureKernel]>;
def FeatureAtomicStorage : CapabilityFeature<"AtomicStorage", "atomic-storage", 21, [FeatureShader]>;
def FeatureInt16 : CapabilityFeature<"Int16", "i16", 22>;
def FeatureInt8 : CapabilityFeature<"Int8", "i8", 39>;
def FeatureStorageBuffer16BitAccess : CapabilityFeature<"StorageBuffer16BitAccess", "storage-buffer-i16-access", 4433>;
def FeatureStorageUniform16BitAccess : CapabilityFeature<"StorageUniform16BitAccess",
  "storage-uniform-i16-access", 4434, [FeatureStorageBuffer16BitAccess]>;
def FeatureStorageInputOutput16BitAccess : CapabilityFeature<"StorageInputOutput16BitAccess",
  "storage-input-output-i16-access", 4436>;
def FeatureVariablePointersStorageBuffer :
  CapabilityFeature<"VariablePointersStorageBuffer", "variable-pointers-storage-buffer", 4441>;
def FeatureVariablePointers : CapabilityFeature<"VariablePointers", "variable-pointers", 4442>;
def FeatureStorageBuffer8BitAccess : CapabilityFeature<"StorageBuffer8BitAccess", "storage-buffer-i8-access", 4448>;
def FeatureStorageUniform8BitAccess : CapabilityFeature<"StorageUniform8BitAccess",
  "storage-uniform-i8-access", 4449, [FeatureStorageBuffer8BitAccess]>;
def FeatureVulkanMemoryModelKHR : CapabilityFeature<"VulkanMemoryModelKHR", "vulkan-memory-model", 5345>;
def FeatureVulkanMemoryModelDeviceScopeKHR :
  CapabilityFeature<"VulkanMemoryModelDeviceScopeKHR", "vulkan-memory-model-device-scope", 5346>;
def FeaturePhysicalStorageBufferAddressesEXT :
  CapabilityFeature<"PhysicalStorageBufferAddresses", "physical-storage-buffer-addresses", 5347, [FeatureShader]>;

