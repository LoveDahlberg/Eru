#include <Support/Result.h>

#include <filesystem>
#include <vector>

namespace Frontend::Linker {

struct LinkerData {
  const std::string& outputPath;
  const std::filesystem::path& executablePath; 
};

Error Link(const std::vector<std::string> objectFiles, const LinkerData data);

} // namespace Frontend::Linker
