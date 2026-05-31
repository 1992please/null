#pragma once

#include <string>
#include <vector>

namespace ne {
class Pipeline {
public:
  Pipeline(const std::string &iVertFilePath, const std::string &iFragFilePath);

private:
  static std::vector<char> readFile(const std::string &iFilePath);

  void createGraphicsPipeline(const std::string &iVertFilePath,
                              const std::string &iFragFilePath);
};
} // namespace ne
