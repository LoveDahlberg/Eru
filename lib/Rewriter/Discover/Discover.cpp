

#include <Rewriter/Discover/Discover.h>
#include <Rewriter/Discover/Relocations.h>
#include <Rewriter/Discover/Sections.h>
#include <Rewriter/Discover/Symbols.h>

namespace Rewriter {

Error DiscoverAndRegisterObject(llvm::object::ELF64LEObjectFile *elfFile,
                                llvm::bolt::BinaryContext *binaryContext) {

  auto maybeMapping = DiscoverAndRegisterSections(elfFile, binaryContext);
  RET_ON_FAILURE(maybeMapping, "Failed to discover and register sections");

  auto sectionVirtualMapping = *maybeMapping;
  RET_ON_FAILURE(
      DiscoverAndProcessSymbols(elfFile, binaryContext, sectionVirtualMapping),
      "Failed to discover and register symbols");

  RET_ON_FAILURE(
      ProcessRelocations(elfFile, binaryContext, sectionVirtualMapping),
      "Failed to process relocations");

  return ERU_SUCCESS;
}

} // namespace Rewriter