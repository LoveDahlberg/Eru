#include <Support/IO/File.h>
#include <Support/Result.h>

#include <filesystem>
#include <vector>

namespace Frontend::Linker {

struct LinkerData {
  const std::filesystem::path& executablePath; 
};

Error Link(Support::IO::Files& files, const LinkerData data);

} // namespace Frontend::Linker
