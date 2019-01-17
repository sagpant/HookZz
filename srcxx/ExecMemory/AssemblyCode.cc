#include "AssemblyCode.h"
#include "ExecutableMemoryArena.h"
#include "ExecMemory/CodePatchTool.h"
#include "logging/logging.h"

#if TARGET_ARCH_ARM
using namespace zz::arm;
#elif TARGET_ARCH_ARM64
using namespace zz::arm64;
#elif TARGET_ARCH_X64
using namespace zz::x64;
#endif

namespace zz {

AssemblyCode *AssemblyCode::FinalizeFromAddress(uintptr_t address, int size) {
  AssemblyCode *code = new AssemblyCode;
  code->initWithAddressRange((void *)address, size);
  return code;
}

AssemblyCode *AssemblyCode::FinalizeFromTruboAssember(AssemblerBase *assembler) {
  TurboAssembler *turboAssembler = reinterpret_cast<TurboAssembler *>(assembler);
  int buffer_size                   = turboAssembler->GetCodeBuffer()->getSize();

// Allocate the executable memory
#if TARGET_ARCH_ARM64 || TARGET_ARCH_ARM
  // extra bytes for align needed
  buffer_size += 4;
#endif
  AssemblyCodeChunk *codeChunk = ExecutableMemoryArena::AllocateCodeChunk(buffer_size);

  // Realize(Relocate) the buffer_code to the executable_memory_address, remove the ExternalLabels, etc, the pc-relative instructions
  turboAssembler->CommitRealizeAddress(codeChunk->address);
  CodePatchTool::PatchCodeBuffer(turboAssembler->GetRealizeAddress(), turboAssembler->GetCodeBuffer());

  // Alloc a new AssemblyCode
  AssemblyCode *code = new AssemblyCode;
  code->initWithCodeBuffer(turboAssembler->GetCodeBuffer());

  DLOG("[*] AssemblyCode finalize assembler at %p\n", code->raw_instruction_start());
  return reinterpret_cast<AssemblyCode *>(code);
}

} // namespace zz
